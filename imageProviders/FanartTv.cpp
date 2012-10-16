#include "FanartTv.h"
#include <QDebug>
#include <QSettings>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include "scrapers/TMDb.h"

/**
 * @brief FanartTv::FanartTv
 * @param parent
 */
FanartTv::FanartTv(QObject *parent)
{
    setParent(parent);
    m_provides << ImageDialogType::MovieBackdrop
               << ImageDialogType::TvShowBanner << ImageDialogType::TvShowBackdrop
               << ImageDialogType::ConcertBackdrop;
    m_tmdbApiKey = "5d832bdf69dcb884922381ab01548d5b";
    m_apiKey = "842f7a5d1cc7396f142b8dd47c4ba42b";
    m_tmdbBaseUrl = "http://cf2.imgobject.com/t/p/";

}

/**
 * @brief Returns the name of this image provider
 * @return Name of this image provider
 */
QString FanartTv::name()
{
    return QString("Fanart.tv");
}

/**
 * @brief Returns a list of supported image types
 * @return List of supported image types
 */
QList<int> FanartTv::provides()
{
    return m_provides;
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager *FanartTv::qnam()
{
    return &m_qnam;
}

/**
 * @brief Loads the setup parameters from TMDb
 * @see FanartTv::setupFinished
 */
void FanartTv::setup()
{
    QUrl url(QString("http://api.themoviedb.org/3/configuration?api_key=%1").arg(m_tmdbApiKey));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    m_setupReply = qnam()->get(request);
    connect(m_setupReply, SIGNAL(finished()), this, SLOT(onSetupFinished()));
}

/**
 * @brief Called when setup parameters were got
 *        Parses json and assigns the baseUrl
 */
void FanartTv::onSetupFinished()
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

    m_tmdbBaseUrl = sc.property("images").property("base_url").toString();
}

/**
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @see FanartTv::searchFinished
 */
void FanartTv::searchMovie(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    QSettings settings;
    m_tmdbLanguage = settings.value("Scrapers/TMDb/Language", "en").toString();
    m_results.clear();
    m_searchString = searchStr;
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrl url(QString("http://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&query=%3").arg(m_tmdbApiKey).arg(m_tmdbLanguage).arg(encodedSearch));
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
void FanartTv::onSearchMovieFinished()
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
        emit sigSearchDone(m_results);
    } else {
        QUrl url(QString("http://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&page=%3&query=%4").arg(m_tmdbApiKey).arg(m_tmdbLanguage).arg(nextPage).arg(m_searchString));
        QNetworkRequest request(url);
        request.setRawHeader("Accept", "application/json");
        m_searchReply = qnam()->get(request);
        connect(m_searchReply, SIGNAL(finished()), this, SLOT(onSearchMovieFinished()));
    }
}

/**
 * @brief Would load movie posters (not supported by fanart.tv)
 * @param tmdbId
 */
void FanartTv::moviePosters(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie backdrops
 * @param tmdbId
 */
void FanartTv::movieBackdrops(QString tmdbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://fanart.tv/webservice/movie/%2/%1/json/all/1/2/").arg(tmdbId).arg(m_apiKey));
    request.setUrl(url);
    m_loadReply = qnam()->get(QNetworkRequest(request));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadBackdropsFinished()));
}

/**
 * @brief Called when the movie posters are downloaded
 * @see TMDbImages::parsePosters
 */
void FanartTv::onLoadBackdropsFinished()
{
    QList<Poster> posters;
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        qDebug() << msg;
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
QList<Poster> FanartTv::parseBackdrops(QString json)
{
    QList<Poster> posters;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    QScriptValueIterator it(sc);
    while (it.hasNext()) {
        it.next();
        QScriptValue v = it.value();
        if (v.property("moviebackground").isArray()) {
            QScriptValueIterator itB(v.property("moviebackground"));
            while (itB.hasNext()) {
                itB.next();
                QScriptValue vB = itB.value();
                if (vB.property("url").toString().isEmpty())
                    continue;
                Poster b;
                b.thumbUrl = vB.property("url").toString() + "/preview";
                b.originalUrl = vB.property("url").toString();
                posters.append(b);
            }
        }
    }

    return posters;
}
