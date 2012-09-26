#include "TMDb.h"

#include <QDebug>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include <QSettings>

#include "globals/Globals.h"
#include "globals/Helper.h"

/**
 * @brief TMDb::TMDb
 * @param parent
 */
TMDb::TMDb(QObject *parent)
{
    setParent(parent);
    m_apiKey = "5d832bdf69dcb884922381ab01548d5b";
    m_language = "en";

    m_scraperSupports << MovieScraperInfos::Title
                      << MovieScraperInfos::Tagline
                      << MovieScraperInfos::Rating
                      << MovieScraperInfos::Released
                      << MovieScraperInfos::Runtime
                      << MovieScraperInfos::Certification
                      << MovieScraperInfos::Trailer
                      << MovieScraperInfos::Overview
                      << MovieScraperInfos::Poster
                      << MovieScraperInfos::Backdrop
                      << MovieScraperInfos::Actors
                      << MovieScraperInfos::Genres
                      << MovieScraperInfos::Studios
                      << MovieScraperInfos::Countries;

    m_baseUrl = "http://cf2.imgobject.com/t/p/";
    setup();
}

TMDb::~TMDb()
{
}

/**
 * @brief languages
 * @return
 */
QMap<QString, QString> TMDb::languages()
{
    QMap<QString, QString> m;

    m.insert(tr("Chinese"), "zh");
    m.insert(tr("Croatian"), "hr");
    m.insert(tr("Czech"), "cs");
    m.insert(tr("Danish"), "da");
    m.insert(tr("Dutch"), "nl");
    m.insert(tr("English"), "en");
    m.insert(tr("Finnish"), "fi");
    m.insert(tr("French"), "fr");
    m.insert(tr("German"), "de");
    m.insert(tr("Greek"), "el");
    m.insert(tr("Hebrew"), "he");
    m.insert(tr("Hungarian"), "hu");
    m.insert(tr("Italian"), "it");
    m.insert(tr("Japanese"), "ja");
    m.insert(tr("Korean"), "ko");
    m.insert(tr("Norwegian"), "no");
    m.insert(tr("Polish"), "pl");
    m.insert(tr("Portuguese"), "pt");
    m.insert(tr("Russian"), "ru");
    m.insert(tr("Slovene"), "sl");
    m.insert(tr("Spanish"), "es");
    m.insert(tr("Swedish"), "sv");
    m.insert(tr("Turkish"), "tr");

    return m;
}

/**
 * @brief language
 * @return
 */
QString TMDb::language()
{
    return m_language;
}

/**
 * @brief Returns the name of the scraper
 * @return Name of the Scraper
 */
QString TMDb::name()
{
    return QString("The Movie DB");
}

/**
 * @brief Returns if the scraper has settings
 * @return Scraper has settings
 */
bool TMDb::hasSettings()
{
    return true;
}

/**
 * @brief Loads scrapers settings
 */
void TMDb::loadSettings()
{
    QSettings settings;
    m_language = settings.value("Scrapers/TMDb/Language", "en").toString();
}

/**
 * @brief Saves scrapers settings
 */
void TMDb::saveSettings()
{
    QSettings settings;
    settings.setValue("Scrapers/TMDb/Language", m_language);
}

/**
 * @brief TMDb::setLanguage
 * @param language
 */
void TMDb::setLanguage(QString language)
{
    m_language = language;
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager *TMDb::qnam()
{
    return &m_qnam;
}

/**
 * @brief Returns a list of infos available from the scraper
 * @return List of supported infos
 */
QList<int> TMDb::scraperSupports()
{
    return m_scraperSupports;
}

/**
 * @brief Loads the setup parameters from TMDb
 * @see TMDb::setupFinished
 */
void TMDb::setup()
{
    QUrl url(QString("http://api.themoviedb.org/3/configuration?api_key=%1").arg(m_apiKey));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    m_setupReply = this->qnam()->get(request);
    connect(m_setupReply, SIGNAL(finished()), this, SLOT(setupFinished()));
}

/**
 * @brief Called when setup parameters were got
 *        Parses json and assigns the baseUrl
 */
void TMDb::setupFinished()
{
    if (m_setupReply->error() != QNetworkReply::NoError ) {
        m_setupReply->deleteLater();
        return;
    }
    QString msg = QString::fromUtf8(m_setupReply->readAll());
    m_setupReply->deleteLater();
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(msg) + ")");

    m_baseUrl = sc.property("images").property("base_url").toString();
}

/**
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @see TMDb::searchFinished
 */
