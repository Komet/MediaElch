#include "TMDbImages.h"

#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include <QSettings>
#include "scrapers/TMDb.h"

/**
 * @brief TMDbImages::TMDbImages
 * @param parent
 */
TMDbImages::TMDbImages(QObject *parent)
{
    setParent(parent);
    m_apiKey = "5d832bdf69dcb884922381ab01548d5b";
    m_baseUrl = "http://cf2.imgobject.com/t/p/";
    m_provides << ImageDialogType::MovieBackdrop << ImageDialogType::MoviePoster;
    m_searchResultLimit = 0;
    setup();
}

/**
 * @brief Returns an instance of TMDbImages
 * @param parent Parent widget (used the first time for constructing)
 * @return Instance of TMDbImages
 */
TMDbImages *TMDbImages::instance(QObject *parent)
{
    static TMDbImages *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new TMDbImages(parent);
    }
    return m_instance;
}

/**
 * @brief Returns the name of this image provider
 * @return Name of this image provider
 */
QString TMDbImages::name()
{
    return QString("The Movie DB");
}

/**
 * @brief Returns a list of supported image types
 * @return List of supported image types
 */
QList<int> TMDbImages::provides()
{
    return m_provides;
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager *TMDbImages::qnam()
{
    return &m_qnam;
}

/**
 * @brief Loads the setup parameters from TMDb
 * @see TMDbImages::setupFinished
 */
void TMDbImages::setup()
{
    QUrl url(QString("http://api.themoviedb.org/3/configuration?api_key=%1").arg(m_apiKey));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    m_setupReply = qnam()->get(request);
    connect(m_setupReply, SIGNAL(finished()), this, SLOT(onSetupFinished()));
}

/**
 * @brief Called when setup parameters were got
 *        Parses json and assigns the baseUrl
 */
void TMDbImages::onSetupFinished()
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
 * @param limit Number of results, if zero, all results are returned
 * @see TMDbImages::searchFinished
 */
void TMDbImages::searchMovie(QString searchStr, int limit)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    QSettings settings;
    m_language = settings.value("Scrapers/TMDb/Language", "en").toString();
    m_results.clear();
    m_searchString = searchStr;
    m_searchResultLimit = limit;
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrl url(QString("http://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&query=%3").arg(m_apiKey).arg(m_language).arg(encodedSearch));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    m_searchReply = qnam()->get(request);
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(onSearchMovieFinished()));
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "sigSearchDone" if there are no more pages in the result set
 * @see TMDb::parseSearch
 */
void TMDbImages::onSearchMovieFinished()
{
    qDebug() << "Entered";
    QList<ScraperSearchResult> results;
    if (m_searchReply->error() != QNetworkReply::NoError ) {
        qWarning() << "Network Error" << m_searchReply->errorString();
        m_searchReply->deleteLater();
        emit sigSearchDone(results);
        return;
    }

    QString msg = QString::fromUtf8(m_searchReply->readAll());
    int nextPage = -1;
    results = TMDb::parseSearch(msg, &nextPage);
    m_results.append(results);
    m_searchReply->deleteLater();

    if (nextPage == -1) {
        if (m_searchResultLimit != 0)
            emit sigSearchDone(m_results.mid(0, m_searchResultLimit));
        else
            emit sigSearchDone(m_results);
    } else {
        QUrl url(QString("http://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&page=%3&query=%4").arg(m_apiKey).arg(m_language).arg(nextPage).arg(m_searchString));
        QNetworkRequest request(url);
        request.setRawHeader("Accept", "application/json");
        m_searchReply = qnam()->get(request);
        connect(m_searchReply, SIGNAL(finished()), this, SLOT(onSearchMovieFinished()));
    }
}

/**
 * @brief Load movie posters
 * @param tmdbId
 */
void TMDbImages::moviePosters(QString tmdbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/images?api_key=%2").arg(tmdbId).arg(m_apiKey));
    request.setUrl(url);
    m_loadReply = qnam()->get(QNetworkRequest(request));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadPostersFinished()));
}

/**
 * @brief Load movie backdrops
 * @param tmdbId
 */
void TMDbImages::movieBackdrops(QString tmdbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/images?api_key=%2").arg(tmdbId).arg(m_apiKey));
    request.setUrl(url);
    m_loadReply = qnam()->get(QNetworkRequest(request));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadBackdropsFinished()));
}

/**
 * @brief Called when the movie posters are downloaded
 * @see TMDbImages::parsePosters
 */
void TMDbImages::onLoadPostersFinished()
{
    QList<Poster> posters;
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        posters = parsePosters(msg);
    }
    m_loadReply->deleteLater();
    emit sigImagesLoaded(posters);
}

/**
 * @brief Called when the movie backdrops are downloaded
 * @see TMDbImages::parseBackdrops
 */
void TMDbImages::onLoadBackdropsFinished()
{
    QList<Poster> posters;
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        posters = parseBackdrops(msg);
    }
    m_loadReply->deleteLater();
    emit sigImagesLoaded(posters);
}

/**
 * @brief Parses JSON data
 * @param json JSON data
 * @return List of posters
 */
QList<Poster> TMDbImages::parsePosters(QString json)
{
    QList<Poster> posters;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    if (sc.property("posters").isArray()) {
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
            posters.append(b);
        }
    }

    return posters;
}

/**
 * @brief Parses JSON data
 * @param json JSON data
 * @return List of posters
 */
QList<Poster> TMDbImages::parseBackdrops(QString json)
{
    QList<Poster> posters;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    if (sc.property("backdrops").isArray()) {
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
            posters.append(b);
        }
    }

    return posters;
}

/**
 * @brief Load movie logos
 * @param tmdbId The Movie DB id
 */
void TMDbImages::movieLogos(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie clear arts
 * @param tmdbId The Movie DB id
 */
void TMDbImages::movieClearArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie cd arts
 * @param tmdbId The Movie DB id
 */
void TMDbImages::movieCdArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}
