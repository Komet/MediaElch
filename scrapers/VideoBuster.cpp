#include "VideoBuster.h"
#include <QTextDocument>
#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
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
                      << MovieScraperInfos::Backdrop;
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
    QString encodedSearch = Helper::toLatin1PercentEncoding(searchStr);
    QUrl url(QString("https://www.videobuster.de/titlesearch.php?tab_search_content=movies&view=title_list_view_option_list&search_title=%1").arg(encodedSearch).toUtf8());
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
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
    if (reply->error() == QNetworkReply::NoError ) {
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
    QRegExp rx("<div class=\"name\">([^>]*)>([^<]*)</a>.*<a class=\"more\" href=\"([^\"]*)\">.*<label>Produktion</label>.*([0-9]+)</div>");
    rx.setMinimal(true);
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.name     = rx.cap(2);
        result.id       = rx.cap(3);
        result.released = QDate::fromString(rx.cap(4), "yyyy");
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
    QNetworkReply *reply = this->qnam()->get(QNetworkRequest(url));
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

    if (reply->error() == QNetworkReply::NoError ) {
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
    rx.setPattern("class=\"name\">([^<]*)</h1>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setName(rx.cap(1).trimmed());

    // Original Title
    rx.setPattern("Originaltitel:</div>.*<div class=\"content\">([^<]*)</div>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setOriginalName(rx.cap(1).trimmed());

    // Year
    rx.setPattern("Produktion:</div>.*<div class=\"content\">.*([0-9]*)</div>");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1)
        movie->setReleased(QDate::fromString(rx.cap(1).trimmed(), "yyyy"));

    // Country
    pos = 0;
    rx.setPattern("Produktion:</div>.*<div class=\"content\">(.*)([0-9]+|</div>)");
    while (infos.contains(MovieScraperInfos::Countries) && (pos = rx.indexIn(html, pos)) != -1) {
        movie->addCountry(Helper::mapCountry(rx.cap(1).trimmed()));
        pos += rx.matchedLength();
    }

    // MPAA
    rx.setPattern("FSK ab ([0-9]+) ");
    if (infos.contains(MovieScraperInfos::Certification) && rx.indexIn(html) != -1)
        movie->setCertification(Helper::mapCertification("FSK " + rx.cap(1)));

    // Actors
    pos = 0;
    rx.setPattern("class=\"actor_link\">([^<]*)</a>");
    while (infos.contains(MovieScraperInfos::Actors) && (pos = rx.indexIn(html, pos)) != -1) {
        Actor a;
        a.name = rx.cap(1).trimmed();
        movie->addActor(a);
        pos += rx.matchedLength();
    }

    // Studio
    rx.setPattern("Studio:</div>.*<div class=\"content\">([^<]*)</div>");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1)
        movie->addStudio(Helper::mapStudio(rx.cap(1).trimmed()));

    // Runtime
    rx.setPattern("Laufzeit ca. ([0-9]*) Minuten");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1)
        movie->setRuntime(rx.cap(1).trimmed().toInt());

    // Rating
    rx.setPattern("Gesamtwertung: ([0-9.]+) Sterne  bei ([0-9]*) Stimmen");
    if (infos.contains(MovieScraperInfos::Rating) && rx.indexIn(html) != -1)
        movie->setRating(rx.cap(1).trimmed().toFloat());

    // Genres
    rx.setPattern("<a href='/genrelist.php/.*>([^<]*)</a>");
    if (infos.contains(MovieScraperInfos::Genres) && rx.indexIn(html) != -1)
        movie->addGenre(Helper::mapGenre(rx.cap(1).trimmed()));

    // Tagline
    rx.setPattern("class=\"long_name\">([^<]*)</p>");
    if (infos.contains(MovieScraperInfos::Tagline) && rx.indexIn(html) != -1)
        movie->setTagline(rx.cap(1).trimmed());

    // Overview
    rx.setPattern("<div class=\"txt movie_description\">(.*)(<br />|</div>)");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setOverview(doc.toPlainText());
        if (Settings::instance()->usePlotForOutline())
            movie->setOutline(doc.toPlainText());
    }

    // Posters
    pos = 0;
    rx.setPattern("src=\"(https://gfx.videobuster.de/archive/resized)/w124/([^\"]*)\"");
    while (infos.contains(MovieScraperInfos::Poster) && (pos = rx.indexIn(html, pos)) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1) + "/h550/" + rx.cap(2);
        p.originalUrl = rx.cap(1) + "/w700/" + rx.cap(2);
        movie->addPoster(p);
        pos += rx.matchedLength();
    }

    // Backdrops
    rx.setPattern("<a href=\"/titledtl.php/([^\\?]*)\\?tab=gallery&content_type_idnr=1");
    if (infos.contains(MovieScraperInfos::Backdrop) && rx.indexIn(html) != -1) {
        QUrl backdropUrl(QString("https://www.videobuster.de/titledtl.php/%1?tab=gallery&content_type_idnr=1").arg(rx.cap(1)));
        QNetworkReply *reply = qnam()->get(QNetworkRequest(backdropUrl));
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, SIGNAL(finished()), this, SLOT(backdropFinished()));
    } else {
        movie->controller()->scraperLoadDone(this);
    }
}

/**
 * @brief Called when backdrops are loaded
 */
void VideoBuster::backdropFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = reply->readAll();
        QRegExp rx("href=\"https://gfx.videobuster.de/archive/resized/([^\"]*)\"(.*)([^<]*)<img (.*) src=\"https://gfx.videobuster.de/archive/resized/c110/([^\"]*)\"");
        rx.setMinimal(true);
        int pos = 0;
        while ((pos = rx.indexIn(msg, pos)) != -1) {
            pos += rx.matchedLength();
            if (rx.cap(2).contains("titledtl_cover_pictures")) {
                continue;
            }
            Poster p;
            p.thumbUrl = QUrl(QString("https://gfx.videobuster.de/archive/resized/w700/%1").arg(rx.cap(5)));
            p.originalUrl = QUrl(QString("https://gfx.videobuster.de/archive/resized/%1").arg(rx.cap(1)));
            movie->addBackdrop(p);
        }
    } else {
        qWarning() << "Network Error" << reply->errorString();
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
