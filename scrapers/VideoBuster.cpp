#include "VideoBuster.h"
#include <QTextDocument>

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

/**
 * @brief Returns a list of infos available from the scraper
 * @return List of supported infos
 */
QList<int> VideoBuster::scraperSupports()
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
    QUrl url(QString("https://www.videobuster.de/titlesearch.php?tab_search_content=movies&view=title_list_view_option_list&search_title=%1").arg(searchStr));
    m_searchReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "searchDone" if there are no more pages in the result set
 * @see VideoBuster::parseSearch
 */
void VideoBuster::searchFinished()
{
    QList<ScraperSearchResult> results;
    if (m_searchReply->error() == QNetworkReply::NoError ) {
        QString msg = m_searchReply->readAll();
        results = parseSearch(msg);
    }
    m_searchReply->deleteLater();
    emit searchDone(results);
}

/**
 * @brief Parses the search results
 * @param html Downloaded HTML data
 * @return List of search results
 */
QList<ScraperSearchResult> VideoBuster::parseSearch(QString html)
{
    QList<ScraperSearchResult> results;
    int pos = 0;
    QRegExp rx("class=\"movietip_yes\">([^<]*)</a>.*<a class=\"more\" href=\"([^\"]*)\">.*<label>Produktion:</label>.*([0-9]+)</div>");
    rx.setMinimal(true);
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.name     = rx.cap(1);
        result.id       = rx.cap(2);
        result.released = QDate::fromString(rx.cap(3), "yyyy");
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
void VideoBuster::loadData(QString id, Movie *movie, QList<int> infos)
{
    m_infosToLoad = infos;
    m_currentMovie = movie;
    QUrl url(QString("https://www.videobuster.de%1").arg(id));
    m_loadReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(loadFinished()));
}

/**
 * @brief Called when the movie infos are downloaded
 * @see VideoBuster::parseAndAssignInfos
 */
void VideoBuster::loadFinished()
{
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = m_loadReply->readAll();
        parseAndAssignInfos(msg, m_currentMovie, m_infosToLoad);
    }
    m_loadReply->deleteLater();
}

/**
 * @brief Parses HTML data and assigns it to the given movie object
 * @param html HTML data
 * @param movie Movie object
 * @param infos List of infos to load
 */
void VideoBuster::parseAndAssignInfos(QString html, Movie *movie, QList<int> infos)
{
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
        movie->addCountry(rx.cap(1).trimmed());
        pos += rx.matchedLength();
    }

    // MPAA
    rx.setPattern("FSK ab ([0-9]+) ");
    if (infos.contains(MovieScraperInfos::Certification) && rx.indexIn(html) != -1)
        movie->setCertification("FSK " + rx.cap(1));

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
        movie->addStudio(rx.cap(1).trimmed());

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
        movie->addGenre(rx.cap(1).trimmed());

    // Tagline
    rx.setPattern("class=\"long_name\">([^<]*)</p>");
    if (infos.contains(MovieScraperInfos::Tagline) && rx.indexIn(html) != -1)
        movie->setTagline(rx.cap(1).trimmed());

    // Overview
    rx.setPattern("<div class=\"txt movie_description\">(.*)(<br />|</div>)");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setOverview(doc.toPlainText());
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
        m_backdropReply = qnam()->get(QNetworkRequest(backdropUrl));
        connect(m_backdropReply, SIGNAL(finished()), this, SLOT(backdropFinished()));
    } else {
        m_currentMovie->scraperLoadDone();
    }
}

/**
 * @brief Called when backdrops are loaded
 */
void VideoBuster::backdropFinished()
{
    if (m_backdropReply->error() == QNetworkReply::NoError ) {
        QString msg = m_backdropReply->readAll();
        QRegExp rx("href=\"https://gfx.videobuster.de/archive/resized/([^\"]*)\"(.*)([^<]*)<img (.*) src=\"https://gfx.videobuster.de/archive/resized/c110/([^\"]*)\"");
        rx.setMinimal(true);
        int pos = 0;
        int counter = 0;
        while ((pos = rx.indexIn(msg, pos)) != -1) {
            pos += rx.matchedLength();
            if (rx.cap(2).contains("titledtl_cover_pictures")) {
                continue;
            }
            Poster p;
            p.thumbUrl = QUrl(QString("https://gfx.videobuster.de/archive/resized/w700/%1").arg(rx.cap(5)));
            p.originalUrl = QUrl(QString("https://gfx.videobuster.de/archive/resized/%1").arg(rx.cap(1)));
            m_currentMovie->addBackdrop(p);
            counter++;
        }
    }
    m_backdropReply->deleteLater();
    m_currentMovie->scraperLoadDone();
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
void VideoBuster::loadSettings()
{
}

/**
 * @brief Saves scrapers settings
 */
void VideoBuster::saveSettings()
{
}

/**
 * @brief Constructs a widget with scrapers settings
 * @return Settings Widget
 */
QWidget* VideoBuster::settingsWidget()
{
    return 0;
}
