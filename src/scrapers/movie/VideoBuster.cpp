#include "VideoBuster.h"

#include <QTextDocument>

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/NetworkReplyWatcher.h"
#include "settings/Settings.h"

VideoBuster::VideoBuster(QObject* parent) :
    m_scraperSupports{MovieScraperInfos::Title,
        MovieScraperInfos::Released,
        MovieScraperInfos::Countries,
        MovieScraperInfos::Certification,
        MovieScraperInfos::Actors,
        MovieScraperInfos::Studios,
        MovieScraperInfos::Runtime,
        MovieScraperInfos::Rating,
        MovieScraperInfos::Genres,
        MovieScraperInfos::Tagline,
        MovieScraperInfos::Overview,
        MovieScraperInfos::Poster,
        MovieScraperInfos::Backdrop,
        MovieScraperInfos::Tags,
        MovieScraperInfos::Director}
{
    setParent(parent);
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager* VideoBuster::qnam()
{
    return &m_qnam;
}

/**
 * @brief Returns the name of the scraper
 * @return Name of the Scraper
 */
QString VideoBuster::name() const
{
    return QString("VideoBuster");
}

QString VideoBuster::identifier() const
{
    return scraperIdentifier;
}

bool VideoBuster::isAdult() const
{
    return false;
}

/**
 * @brief Returns a list of infos available from the scraper
 * @return List of supported infos
 */
QVector<MovieScraperInfos> VideoBuster::scraperSupports()
{
    return m_scraperSupports;
}

QVector<MovieScraperInfos> VideoBuster::scraperNativelySupports()
{
    return m_scraperSupports;
}

std::vector<ScraperLanguage> VideoBuster::supportedLanguages()
{
    return {{tr("German"), "de"}};
}

void VideoBuster::changeLanguage(QString /*languageKey*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

QString VideoBuster::defaultLanguageKey()
{
    return QStringLiteral("de");
}

/**
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @see VideoBuster::searchFinished
 */
void VideoBuster::search(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    QString encodedSearch = helper::toLatin1PercentEncoding(searchStr);
    QUrl url(QString("https://www.videobuster.de/"
                     "titlesearch.php?tab_search_content=movies&view=title_list_view_option_list&search_title=%1")
                 .arg(encodedSearch)
                 .toUtf8());
    QNetworkReply* reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    connect(reply, &QNetworkReply::finished, this, &VideoBuster::searchFinished);
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "searchDone" if there are no more pages in the result set
 * @see VideoBuster::parseSearch
 */
void VideoBuster::searchFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[VideoBuster] onSearchFinished: nullptr reply | Please report this issue!";
        emit searchDone({}, {ScraperSearchError::ErrorType::InternalError, tr("Internal Error: Please report!")});
        return;
    }
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[AdultDvdEmpire] Search: Network Error" << reply->errorString();
        emit searchDone({}, {ScraperSearchError::ErrorType::NetworkError, reply->errorString()});
        return;
    }

    QString msg = reply->readAll();
    msg = replaceEntities(msg);
    emit searchDone(parseSearch(msg), {});
}

/**
 * @brief Parses the search results
 * @param html Downloaded HTML data
 * @return List of search results
 */
QVector<ScraperSearchResult> VideoBuster::parseSearch(QString html)
{
    qDebug() << "Entered";
    QVector<ScraperSearchResult> results;
    int pos = 0;
    QRegExp rx("<div class=\"infos\"><a href=\"([^\"]*)\" class=\"title\">([^<]*)</a>");
    rx.setMinimal(true);
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.name = rx.cap(2);
        result.id = rx.cap(1);
        results.append(result);
        pos += rx.matchedLength();
    }
    return results;
}

/**
 * @brief Starts network requests to download infos from VideoBuster
 * @param ids VideoBuster movie ID
 * @param movie Movie object
 * @param infos List of infos to load
 * @see VideoBuster::loadFinished
 */
void VideoBuster::loadData(QMap<MovieScraperInterface*, QString> ids, Movie* movie, QVector<MovieScraperInfos> infos)
{
    movie->clear(infos);

    QUrl url(QString("https://www.videobuster.de%1").arg(ids.values().first()));
    QNetworkReply* reply = this->qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, &QNetworkReply::finished, this, &VideoBuster::loadFinished);
}

