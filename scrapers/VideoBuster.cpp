#include "VideoBuster.h"
#include <QTextDocument>
#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/NetworkReplyWatcher.h"
#include "settings/Settings.h"

/**
 * @brief VideoBuster::VideoBuster
 * @param parent
 */
VideoBuster::VideoBuster(QObject *parent)
{
    setParent(parent);
    m_scraperSupports << MovieScraperInfos::Title
                      << MovieScraperInfos::Released
                      << MovieScraperInfos::Countries
                      << MovieScraperInfos::Certification
                      << MovieScraperInfos::Actors
                      << MovieScraperInfos::Studios
                      << MovieScraperInfos::Runtime
                      << MovieScraperInfos::Rating
                      << MovieScraperInfos::Genres
                      << MovieScraperInfos::Tagline
                      << MovieScraperInfos::Overview
                      << MovieScraperInfos::Poster
                      << MovieScraperInfos::Backdrop
                      << MovieScraperInfos::Tags
                      << MovieScraperInfos::Director;
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager *VideoBuster::qnam()
{
    return &m_qnam;
}

/**
 * @brief Returns the name of the scraper
 * @return Name of the Scraper
 */
QString VideoBuster::name()
{
    return QString("VideoBuster");
}

QString VideoBuster::identifier()
{
    return QString("videobuster");
}

bool VideoBuster::isAdult()
{
    return false;
}

/**
 * @brief Returns a list of infos available from the scraper
 * @return List of supported infos
 */
QList<int> VideoBuster::scraperSupports()
{
    return m_scraperSupports;
}

QList<int> VideoBuster::scraperNativelySupports()
{
    return m_scraperSupports;
}

/**
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @see VideoBuster::searchFinished
 */
void VideoBuster::search(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    QString encodedSearch = Helper::instance()->toLatin1PercentEncoding(searchStr);
    QUrl url(QString("https://www.videobuster.de/titlesearch.php?tab_search_content=movies&view=title_list_view_option_list&search_title=%1").arg(encodedSearch).toUtf8());
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    connect(reply, SIGNAL(finished()), this, SLOT(searchFinished()));
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "searchDone" if there are no more pages in the result set
 * @see VideoBuster::parseSearch
 */
void VideoBuster::searchFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());

    QList<ScraperSearchResult> results;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = reply->readAll();
        msg = replaceEntities(msg);
        results = parseSearch(msg);
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }
    reply->deleteLater();
    emit searchDone(results);
}

/**
 * @brief Parses the search results
 * @param html Downloaded HTML data
 * @return List of search results
 */
QList<ScraperSearchResult> VideoBuster::parseSearch(QString html)
{
    qDebug() << "Entered";
    QList<ScraperSearchResult> results;
    int pos = 0;
    QRegExp rx("<div class=\"infos\"><a href=\"([^\"]*)\" class=\"title\">([^<]*)</a>");
    rx.setMinimal(true);
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.name     = rx.cap(2);
        result.id       = rx.cap(1);
        results.append(result);
        pos += rx.matchedLength();
    }
    return results;
}

/**
 * @brief Starts network requests to download infos from VideoBuster
 * @param id VideoBuster movie ID
 * @param movie Movie object
 * @param infos List of infos to load
 * @see VideoBuster::loadFinished
 */
void VideoBuster::loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos)
{
    movie->clear(infos);

    QUrl url(QString("https://www.videobuster.de%1").arg(ids.values().first()));
    qDebug() << url;
    QNetworkReply *reply = this->qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(loadFinished()));
}

/**
 * @brief Called when the movie infos are downloaded
 * @see VideoBuster::parseAndAssignInfos
 */
