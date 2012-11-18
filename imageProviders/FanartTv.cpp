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
    m_provides << ImageDialogType::MovieBackdrop << ImageDialogType::MovieLogo << ImageDialogType::MovieClearArt << ImageDialogType::MovieCdArt
               << ImageDialogType::TvShowClearArt << ImageDialogType::TvShowBackdrop << ImageDialogType::TvShowBanner
               << ImageDialogType::TvShowLogos << ImageDialogType::TvShowCharacterArt
               << ImageDialogType::ConcertBackdrop << ImageDialogType::ConcertLogo << ImageDialogType::ConcertClearArt << ImageDialogType::ConcertCdArt;
    m_apiKey = "842f7a5d1cc7396f142b8dd47c4ba42b";
    m_searchResultLimit = 0;
    m_tvdb = new TheTvDb(this);
    m_tvdb->loadSettings();
    m_tmdb = new TMDb(this);
    m_tmdb->loadSettings();
    connect(m_tvdb, SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchTvShowFinished(QList<ScraperSearchResult>)));
    connect(m_tmdb, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchMovieFinished(QList<ScraperSearchResult>)));
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
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @param limit Number of results, if zero, all results are returned
 * @see FanartTv::onSearchMovieFinished
 */
void FanartTv::searchMovie(QString searchStr, int limit)
{
    m_searchResultLimit = limit;
    m_tmdb->search(searchStr);
}

/**
 * @brief Searches for a concert
 * @param searchStr The Concert name/search string
 * @param limit Number of results, if zero, all results are returned
 * @see FanartTv::searchMovie
 */
void FanartTv::searchConcert(QString searchStr, int limit)
{
    searchMovie(searchStr, limit);
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "sigSearchDone" if there are no more pages in the result set
 * @param results List of results from scraper
 * @see TMDb::parseSearch
 */
void FanartTv::onSearchMovieFinished(QList<ScraperSearchResult> results)
{
    if (m_searchResultLimit == 0)
        emit sigSearchDone(results);
    else
        emit sigSearchDone(results.mid(0, m_searchResultLimit));
}

/**
 * @brief Loads given image types
 * @param movie
 * @param tmdbId
 * @param types
 */
void FanartTv::movieImages(Movie *movie, QString tmdbId, QList<int> types)
{
    m_currentMovie = movie;
    loadMovieData(tmdbId, types);
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
    loadMovieData(tmdbId, TypeBackdrop);
}

/**
 * @brief Load movie logos
 * @param tmdbId The Movie DB id
 */
void FanartTv::movieLogos(QString tmdbId)
{
    loadMovieData(tmdbId, TypeLogo);
}

/**
 * @brief Load movie clear arts
 * @param tmdbId The Movie DB id
 */
void FanartTv::movieClearArts(QString tmdbId)
{
    loadMovieData(tmdbId, TypeClearArt);
}

/**
 * @brief Load movie cd arts
 * @param tmdbId The Movie DB id
 */
void FanartTv::movieCdArts(QString tmdbId)
{
    loadMovieData(tmdbId, TypeCdArt);
}

/**
 * @brief Loads given image types
 * @param concert
 * @param tmdbId
 * @param types
 */
void FanartTv::concertImages(Concert *concert, QString tmdbId, QList<int> types)
{
    m_currentConcert = concert;
    loadConcertData(tmdbId, types);
}

/**
 * @brief Would load concert posters (not supported by fanart.tv)
 * @param tmdbId
 */
void FanartTv::concertPosters(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert backdrops
 * @param tmdbId
 */
void FanartTv::concertBackdrops(QString tmdbId)
{
    loadMovieData(tmdbId, TypeBackdrop);
}

/**
 * @brief Load concert logos
 * @param tmdbId The Movie DB id
 */
void FanartTv::concertLogos(QString tmdbId)
{
    loadMovieData(tmdbId, TypeLogo);
}

/**
 * @brief Load concert clear arts
 * @param tmdbId The Movie DB id
 */
void FanartTv::concertClearArts(QString tmdbId)
{
    loadMovieData(tmdbId, TypeClearArt);
}

/**
 * @brief Load concert cd arts
 * @param tmdbId The Movie DB id
 */
void FanartTv::concertCdArts(QString tmdbId)
{
    loadMovieData(tmdbId, TypeCdArt);
}

/**
 * @brief FanartTv::loadMovieData
 * @param tmdbId
 * @param type
 */
void FanartTv::loadMovieData(QString tmdbId, int type)
{
    m_currentType = type;
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://api.fanart.tv/webservice/movie/%2/%1/json/all/1/2/").arg(tmdbId).arg(m_apiKey));
    request.setUrl(url);
    m_loadReply = qnam()->get(QNetworkRequest(request));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadMovieDataFinished()));
}