void TMDb::search(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    m_results.clear();
    m_searchString = searchStr;
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrl url(QString("http://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&query=%3").arg(m_apiKey).arg(m_language).arg(encodedSearch));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    m_searchReply = this->qnam()->get(request);
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "searchDone" if there are no more pages in the result set
 * @see TMDb::parseSearch
 */
void TMDb::searchFinished()
{
    qDebug() << "Entered";
    QList<ScraperSearchResult> results;
    if (m_searchReply->error() != QNetworkReply::NoError ) {
        qWarning() << "Network Error" << m_searchReply->errorString();
        m_searchReply->deleteLater();
        emit searchDone(results);
        return;
    }

    QString msg = QString::fromUtf8(m_searchReply->readAll());
    int nextPage = -1;
    results = parseSearch(msg, &nextPage);
    m_results.append(results);
    m_searchReply->deleteLater();

    if (nextPage == -1) {
        emit searchDone(m_results);
    } else {
        QUrl url(QString("http://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&page=%3&query=%4").arg(m_apiKey).arg(m_language).arg(nextPage).arg(m_searchString));
        QNetworkRequest request(url);
        request.setRawHeader("Accept", "application/json");
        m_searchReply = this->qnam()->get(request);
        connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
    }
}

/**
 * @brief Parses the JSON search results
 * @param json JSON string
 * @param nextPage This will hold the next page to get, -1 if there are no more pages
 * @return List of search results
 */
QList<ScraperSearchResult> TMDb::parseSearch(QString json, int *nextPage)
{
    qDebug() << "Entered";
    QList<ScraperSearchResult> results;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    // only get the first 3 pages
    if (sc.property("page").toInteger() < sc.property("total_pages").toInteger() && sc.property("page").toInteger() < 3)
        *nextPage = sc.property("page").toInteger()+1;

    if (sc.property("results").isArray() ) {
        QScriptValueIterator it(sc.property("results"));
        while (it.hasNext() ) {
            it.next();
            if (it.value().property("id").toString().isEmpty()) {
                continue;
            }
            ScraperSearchResult result;
            result.name     = it.value().property("title").toString();
            if (result.name.isEmpty())
                it.value().property("original_title").toString();
            result.id       = it.value().property("id").toString();
            result.released = QDate::fromString(it.value().property("release_date").toString(), "yyyy-MM-dd");
            results.append(result);
        }
    }

    return results;
}

/**
 * @brief Starts network requests to download infos from TMDb
 * @param id TMDb movie ID
 * @param movie Movie object
 * @param infos List of infos to load
 * @see TMDb::loadFinished
 * @see TMDb::loadCastsFinished
 * @see TMDb::loadTrailersFinished
 * @see TMDb::loadImagesFinished
 * @see TMDb::loadReleasesFinished
 */
void TMDb::loadData(QString id, Movie *movie, QList<int> infos)
{
    qDebug() << "Entered, id=" << id << "movie=" << movie->name();
    m_infosToLoad = infos;
    m_currentMovie = movie;
    m_currentMovie->clear(infos);
    m_currentId = id;
    m_loadDoneFired = false;
    m_loadsLeft.clear();

    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");

    // Infos
    m_loadsLeft.append(DataInfos);
    url.setUrl(QString("http://api.themoviedb.org/3/movie/%1?api_key=%2&language=%3").arg(id).arg(m_apiKey).arg(m_language));
    request.setUrl(url);
    m_loadReply = this->qnam()->get(QNetworkRequest(request));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(loadFinished()));

    // Casts
    if (m_infosToLoad.contains(MovieScraperInfos::Actors)) {
        m_loadsLeft.append(DataCasts);
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/casts?api_key=%2").arg(m_currentId).arg(m_apiKey));
        request.setUrl(url);
        m_castsReply = this->qnam()->get(QNetworkRequest(request));
        connect(m_castsReply, SIGNAL(finished()), this, SLOT(loadCastsFinished()));
    }

    // Trailers
    if (m_infosToLoad.contains(MovieScraperInfos::Trailer)) {
        m_loadsLeft.append(DataTrailers);
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/trailers?api_key=%2").arg(m_currentId).arg(m_apiKey));
        request.setUrl(url);
        m_trailersReply = this->qnam()->get(QNetworkRequest(request));
        connect(m_trailersReply, SIGNAL(finished()), this, SLOT(loadTrailersFinished()));
    }

    // Images
    if (m_infosToLoad.contains(MovieScraperInfos::Poster) || m_infosToLoad.contains(MovieScraperInfos::Backdrop)) {
        m_loadsLeft.append(DataImages);
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/images?api_key=%2").arg(m_currentId).arg(m_apiKey));
        request.setUrl(url);
        m_imagesReply = this->qnam()->get(QNetworkRequest(request));
        connect(m_imagesReply, SIGNAL(finished()), this, SLOT(loadImagesFinished()));
    }

    // Releases
    if (m_infosToLoad.contains(MovieScraperInfos::Certification)) {
        m_loadsLeft.append(DataReleases);
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/releases?api_key=%2").arg(m_currentId).arg(m_apiKey));
        request.setUrl(url);
        m_releasesReply = this->qnam()->get(QNetworkRequest(request));
        connect(m_releasesReply, SIGNAL(finished()), this, SLOT(loadReleasesFinished()));
    }
}

