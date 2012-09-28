#include "Cinefacts.h"
#include <QTextDocument>
#include "globals/Globals.h"
#include "globals/Helper.h"

/**
 * @brief Cinefacts::Cinefacts
 * @param parent
 */
Cinefacts::Cinefacts(QObject *parent)
{
    setParent(parent);
    m_scraperSupports << MovieScraperInfos::Title
                      << MovieScraperInfos::Genres
                      << MovieScraperInfos::Released
                      << MovieScraperInfos::Countries
                      << MovieScraperInfos::Actors
                      << MovieScraperInfos::Studios
                      << MovieScraperInfos::Certification
                      << MovieScraperInfos::Runtime
                      << MovieScraperInfos::Overview
                      << MovieScraperInfos::Backdrop
                      << MovieScraperInfos::Poster;
}

/**
 * @brief Returns the name of the scraper
 * @return Name of the Scraper
 */
QString Cinefacts::name()
{
    return QString("Cinefacts");
}

/**
 * @brief Returns if the scraper has settings
 * @return Scraper has settings
 */
bool Cinefacts::hasSettings()
{
    return false;
}

/**
 * @brief Loads scrapers settings
 */
void Cinefacts::loadSettings()
{
}

/**
 * @brief Saves scrapers settings
 */
void Cinefacts::saveSettings()
{
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager *Cinefacts::qnam()
{
    return &m_qnam;
}

/**
 * @brief Returns a list of infos available from the scraper
 * @return List of supported infos
 */
QList<int> Cinefacts::scraperSupports()
{
    return m_scraperSupports;
}

/**
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @see Cinefacts::searchFinished
 */
void Cinefacts::search(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    QString encodedSearch = Helper::toLatin1PercentEncoding(searchStr);
    QUrl url;
    url.setEncodedUrl(QString("http://www.cinefacts.de/suche/suche.php?name=%1").arg(encodedSearch).toUtf8());
    m_searchReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "searchDone" if there are no more pages in the result set
 * @see Cinefacts::parseSearch
 */
void Cinefacts::searchFinished()
{
    qDebug() << "Entered";
    QList<ScraperSearchResult> results;
    if (m_searchReply->error() == QNetworkReply::NoError ) {
        QString msg = m_searchReply->readAll();
        results = parseSearch(msg);
    } else {
        qWarning() << "Network Error" << m_searchReply->errorString();
    }
    m_searchReply->deleteLater();
    emit searchDone(results);
}

/**
 * @brief Parses the search results
 * @param html Downloaded HTML data
 * @return List of search results
 */
QList<ScraperSearchResult> Cinefacts::parseSearch(QString html)
{
    qDebug() << "Entered";
    QList<ScraperSearchResult> results;
    int pos = 0;
    QRegExp rx("<a href=\"/kino/([0-9]*)/(.[^/]*)/filmdetails.html\">[^<]*<b title=\"([^\"]*)\" class=\"headline\".*([0-9]{4})");
    rx.setMinimal(true);
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.name     = rx.cap(3);
        result.id       = rx.cap(1) + "/" + rx.cap(2);
        result.released = QDate::fromString(rx.cap(4), "yyyy");
        results.append(result);
        pos += rx.matchedLength();
    }
    return results;
}

/**
 * @brief Starts network requests to download infos from Cinefacts
 * @param id Cinefacts movie ID
 * @param movie Movie object
 * @param infos List of infos to load
 * @see Cinefacts::loadFinished
 */
void Cinefacts::loadData(QString id, Movie *movie, QList<int> infos)
{
    qDebug() << "Entered, id=" << id << "movie=" << movie->name();
    m_infosToLoad = infos;
    m_currentMovie = movie;
    QUrl url(QString("http://www.cinefacts.de/kino/%1/filmdetails.html").arg(id));
    m_loadReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(loadFinished()));
}

/**
 * @brief Called when the movie infos are downloaded
 * @see Cinefacts::parseAndAssignInfos
 */
void Cinefacts::loadFinished()
{
    qDebug() << "Entered";
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = m_loadReply->readAll();
        parseAndAssignInfos(msg, m_currentMovie, m_infosToLoad);
    } else {
        qWarning() << "Network Error" << m_loadReply->errorString();
        m_currentMovie->scraperLoadDone();
    }
    m_loadReply->deleteLater();
}

/**
 * @brief Parses HTML data and assigns it to the given movie object
 * @param html HTML data
 * @param movie Movie object
 * @param infos List of infos to load
 */