/**
 * @brief FanartTv::loadMovieData
 * @param tmdbId
 * @param types
 */
void FanartTv::loadMovieData(QString tmdbId, QList<int> types)
{
    m_currentTypes = types;
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://api.fanart.tv/webservice/movie/%2/%1/json/all/1/2/").arg(tmdbId).arg(m_apiKey));
    request.setUrl(url);
    m_loadReply = qnam()->get(QNetworkRequest(request));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadAllMovieDataFinished()));
}

/**
 * @brief FanartTv::loadConcertData
 * @param tmdbId
 * @param types
 */
void FanartTv::loadConcertData(QString tmdbId, QList<int> types)
{
    m_currentTypes = types;
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://api.fanart.tv/webservice/movie/%2/%1/json/all/1/2/").arg(tmdbId).arg(m_apiKey));
    request.setUrl(url);
    m_loadReply = qnam()->get(QNetworkRequest(request));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadAllConcertDataFinished()));
}

/**
 * @brief Called when the movie images are downloaded
 * @see TMDbImages::parseMovieData
 */
void FanartTv::onLoadMovieDataFinished()
{
    QList<Poster> posters;
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        posters = parseMovieData(msg, m_currentType);
    }
    m_loadReply->deleteLater();
    emit sigImagesLoaded(posters);
}

/**
 * @brief Called when all movie images are downloaded
 * @see TMDbImages::parseMovieData
 */
void FanartTv::onLoadAllMovieDataFinished()
{
    QMap<int, QList<Poster> > posters;
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        foreach (int type, m_currentTypes)
            posters.insert(type, parseMovieData(msg, type));
    }
    m_loadReply->deleteLater();
    emit sigImagesLoaded(m_currentMovie, posters);
}

/**
 * @brief Called when all concert images are downloaded
 * @see TMDbImages::parseMovieData
 */
void FanartTv::onLoadAllConcertDataFinished()
{
    QMap<int, QList<Poster> > posters;
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        foreach (int type, m_currentTypes)
            posters.insert(type, parseMovieData(msg, type));
    }
    m_loadReply->deleteLater();
    emit sigImagesLoaded(m_currentConcert, posters);
}

/**
 * @brief Parses JSON data for movies
 * @param json JSON data
 * @param type Type of image (ImageType)
 * @return List of posters
 */
QList<Poster> FanartTv::parseMovieData(QString json, int type)
{
    QMap<int, QStringList> map;
    map.insert(TypeBackdrop, QStringList() << "moviebackground");
    map.insert(TypeLogo, QStringList() << "hdmovielogo" << "movielogo");
    map.insert(TypeClearArt, QStringList() << "hdmovieclearart" << "movieart");
    map.insert(TypeCdArt, QStringList() << "moviedisc");
    QList<Poster> posters;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    QScriptValueIterator it(sc);
    while (it.hasNext()) {
        it.next();
        QScriptValue v = it.value();
        foreach (const QString &section, map.value(type)) {
            if (v.property(section).isArray()) {
                QScriptValueIterator itB(v.property(section));
                while (itB.hasNext()) {
                    itB.next();
                    QScriptValue vB = itB.value();
                    if (vB.property("url").toString().isEmpty())
                        continue;
                    Poster b;
                    b.thumbUrl = vB.property("url").toString() + "/preview";
                    b.originalUrl = vB.property("url").toString();
                    if (section == "hdmovielogo" || section == "hdmovieclearart")
                        b.hint = "HD";
                    else if (section == "movielogo" || section == "movieart")
                        b.hint = "SD";
                    posters.append(b);
                }
            }
        }
    }

    return posters;
}

/**
 * @brief Searches for a tv show
 * @param searchStr The tv show name/search string
 * @param limit Number of results, if zero, all results are returned
 * @see FanartTv::onSearchTvShowFinished
 */
void FanartTv::searchTvShow(QString searchStr, int limit)
{
    m_searchResultLimit = limit;
    m_tvdb->search(searchStr);
}

/**
 * @brief FanartTv::onSearchTvShowFinished
 * @param results Result list
 */
void FanartTv::onSearchTvShowFinished(QList<ScraperSearchResult> results)
{
    if (m_searchResultLimit == 0)
        emit sigSearchDone(results);
    else
        emit sigSearchDone(results.mid(0, m_searchResultLimit));
}

/**
 * @brief Loads given image types
 * @param show
 * @param tvdbId
 * @param types
 */
void FanartTv::tvShowImages(TvShow *show, QString tvdbId, QList<int> types)
{
    m_currentShow = show;
    loadTvShowData(tvdbId, types);
}

/**
 * @brief FanartTv::loadTvShowData
 * @param tvdbId The Tv DB Id
 * @param type
 */