/**
 * @brief Called when the movie infos are downloaded
 * @see TMDb::parseAndAssignInfos
 */
void TMDb::loadFinished()
{
    qDebug() << "Entered";
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        parseAndAssignInfos(msg, m_currentMovie, m_infosToLoad);
    } else {
        qWarning() << "Network Error (load)" << m_loadReply->errorString();
    }
    m_loadReply->deleteLater();
    m_loadsLeft.removeOne(DataInfos);
    checkDownloadsFinished();
}

/**
 * @brief Called when the movie casts are downloaded
 * @see TMDb::parseAndAssignInfos
 */
void TMDb::loadCastsFinished()
{
    qDebug() << "Entered";
    if (m_castsReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_castsReply->readAll());
        parseAndAssignInfos(msg, m_currentMovie, m_infosToLoad);
    } else {
        qWarning() << "Network Error (casts)" << m_castsReply->errorString();
    }
    m_castsReply->deleteLater();
    m_loadsLeft.removeOne(DataCasts);
    checkDownloadsFinished();
}

/**
 * @brief Called when the movie trailers are downloaded
 * @see TMDb::parseAndAssignInfos
 */
void TMDb::loadTrailersFinished()
{
    qDebug() << "Entered";
    if (m_trailersReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_trailersReply->readAll());
        parseAndAssignInfos(msg, m_currentMovie, m_infosToLoad);
    } else {
        qDebug() << "Network Error (trailers)" << m_trailersReply->errorString();
    }
    m_trailersReply->deleteLater();
    m_loadsLeft.removeOne(DataTrailers);
    checkDownloadsFinished();
}

/**
 * @brief Called when the movie images are downloaded
 * @see TMDb::parseAndAssignInfos
 */
void TMDb::loadImagesFinished()
{
    qDebug() << "Entered";
    if (m_imagesReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_imagesReply->readAll());
        parseAndAssignInfos(msg, m_currentMovie, m_infosToLoad);
    } else {
        qWarning() << "Network Error (images)" << m_imagesReply->errorString();
    }
    m_imagesReply->deleteLater();
    m_loadsLeft.removeOne(DataImages);
    checkDownloadsFinished();
}

/**
 * @brief Called when the movie releases are downloaded
 * @see TMDb::parseAndAssignInfos
 */
void TMDb::loadReleasesFinished()
{
    qDebug() << "Entered";
    if (m_releasesReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_releasesReply->readAll());
        parseAndAssignInfos(msg, m_currentMovie, m_infosToLoad);
    } else {
        qWarning() << "Network Error (releases)" << m_releasesReply->errorString();
    }
    m_releasesReply->deleteLater();
    m_loadsLeft.removeOne(DataReleases);
    checkDownloadsFinished();
}

/**
 * @brief Called when one of the movie infos has finished loading
 *        Checks if there are downloads left. If all downloads have finished
 *        the movie object is told that the scraper has finished loading
 */
void TMDb::checkDownloadsFinished()
{
    qDebug() << "Entered";
    m_mutex.lock();
    if (m_loadsLeft.isEmpty() && !m_loadDoneFired) {
        m_loadDoneFired = true;
        m_currentMovie->scraperLoadDone();
    }
    m_mutex.unlock();
}

/**
 * @brief Parses JSON data and assigns it to the given movie object
 *        Handles all types of data from TMDb (info, releases, trailers, casts, images)
 * @param json JSON data
 * @param movie Movie object
 * @param infos List of infos to load
 */