void Cinefacts::parseAndAssignInfos(QString html, Movie *movie, QList<int> infos)
{
    qDebug() << "Entered";
    m_backdropUrl.clear();
    movie->clear(infos);
    QRegExp rx;
    rx.setMinimal(true);
    int pos = 0;
    QTextDocument doc;

    // Title
    rx.setPattern("<h1>([^<]*)<");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setName(rx.cap(1).trimmed());

    // Original Title
    rx.setPattern("<dt class=\"c1\">Originaltitel:</dt>[^<]*<dd class=\"first\">(.[^<]*)</dd>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setOriginalName(rx.cap(1).trimmed());

    // Genre
    rx.setPattern("Genre:([^:]*)<dt");
    if (infos.contains(MovieScraperInfos::Genres) && rx.indexIn(html) != -1) {
        QString genres = rx.cap(1);
        pos = 0;
        rx.setPattern(">*[ A-Za-z]([^<>]*)</a>");
        while ((pos = rx.indexIn(genres, pos)) != -1) {
            movie->addGenre(rx.cap(1).trimmed());
            pos += rx.matchedLength();
        }
    }

    // Year
    rx.setPattern("</a> ([0-9]*) </dd>");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1)
        movie->setReleased(QDate::fromString(rx.cap(1).trimmed(), "yyyy"));

    // Country
    rx.setPattern("Produktionsland:([^:]*)<dt");
    if (infos.contains(MovieScraperInfos::Countries) && rx.indexIn(html) != -1) {
        QString countries = rx.cap(1);
        pos = 0;
        rx.setPattern("<a href=\"[^\"]*\">([^<]*)</a>");
        while ((pos = rx.indexIn(countries, pos)) != -1) {
            movie->addCountry(rx.cap(1).trimmed());
            pos += rx.matchedLength();
        }
    }

    // Actors
    rx.setPattern("Darsteller:</td>(.*)</table");
    if (infos.contains(MovieScraperInfos::Actors) && rx.indexIn(html) != -1) {
        QString actors = rx.cap(1);
        pos = 0;
        rx.setPattern(">([^<>]*)</a></td>+[^<]+<[^>]+> als([ A-Za-z]*)&nbsp;");
        while ((pos = rx.indexIn(actors, pos)) != -1) {
            Actor actor;
            actor.name = rx.cap(1).trimmed();
            actor.role = rx.cap(2).trimmed();
            movie->addActor(actor);
            pos += rx.matchedLength();
        }
    }

    // Studio
    rx.setPattern("Verleih:([^\\.]*)\\.");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1)
        movie->addStudio(rx.cap(1).trimmed());

    // MPAA
    rx.setPattern("FSK:</dt>[^>]*>ab ([^<]*) Jahren<");
    if (infos.contains(MovieScraperInfos::Certification) && rx.indexIn(html) != -1)
        movie->setCertification("FSK " + rx.cap(1));

    // Runtime
    rx.setPattern("L.nge:</dt>[^>]*>([0-9]*) Minuten");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1)
        movie->setRuntime(rx.cap(1).trimmed().toInt());

    // Overview
    rx.setPattern("KURZINHALT</h2></li>.*<li[^>]*>(.*)</li>");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setOverview(doc.toPlainText());
    }

    // Backdrops
    rx.setPattern("<a href=\"/kino/film/([0-9]*)/0/([^/]*)/szenenbilder_seite_1.html\">");
    if (infos.contains(MovieScraperInfos::Backdrop) && rx.indexIn(html) != -1)
        m_backdropUrl = QUrl(QString("http://www.cinefacts.de/kino/film/%1/0/%2/szenenbilder_seite_1.html").arg(rx.cap(1)).arg(rx.cap(2)));

    // Posters
    rx.setPattern("<a href=\"/kino/film/([0-9]*)/([^/]*)/plakate.html\">");
    if (infos.contains(MovieScraperInfos::Poster) && rx.indexIn(html) != -1) {
        QUrl posterUrl(QString("http://www.cinefacts.de/kino/film/%1/%2/plakate.html").arg(rx.cap(1)).arg(rx.cap(2)));
        m_posterReply = this->qnam()->get(QNetworkRequest(posterUrl));
        connect(m_posterReply, SIGNAL(finished()), this, SLOT(posterFinished()));
        return;
    }

    if (!m_backdropUrl.isEmpty()) {
        m_backdropReply = this->qnam()->get(QNetworkRequest(m_backdropUrl));
        connect(m_backdropReply, SIGNAL(finished()), this, SLOT(backdropFinished()));
        return;
    }

    m_currentMovie->scraperLoadDone();
}

/**
 * @brief Called when poster scraping has finished
 * Starts the next poster download or the backdrop download or tells the movie that scraping is done
 */
void Cinefacts::posterFinished()
{
    qDebug() << "Entered";
    m_posterQueue.clear();
    if (m_posterReply->error() == QNetworkReply::NoError ) {
        QString msg = m_posterReply->readAll();
        int pos = 0;
        QRegExp rx("<a href=\"/kino/film/([^\"]+)\">[^<]*<img");
        rx.setMinimal(true);
        while ((pos = rx.indexIn(msg, pos)) != -1) {
            QUrl url(QString("http://www.cinefacts.de/kino/film/%1").arg(rx.cap(1)));
            m_posterQueue.append(url);
            pos += rx.matchedLength();
        }
        startNextPosterDownload();
    } else if (!m_backdropUrl.isEmpty()) {
        m_backdropReply = this->qnam()->get(QNetworkRequest(m_backdropUrl));
        connect(m_backdropReply, SIGNAL(finished()), this, SLOT(backdropFinished()));
    } else {
        m_currentMovie->scraperLoadDone();
    }
    m_posterReply->deleteLater();
}