void VideoBuster::loadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = reply->readAll();
        msg = replaceEntities(msg);
        parseAndAssignInfos(msg, movie, infos);
    } else {
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
void VideoBuster::parseAndAssignInfos(QString html, Movie *movie, QList<int> infos)
{
    qDebug() << "Entered";
    movie->clear(infos);
    QRegExp rx;
    rx.setMinimal(true);
    int pos = 0;
    QTextDocument doc;

    // Title
    rx.setPattern("<h1 itemprop=\"name\">(.*)</h1>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setName(rx.cap(1).trimmed());

    // Original Title
    rx.setPattern("<label>Originaltitel</label><br><span itemprop=\"alternateName\">(.*)</span>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setOriginalName(rx.cap(1).trimmed());

    // Year
    rx.setPattern("<span itemprop=\"copyrightYear\">([0-9]*)</span>");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1)
        movie->setReleased(QDate::fromString(rx.cap(1).trimmed(), "yyyy"));

    // Country
    pos = 0;
    rx.setPattern("<label>Produktion</label><br><a href=\"[^\"]*\">(.*)</a>");
    while (infos.contains(MovieScraperInfos::Countries) && (pos = rx.indexIn(html, pos)) != -1) {
        movie->addCountry(Helper::instance()->mapCountry(rx.cap(1).trimmed()));
        pos += rx.matchedLength();
    }

    // MPAA
    rx.setPattern("Freigegeben ab ([0-9]+) Jahren");
    if (infos.contains(MovieScraperInfos::Certification) && rx.indexIn(html) != -1)
        movie->setCertification(Helper::instance()->mapCertification("FSK " + rx.cap(1)));

    // Actors
    pos = 0;

    rx.setPattern("<span itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\"><a href=\"[^\"]*\" itemprop=\"url\"><span itemprop=\"name\">(.*)</span></a></span>");
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
            rx.setPattern("<a href=\"/persondtl.php/[^\"]*\">(.*)</a>");
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
            rx.setPattern("<a href=\"/titlesearch.php[^\"]*\">(.*)</a>");
            while ((pos = rx.indexIn(contents, pos)) != -1) {
                movie->addTag(rx.cap(1).trimmed());
                pos += rx.matchedLength();
            }
        }
    }

    // Studio
    rx.setPattern("<label>Studio</label><br><span itemprop=\"publisher\" itemscope itemtype=\"http://schema.org/Organization\">.*<span itemprop=\"name\">(.*)</span></a></span>");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1)
        movie->addStudio(Helper::instance()->mapStudio(rx.cap(1).trimmed()));

    // Runtime
    rx.setPattern("ca. ([0-9]*) Minuten");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1)
        movie->setRuntime(rx.cap(1).trimmed().toInt());

    // Rating
    if (infos.contains(MovieScraperInfos::Rating)) {
        rx.setPattern("<span itemprop=\"ratingCount\">([0-9]*)</span>");
        if (rx.indexIn(html) != -1)
            movie->setVotes(rx.cap(1).trimmed().toInt());
        rx.setPattern("<span itemprop=\"ratingValue\">(.*)</span>");
        if (rx.indexIn(html) != -1)
            movie->setRating(rx.cap(1).trimmed().replace(".", "").replace(",", ".").toFloat());
    }

    // Genres
    if (infos.contains(MovieScraperInfos::Genres)) {
        pos = 0;
        rx.setPattern("<a href=\"/genrelist\\.php/.*\">(.*)</a>");
        while ((pos = rx.indexIn(html, pos)) != -1) {
            movie->addGenre(Helper::instance()->mapGenre(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    // Tagline
    rx.setPattern("<p class=\"long_name\" itemprop=\"alternativeHeadline\">(.*)</p>");
    if (infos.contains(MovieScraperInfos::Tagline) && rx.indexIn(html) != -1)
        movie->setTagline(rx.cap(1).trimmed());

    // Overview
    rx.setPattern("<p itemprop=\"description\">(.*)</p>");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setOverview(doc.toPlainText());
        if (Settings::instance()->usePlotForOutline())
            movie->setOutline(doc.toPlainText());
    }

    // Posters
    if (infos.contains(MovieScraperInfos::Poster)) {
        rx.setPattern("<h3>Poster</h3><ul class=\"gallery_box  posters\">(.*)</ul>");
        if (rx.indexIn(html) != -1) {
            QString contents = rx.cap(1);
            pos = 0;
            rx.setPattern("<a href=\"https://gfx.videobuster.de/archive/([^\"]*)\" data-title=\"[^\"]*\" rel=\"gallery_posters\" target=\"_blank\" class=\"image\">");
            while ((pos = rx.indexIn(contents, pos)) != -1) {
                Poster p;
                p.thumbUrl = "https://gfx.videobuster.de/archive/" + rx.cap(1);
                p.originalUrl = "https://gfx.videobuster.de/archive/" + rx.cap(1);
                movie->addPoster(p);
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
            rx.setPattern("<a href=\"https://gfx.videobuster.de/archive/([^\"]*)\" data-title=\"[^\"]*\" rel=\"gallery_pictures\" target=\"_blank\" class=\"image\">");
            while ((pos = rx.indexIn(contents, pos)) != -1) {
                Poster p;
                p.thumbUrl = "https://gfx.videobuster.de/archive/" + rx.cap(1);
                p.originalUrl = "https://gfx.videobuster.de/archive/" + rx.cap(1);
                movie->addBackdrop(p);
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
bool VideoBuster::hasSettings()
{
    return false;
}

/**
 * @brief Loads scrapers settings
 */
void VideoBuster::loadSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

/**
 * @brief Saves scrapers settings
 */
void VideoBuster::saveSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

QWidget *VideoBuster::settingsWidget()
{
    return 0;
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
