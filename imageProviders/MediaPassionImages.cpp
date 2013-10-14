#include "MediaPassionImages.h"

#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include <QSettings>
#include "scrapers/TMDb.h"

MediaPassionImages::MediaPassionImages(QObject *parent)
{
    setParent(parent);
    m_provides << ImageType::MovieBackdrop << ImageType::MoviePoster << ImageType::MovieCdArt << ImageType::MovieLogo << ImageType::MovieClearArt;
    m_searchResultLimit = 0;
    QSettings settings;
    m_mediaPassion = new MediaPassion(this);
    m_mediaPassion->loadSettings(settings);
    m_dummyMovie = new Movie(QStringList(), this);
    connect(m_dummyMovie->controller(), SIGNAL(sigInfoLoadDone(Movie*)), this, SLOT(onLoadImagesFinished()));
    connect(m_mediaPassion, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchMovieFinished(QList<ScraperSearchResult>)));
}

QString MediaPassionImages::name()
{
    return QString("Media Passion");
}

QString MediaPassionImages::identifier()
{
    return QString("images.mediapassion");
}

QList<int> MediaPassionImages::provides()
{
    return m_provides;
}

void MediaPassionImages::searchMovie(QString searchStr, int limit)
{
    QSettings settings;
    m_mediaPassion->loadSettings(settings);
    m_searchResultLimit = limit;
    m_mediaPassion->search(searchStr);
}

void MediaPassionImages::onSearchMovieFinished(QList<ScraperSearchResult> results)
{
    if (m_searchResultLimit == 0)
        emit sigSearchDone(results);
    else
        emit sigSearchDone(results.mid(0, m_searchResultLimit));
}

void MediaPassionImages::moviePosters(QString id)
{
    QSettings settings;
    m_mediaPassion->loadSettings(settings);
    m_dummyMovie->clear();
    m_imageType = ImageType::MoviePoster;
    QMap<ScraperInterface*, QString> ids;
    ids.insert(0, id);
    m_mediaPassion->loadData(ids, m_dummyMovie, QList<int>() << MovieScraperInfos::Poster);
}

void MediaPassionImages::movieBackdrops(QString id)
{
    QSettings settings;
    m_mediaPassion->loadSettings(settings);
    m_dummyMovie->clear();
    m_imageType = ImageType::MovieBackdrop;
    QMap<ScraperInterface*, QString> ids;
    ids.insert(0, id);
    m_mediaPassion->loadData(ids, m_dummyMovie, QList<int>() << MovieScraperInfos::Backdrop);
}

void MediaPassionImages::movieCdArts(QString id)
{
    QSettings settings;
    m_mediaPassion->loadSettings(settings);
    m_dummyMovie->clear();
    m_imageType = ImageType::MovieCdArt;
    QMap<ScraperInterface*, QString> ids;
    ids.insert(0, id);
    m_mediaPassion->loadData(ids, m_dummyMovie, QList<int>() << MovieScraperInfos::CdArt);
}

void MediaPassionImages::concertPosters(QString id)
{
    Q_UNUSED(id);
}

void MediaPassionImages::concertBackdrops(QString id)
{
    Q_UNUSED(id);
}

void MediaPassionImages::onLoadImagesFinished()
{
    QList<Poster> posters;
    if (m_imageType == ImageType::MovieBackdrop)
        posters = m_dummyMovie->backdrops();
    else if (m_imageType == ImageType::MoviePoster)
        posters = m_dummyMovie->posters();
    else if (m_imageType == ImageType::MovieCdArt)
        posters = m_dummyMovie->discArts();
    else if (m_imageType == ImageType::MovieLogo)
        posters = m_dummyMovie->logos();
    else if (m_imageType == ImageType::MovieClearArt)
        posters = m_dummyMovie->clearArts();

    emit sigImagesLoaded(posters);
}

void MediaPassionImages::movieImages(Movie *movie, QString tmdbId, QList<int> types)
{
    Q_UNUSED(movie);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

void MediaPassionImages::movieLogos(QString id)
{
    QSettings settings;
    m_mediaPassion->loadSettings(settings);
    m_dummyMovie->clear();
    m_imageType = ImageType::MovieLogo;
    QMap<ScraperInterface*, QString> ids;
    ids.insert(0, id);
    m_mediaPassion->loadData(ids, m_dummyMovie, QList<int>() << MovieScraperInfos::Logo);
}

void MediaPassionImages::movieBanners(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void MediaPassionImages::movieThumbs(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void MediaPassionImages::movieClearArts(QString id)
{
    QSettings settings;
    m_mediaPassion->loadSettings(settings);
    m_dummyMovie->clear();
    m_imageType = ImageType::MovieClearArt;
    QMap<ScraperInterface*, QString> ids;
    ids.insert(0, id);
    m_mediaPassion->loadData(ids, m_dummyMovie, QList<int>() << MovieScraperInfos::ClearArt);
}

void MediaPassionImages::searchConcert(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void MediaPassionImages::concertImages(Concert *concert, QString tmdbId, QList<int> types)
{
    Q_UNUSED(concert);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

void MediaPassionImages::concertLogos(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void MediaPassionImages::concertClearArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void MediaPassionImages::concertCdArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void MediaPassionImages::searchTvShow(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void MediaPassionImages::tvShowImages(TvShow *show, QString tvdbId, QList<int> types)
{
    Q_UNUSED(show);
    Q_UNUSED(tvdbId);
    Q_UNUSED(types);
}

void MediaPassionImages::tvShowPosters(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void MediaPassionImages::tvShowBackdrops(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void MediaPassionImages::tvShowLogos(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void MediaPassionImages::tvShowThumbs(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void MediaPassionImages::tvShowClearArts(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void MediaPassionImages::tvShowCharacterArts(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void MediaPassionImages::tvShowBanners(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void MediaPassionImages::tvShowEpisodeThumb(QString tvdbId, int season, int episode)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
}

void MediaPassionImages::tvShowSeason(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void MediaPassionImages::tvShowSeasonBanners(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void MediaPassionImages::tvShowSeasonThumbs(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void MediaPassionImages::tvShowSeasonBackdrops(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}