/**
 * @brief Called when backdrop scraping has finished
 * Starts the next backdrop download or tells the movie that scraping is done
 */
void Cinefacts::backdropFinished()
{
    qDebug() << "Entered";
    m_backdropQueue.clear();
    if (m_backdropReply->error() == QNetworkReply::NoError ) {
        QString msg = m_backdropReply->readAll();
        int pos = 0;
        QRegExp rx("<a href=\"/kino/film/([^\"]+)\">[^<]*<img");
        rx.setMinimal(true);
        while ((pos = rx.indexIn(msg, pos)) != -1) {
            QUrl url(QString("http://www.cinefacts.de/kino/film/%1").arg(rx.cap(1)));
            m_backdropQueue.append(url);
            pos += rx.matchedLength();
        }
        startNextBackdropDownload();
    } else {
        qWarning() << "Network Error" << m_backdropReply->errorString();
        m_currentMovie->scraperLoadDone();
    }
    m_backdropReply->deleteLater();
}

/**
 * @brief Starts the next poster download
 */
void Cinefacts::startNextPosterDownload()
{
    qDebug() << "Entered";
    if (m_posterQueue.isEmpty()) {
        if (!m_backdropUrl.isEmpty()) {
            m_backdropReply = this->qnam()->get(QNetworkRequest(m_backdropUrl));
            connect(m_backdropReply, SIGNAL(finished()), this, SLOT(backdropFinished()));
        } else {
            m_currentMovie->scraperLoadDone();
        }
        return;
    }
    QUrl url = m_posterQueue.dequeue();
    m_posterSubReply = qnam()->get(QNetworkRequest(url));
    connect(m_posterSubReply, SIGNAL(finished()), this, SLOT(posterSubFinished()));
}

/**
 * @brief Starts the next backdrop download
 */
void Cinefacts::startNextBackdropDownload()
{
    qDebug() << "Entered";
    if (m_backdropQueue.isEmpty()) {
        m_currentMovie->scraperLoadDone();
        return;
    }
    QUrl url = m_backdropQueue.dequeue();
    m_backdropSubReply = qnam()->get(QNetworkRequest(url));
    connect(m_backdropSubReply, SIGNAL(finished()), this, SLOT(backdropSubFinished()));
}

/**
 * @brief Called when a poster download has finished
 */
void Cinefacts::posterSubFinished()
{
    qDebug() << "Entered";
    if (m_posterSubReply->error() == QNetworkReply::NoError ) {
        QString msg = m_posterSubReply->readAll();
        QRegExp rx("src=\"/kino/plakat/([^\"]*)\"");
        rx.setMinimal(true);
        if (rx.indexIn(msg) != -1) {
            Poster p;
            p.thumbUrl = QUrl(QString("http://www.cinefacts.de/kino/plakat/%1").arg(rx.cap(1)));
            rx.setPattern("<a rel=\"shadowbox\" href=\"/kino/plakat/([^\"]*)\"");
            if (rx.indexIn(msg) != -1)
                p.originalUrl = QUrl(QString("http://www.cinefacts.de/kino/plakat/%1").arg(rx.cap(1)));
            else
                p.originalUrl = p.thumbUrl;
            m_currentMovie->addPoster(p);
        }
    } else {
        qWarning() << "Network Error" << m_posterSubReply->errorString();
    }
    m_posterSubReply->deleteLater();
    startNextPosterDownload();
}

/**
 * @brief Called when a backdrop download has finished
 */
void Cinefacts::backdropSubFinished()
{
    qDebug() << "Entered";
    if (m_backdropSubReply->error() == QNetworkReply::NoError ) {
        QString msg = m_backdropSubReply->readAll();
        QRegExp rx("src=\"/kino/bild/([^\"]*)\"");
        rx.setMinimal(true);
        if (rx.indexIn(msg) != -1) {
            Poster p;
            p.thumbUrl = QUrl(QString("http://www.cinefacts.de/kino/bild/%1").arg(rx.cap(1)));
            rx.setPattern("<a rel=\"shadowbox\" href=\"/kino/bild/([^\"]*)\"");
            if (rx.indexIn(msg) != -1)
                p.originalUrl = QUrl(QString("http://www.cinefacts.de/kino/bild/%1").arg(rx.cap(1)));
            else
                p.originalUrl = p.thumbUrl;
            m_currentMovie->addBackdrop(p);
        }
    } else {
        qWarning() << "Network Error" << m_backdropSubReply->errorString();
    }
    m_backdropSubReply->deleteLater();
    startNextBackdropDownload();
}

/**
 * @brief Cinefacts::languages
 * @return
 */
QMap<QString, QString> Cinefacts::languages()
{
    QMap<QString, QString> m;
    return m;
}

/**
 * @brief language
 * @return
 */
QString Cinefacts::language()
{
    return QString();
}

/**
 * @brief Cinefacts::setLanguage
 * @param language
 */
void Cinefacts::setLanguage(QString language)
{
    Q_UNUSED(language);
}