void FanartTv::loadTvShowData(QString tvdbId, int type)
{
    m_currentType = type;
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://api.fanart.tv/webservice/series/%2/%1/json/all/1/2/").arg(tvdbId).arg(m_apiKey));
    request.setUrl(url);
    m_loadReply = qnam()->get(QNetworkRequest(request));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadTvShowDataFinished()));
}

/**
 * @brief FanartTv::loadTvShowData
 * @param tvdbId The Tv DB Id
 * @param types
 */
void FanartTv::loadTvShowData(QString tvdbId, QList<int> types)
{
    m_currentTypes = types;
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://api.fanart.tv/webservice/series/%2/%1/json/all/1/2/").arg(tvdbId).arg(m_apiKey));
    request.setUrl(url);
    m_loadReply = qnam()->get(QNetworkRequest(request));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadAllTvShowDataFinished()));
}

/**
 * @brief Called when the tv show images are downloaded
 * @see TMDbImages::parseTvShowData
 */
void FanartTv::onLoadTvShowDataFinished()
{
    QList<Poster> posters;
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        posters = parseTvShowData(msg, m_currentType);
    }
    m_loadReply->deleteLater();
    emit sigImagesLoaded(posters);
}

/**
 * @brief Called when all tv show images are downloaded
 * @see TMDbImages::parseTvShowData
 */
void FanartTv::onLoadAllTvShowDataFinished()
{
    QMap<int, QList<Poster> > posters;
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        foreach (int type, m_currentTypes)
            posters.insert(type, parseTvShowData(msg, type));
    }
    m_loadReply->deleteLater();
    emit sigImagesLoaded(m_currentShow, posters);
}

/**
 * @brief Load tv show posters
 * @param tvdbId The TV DB id
 */
void FanartTv::tvShowPosters(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load tv show backdrops
 * @param tvdbId The TV DB id
 */
void FanartTv::tvShowBackdrops(QString tvdbId)
{
    loadTvShowData(tvdbId, TypeBackdrop);
}

/**
 * @brief Load tv show logos
 * @param tvdbId The TV DB id
 */
void FanartTv::tvShowLogos(QString tvdbId)
{
    loadTvShowData(tvdbId, TypeLogo);
}

/**
 * @brief Load tv show clear arts
 * @param tvdbId The TV DB id
 */
void FanartTv::tvShowClearArts(QString tvdbId)
{
    loadTvShowData(tvdbId, TypeClearArt);
}

/**
 * @brief Load tv show character arts
 * @param tvdbId The TV DB id
 */
void FanartTv::tvShowCharacterArts(QString tvdbId)
{
    loadTvShowData(tvdbId, TypeCharacterArt);
}

/**
 * @brief Load tv show banners
 * @param tvdbId The TV DB id
 */
void FanartTv::tvShowBanners(QString tvdbId)
{
    loadTvShowData(tvdbId, TypeBanner);
}

/**
 * @brief Load tv show thumbs
 * @param tvdbId The TV DB id
 * @param season Season number
 * @param episode Episode number
 */
void FanartTv::tvShowThumb(QString tvdbId, int season, int episode)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
}

/**
 * @brief Load tv show season
 * @param tvdbId The TV DB id
 * @param season Season number
 */
void FanartTv::tvShowSeason(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

/**
 * @brief Parses JSON data for tv shows
 * @param json JSON data
 * @param type Type of image (ImageType)
 * @return List of posters
 */
QList<Poster> FanartTv::parseTvShowData(QString json, int type)
{
    QMap<int, QStringList> map;
    map.insert(TypeBackdrop, QStringList() << "showbackground");
    map.insert(TypeLogo, QStringList() << "hdtvlogo" << "clearlogo");
    map.insert(TypeClearArt, QStringList() << "hdclearart" << "clearart");
    map.insert(TypeBanner, QStringList() << "tvbanner");
    map.insert(TypeCharacterArt, QStringList() << "characterart");
    QList<Poster> posters;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    QScriptValueIterator it(sc);
    while (it.hasNext()) {
        it.next();
        QScriptValue v = it.value();
        foreach (const QString &section, map.value(type)) {
            if (v.property(section).isArray()) {
                QScriptValueIterator itB(v.property(section));
                while (itB.hasNext()) {
                    itB.next();
                    QScriptValue vB = itB.value();
                    if (vB.property("url").toString().isEmpty())
                        continue;
                    Poster b;
                    b.thumbUrl = vB.property("url").toString() + "/preview";
                    b.originalUrl = vB.property("url").toString();
                    if (section == "hdtvlogo" || section == "hdclearart")
                        b.hint = "HD";
                    else if (section == "clearlogo" || section == "clearart")
                        b.hint = "SD";
                    posters.append(b);
                }
            }
        }
    }

    return posters;
}