void TMDb::parseAndAssignInfos(QString json, Movie *movie, QList<int> infos)
{
    qDebug() << "Entered";
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    // Infos
    if (infos.contains(MovieScraperInfos::Title) && sc.property("title").isValid())
        movie->setName(sc.property("title").toString());
    if (infos.contains(MovieScraperInfos::Title) && sc.property("original_title").isValid())
        movie->setOriginalName(sc.property("original_title").toString());
    if (infos.contains(MovieScraperInfos::Overview) && sc.property("overview").isValid() && !sc.property("overview").isNull())
        movie->setOverview(sc.property("overview").toString());
    if (infos.contains(MovieScraperInfos::Rating) && sc.property("vote_average").isValid())
        movie->setRating(sc.property("vote_average").toNumber());
    if (infos.contains(MovieScraperInfos::Tagline) && sc.property("tagline").isValid() && !sc.property("tagline").isNull())
       movie->setTagline(sc.property("tagline").toString());
    if (infos.contains(MovieScraperInfos::Released) && sc.property("release_date").isValid())
        movie->setReleased(QDate::fromString(sc.property("release_date").toString(), "yyyy-MM-dd"));
    if (infos.contains(MovieScraperInfos::Runtime) && sc.property("runtime").isValid())
        movie->setRuntime(sc.property("runtime").toInteger());
    if (infos.contains(MovieScraperInfos::Genres) && sc.property("genres").isArray()) {
        QScriptValueIterator itC(sc.property("genres"));
        while (itC.hasNext()) {
            itC.next();
            QScriptValue vC = itC.value();
            if (vC.property("id").toString().isEmpty())
                continue;
            movie->addGenre(vC.property("name").toString());
        }
    }
    if (infos.contains(MovieScraperInfos::Studios) && sc.property("production_companies").isArray()) {
        QScriptValueIterator itS(sc.property("production_companies"));
        while (itS.hasNext()) {
            itS.next();
            QScriptValue vS = itS.value();
            if (vS.property("id").toString().isEmpty())
                continue;
            movie->addStudio(vS.property("name").toString());
        }
    }
    if (infos.contains(MovieScraperInfos::Countries) && sc.property("production_countries").isArray()) {
        QScriptValueIterator itC(sc.property("production_countries"));
        while (itC.hasNext()) {
            itC.next();
            QScriptValue vC = itC.value();
            if (vC.property("name").toString().isEmpty())
                continue;
            movie->addCountry(vC.property("name").toString());
        }
    }

    // Casts
    if (infos.contains(MovieScraperInfos::Actors) && sc.property("cast").isArray()) {
        QScriptValueIterator itC(sc.property("cast"));
        while (itC.hasNext()) {
            itC.next();
            QScriptValue vC = itC.value();
            if (vC.property("name").toString().isEmpty())
                continue;
            Actor a;
            a.name = vC.property("name").toString();
            a.role = vC.property("character").toString();
            if (!vC.property("profile_path").isNull())
                a.thumb = m_baseUrl + "original" + vC.property("profile_path").toString();
            movie->addActor(a);
        }
    }

    // Trailers
    if (infos.contains(MovieScraperInfos::Trailer) && sc.property("youtube").isArray()) {
        QScriptValueIterator itC(sc.property("youtube"));
        while (itC.hasNext()) {
            itC.next();
            QScriptValue vC = itC.value();
            if (vC.property("source").toString().isEmpty())
                continue;
            movie->setTrailer(QUrl(Helper::formatTrailerUrl(QString("http://www.youtube.com/watch?v=%1").arg(vC.property("source").toString()))));
            break;
        }
    }

    // Images
    if (infos.contains(MovieScraperInfos::Backdrop) && sc.property("backdrops").isArray()) {
        QScriptValueIterator itB(sc.property("backdrops"));
        while (itB.hasNext()) {
            itB.next();
            QScriptValue vB = itB.value();
            if (vB.property("file_path").toString().isEmpty())
                continue;
            Poster b;
            b.thumbUrl = m_baseUrl + "w780" + vB.property("file_path").toString();
            b.originalUrl = m_baseUrl + "original" + vB.property("file_path").toString();
            b.originalSize.setWidth(vB.property("width").toString().toInt());
            b.originalSize.setHeight(vB.property("height").toString().toInt());
            movie->addBackdrop(b);
        }
    }

    if (infos.contains(MovieScraperInfos::Poster) && sc.property("posters").isArray()) {
        QScriptValueIterator itB(sc.property("posters"));
        while (itB.hasNext()) {
            itB.next();
            QScriptValue vB = itB.value();
            if (vB.property("file_path").toString().isEmpty())
                continue;
            Poster b;
            b.thumbUrl = m_baseUrl + "w342" + vB.property("file_path").toString();
            b.originalUrl = m_baseUrl + "original" + vB.property("file_path").toString();
            b.originalSize.setWidth(vB.property("width").toString().toInt());
            b.originalSize.setHeight(vB.property("height").toString().toInt());
            movie->addPoster(b);
        }
    }

    // Releases
    if (infos.contains(MovieScraperInfos::Certification) && sc.property("countries").isArray()) {
        QString locale;
        QString us;
        QString gb;
        QScriptValueIterator itB(sc.property("countries"));
        while (itB.hasNext()) {
            itB.next();
            QScriptValue vB = itB.value();
            if (vB.property("iso_3166_1").toString() == "US")
                us = vB.property("certification").toString();
            if (vB.property("iso_3166_1").toString() == "GB")
                gb = vB.property("certification").toString();
            if (vB.property("iso_3166_1").toString().toLower() == m_language)
                locale = vB.property("certification").toString();
        }
        if (!locale.isEmpty())
            movie->setCertification(locale);
        else if (!us.isEmpty())
            movie->setCertification(us);
        else if (!gb.isEmpty())
            movie->setCertification(gb);
    }

}