/**
 * @brief Called when the movie infos are downloaded
 * @see VideoBuster::parseAndAssignInfos
 */
void VideoBuster::loadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* movie = reply->property("storage").value<Storage*>()->movie();
    QVector<MovieScraperInfos> infos = reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = reply->readAll();
        msg = replaceEntities(msg);
        parseAndAssignInfos(msg, movie, infos);
    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error" << reply->errorString();
        movie->controller()->scraperLoadDone(this);
    }
}

/**
 * @brief Parses HTML data and assigns it to the given movie object
 * @param html HTML data
 * @param movie Movie object
 * @param infos List of infos to load
 */
void VideoBuster::parseAndAssignInfos(QString html, Movie* movie, QVector<MovieScraperInfos> infos)
{
    qDebug() << "[VideoBuster] parseAndAssignInfos";
    movie->clear(infos);
    QRegExp rx;
    rx.setMinimal(true);
    int pos = 0;
    QTextDocument doc;

    // Title
    rx.setPattern("<h1 itemprop=\"name\">(.*)</h1>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1) {
        movie->setName(rx.cap(1).trimmed());
    }

    // Original Title
    rx.setPattern("<label>Originaltitel</label><br><span itemprop=\"alternateName\">(.*)</span>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1) {
        movie->setOriginalName(rx.cap(1).trimmed());
    }

    // Year
    rx.setPattern("<span itemprop=\"copyrightYear\">([0-9]*)</span>");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1) {
        movie->setReleased(QDate::fromString(rx.cap(1).trimmed(), "yyyy"));
    }

    // Country
    pos = 0;
    rx.setPattern(R"(<label>Produktion</label><br><a href="[^"]*">(.*)</a>)");
    while (infos.contains(MovieScraperInfos::Countries) && (pos = rx.indexIn(html, pos)) != -1) {
        movie->addCountry(helper::mapCountry(rx.cap(1).trimmed()));
        pos += rx.matchedLength();
    }

    // MPAA
    if (infos.contains(MovieScraperInfos::Certification)) {
        // 2016 | FSK 0
        rx.setPattern("[0-9]{4} [|] FSK ([0-9]+)");
        if (rx.indexIn(html) != -1) {
            movie->setCertification(helper::mapCertification(Certification::FSK(rx.cap(1))));
        }
    }

    // Actors
    pos = 0;

    // clear actors
    movie->setActors({});

    rx.setPattern("<span itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\"><a href=\"[^\"]*\" "
                  "itemprop=\"url\"><span itemprop=\"name\">(.*)</span></a></span>");
    while (infos.contains(MovieScraperInfos::Actors) && (pos = rx.indexIn(html, pos)) != -1) {
        Actor a;
        a.name = rx.cap(1).trimmed();
        movie->addActor(a);
        pos += rx.matchedLength();
    }

    if (infos.contains(MovieScraperInfos::Director)) {
        rx.setPattern("<p><label>Regie</label><br>(.*)</p>");
        if (rx.indexIn(html) != -1) {
            pos = 0;
            QString contents = rx.cap(1);
            QStringList directors;
            rx.setPattern(R"(<a href="/persondtl.php/[^"]*">(.*)</a>)");
            while ((pos = rx.indexIn(contents, pos)) != -1) {
                directors.append(rx.cap(1).trimmed());
                pos += rx.matchedLength();
            }
            movie->setDirector(directors.join(", "));
        }
    }

    if (infos.contains(MovieScraperInfos::Tags)) {
        rx.setPattern("<label>Schlagw&ouml;rter</label><br><span itemprop=\"keywords\">(.*)</span>");
        if (rx.indexIn(html) != -1) {
            pos = 0;
            QString contents = rx.cap(1);
            rx.setPattern(R"(<a href="/titlesearch.php[^"]*">(.*)</a>)");
            while ((pos = rx.indexIn(contents, pos)) != -1) {
                movie->addTag(rx.cap(1).trimmed());
                pos += rx.matchedLength();
            }
        }
    }

    // Studio
    rx.setPattern("<label>Studio</label><br><span itemprop=\"publisher\" itemscope "
                  "itemtype=\"http://schema.org/Organization\">.*<span itemprop=\"name\">(.*)</span></a></span>");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1) {
        movie->addStudio(helper::mapStudio(rx.cap(1).trimmed()));
    }

    // Runtime
    rx.setPattern("ca. ([0-9]*) Minuten");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1) {
        movie->setRuntime(std::chrono::minutes(rx.cap(1).trimmed().toInt()));
    }

    // Rating
    if (infos.contains(MovieScraperInfos::Rating)) {
        Rating rating;
        rating.source = "VideoBuster";
        rx.setPattern("<span itemprop=\"ratingCount\">([0-9]*)</span>");
        if (rx.indexIn(html) != -1) {
            rating.voteCount = rx.cap(1).trimmed().toInt();
        }
        rx.setPattern("<span itemprop=\"ratingValue\">(.*)</span>");
        if (rx.indexIn(html) != -1) {
            rating.rating = rx.cap(1).trimmed().replace(".", "").replace(",", ".").toDouble();
        }
        movie->ratings().push_back(rating);
    }

    // Genres
    if (infos.contains(MovieScraperInfos::Genres)) {
        pos = 0;
        rx.setPattern(R"(<a href="/genrelist\.php/.*">(.*)</a>)");
        while ((pos = rx.indexIn(html, pos)) != -1) {
            movie->addGenre(helper::mapGenre(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    // Tagline
    rx.setPattern(R"(<p class="long_name" itemprop="alternativeHeadline">(.*)</p>)");
    if (infos.contains(MovieScraperInfos::Tagline) && rx.indexIn(html) != -1) {
        movie->setTagline(rx.cap(1).trimmed());
    }

    // Overview
    rx.setPattern("<p itemprop=\"description\">(.*)</p>");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setOverview(doc.toPlainText());
        if (Settings::instance()->usePlotForOutline()) {
            movie->setOutline(doc.toPlainText());
        }
    }

    // Posters
    if (infos.contains(MovieScraperInfos::Poster)) {
        rx.setPattern("<h3>Poster</h3><ul class=\"gallery_box  posters\">(.*)</ul>");
        if (rx.indexIn(html) != -1) {
            QString contents = rx.cap(1);
            pos = 0;
            rx.setPattern("<a href=\"https://gfx.videobuster.de/archive/([^\"]*)\" data-title=\"[^\"]*\" "
                          "rel=\"gallery_posters\" target=\"_blank\" class=\"image\">");
            while ((pos = rx.indexIn(contents, pos)) != -1) {
                Poster p;
                p.thumbUrl = "https://gfx.videobuster.de/archive/" + rx.cap(1);
                p.originalUrl = "https://gfx.videobuster.de/archive/" + rx.cap(1);
                movie->images().addPoster(p);
                pos += rx.matchedLength();
            }
        }
    }

    // Backdrops
    if (infos.contains(MovieScraperInfos::Backdrop)) {
        rx.setPattern("<h3>Szenenbilder</h3><ul class=\"gallery_box  pictures\">(.*)</ul>");
        if (rx.indexIn(html) != -1) {
            QString contents = rx.cap(1);
            pos = 0;
            rx.setPattern("<a href=\"https://gfx.videobuster.de/archive/([^\"]*)\" data-title=\"[^\"]*\" "
                          "rel=\"gallery_pictures\" target=\"_blank\" class=\"image\">");
            while ((pos = rx.indexIn(contents, pos)) != -1) {
                Poster p;
                p.thumbUrl = "https://gfx.videobuster.de/archive/" + rx.cap(1);
                p.originalUrl = "https://gfx.videobuster.de/archive/" + rx.cap(1);
                movie->images().addBackdrop(p);
                pos += rx.matchedLength();
            }
        }
    }

    movie->controller()->scraperLoadDone(this);
}

/**
 * @brief Returns if the scraper has settings
 * @return Scraper has settings
 */
bool VideoBuster::hasSettings() const
{
    return false;
}

/**
 * @brief Loads scrapers settings
 */
void VideoBuster::loadSettings(const ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

/**
 * @brief Saves scrapers settings
 */
void VideoBuster::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QWidget* VideoBuster::settingsWidget()
{
    return nullptr;
}

/**
 * @brief This function replaces entities with their unicode counterparts
 * @param msg String with entities
 * @return String without entities
 */
QString VideoBuster::replaceEntities(const QString msg)
{
    // not nice but I don't know other methods which don't require the gui module
    QString m = msg;
    m.replace("&#039;", "'");
    return m;
}
