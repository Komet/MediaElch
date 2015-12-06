#include "FanartTvMusicArtists.h"
#include <QDebug>
#include <QSettings>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include "data/Storage.h"
#include "imageProviders/FanartTv.h"
#include "scrapers/TMDb.h"

/**
 * @brief FanartTvMusicArtists::FanartTv
 * @param parent
 */
FanartTvMusicArtists::FanartTvMusicArtists(QObject *parent)
{
    setParent(parent);
    m_provides << ImageType::ConcertBackdrop << ImageType::ConcertLogo;
    m_apiKey = "842f7a5d1cc7396f142b8dd47c4ba42b";
    m_searchResultLimit = 0;
    m_language = "en";
    m_preferredDiscType = "BluRay";
}

/**
 * @brief Returns the name of this image provider
 * @return Name of this image provider
 */
QString FanartTvMusicArtists::name()
{
    return QString("Fanart.tv Music Artists");
}

QUrl FanartTvMusicArtists::siteUrl()
{
    return QUrl("https://fanart.tv");
}

QString FanartTvMusicArtists::identifier()
{
    return QString("images.fanarttv-music");
}

/**
 * @brief Returns a list of supported image types
 * @return List of supported image types
 */
QList<int> FanartTvMusicArtists::provides()
{
    return m_provides;
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager *FanartTvMusicArtists::qnam()
{
    return &m_qnam;
}


/**
 * @brief Searches for an artist
 * @param searchStr The Artist name/search string
 * @param limit Number of results, if zero, all results are returned
 * @see FanartTvMusicArtists::searchMovie
 */
void FanartTvMusicArtists::searchConcert(QString searchStr, int limit)
{
    Q_UNUSED(limit);
    QUrl url(QString("http://www.musicbrainz.org/ws/2/artist/?query=artist:%1").arg(QString(QUrl::toPercentEncoding(searchStr))));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchArtistFinished()));
}

void FanartTvMusicArtists::onSearchArtistFinished()
{
    QList<ScraperSearchResult> results;
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        QDomDocument domDoc;
        domDoc.setContent(msg);
        for (int i=0, n=domDoc.elementsByTagName("artist").count() ; i<n ; ++i) {
            QDomElement elem = domDoc.elementsByTagName("artist").at(i).toElement();
            QString name;
            if (!elem.elementsByTagName("name").isEmpty())
                name = elem.elementsByTagName("name").at(0).toElement().text();
            if (!elem.elementsByTagName("disambiguation").isEmpty())
                name.append(QString(" (%1)").arg(elem.elementsByTagName("disambiguation").at(0).toElement().text()));

            if (!name.isEmpty() && !elem.attribute("id").isEmpty()) {
                ScraperSearchResult result;
                result.id = elem.attribute("id");
                result.name = name;
                results.append(result);
            }
        }
    }
    emit sigSearchDone(results);
}

void FanartTvMusicArtists::concertBackdrops(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/music/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infoToLoad", ImageType::ConcertBackdrop);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadConcertFinished()));
}

void FanartTvMusicArtists::concertLogos(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/music/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infoToLoad", ImageType::ConcertLogo);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadConcertFinished()));
}

void FanartTvMusicArtists::onLoadConcertFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    int info = reply->property("infoToLoad").toInt();
    reply->deleteLater();
    QList<Poster> posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        posters = parseData(msg, info);
    }
    emit sigImagesLoaded(posters);
}

QList<Poster> FanartTvMusicArtists::parseData(QString json, int type)
{
    QMap<int, QStringList> map;
    map.insert(ImageType::ConcertBackdrop, QStringList() << "artistbackground");
    map.insert(ImageType::ConcertLogo, QStringList() << "hdmusiclogo" << "musiclogo");
    QList<Poster> posters;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    foreach (const QString &section, map.value(type)) {
        if (sc.property(section).isArray()) {
            QScriptValueIterator itB(sc.property(section));
            while (itB.hasNext()) {
                itB.next();
                QScriptValue vB = itB.value();
                if (vB.property("url").toString().isEmpty())
                    continue;
                Poster b;
                b.thumbUrl = vB.property("url").toString().replace("/fanart/", "/preview/");
                b.originalUrl = vB.property("url").toString();
                if (section == "hdmusiclogo")
                    b.hint = "HD";
                else if (section == "musiclogo")
                    b.hint = "SD";
                else if (vB.property("disc_type").toString() == "bluray")
                    b.hint = "BluRay";
                else if (vB.property("disc_type").toString() == "dvd")
                    b.hint = "DVD";
                else if (vB.property("disc_type").toString() == "3d")
                    b.hint = "3D";
                b.language = vB.property("lang").toString();
                FanartTv::insertPoster(posters, b, m_language, m_preferredDiscType);
            }
        }
    }

    return posters;
}

void FanartTvMusicArtists::searchTvShow(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusicArtists::tvShowImages(TvShow *show, QString tvdbId, QList<int> types)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(show);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::tvShowPosters(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowBackdrops(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowLogos(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowThumbs(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowClearArts(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowCharacterArts(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowBanners(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowEpisodeThumb(QString tvdbId, int season, int episode)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
}

void FanartTvMusicArtists::tvShowSeason(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusicArtists::tvShowSeasonBanners(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusicArtists::tvShowSeasonThumbs(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusicArtists::tvShowSeasonBackdrops(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusicArtists::movieImages(Movie *movie, QString tmdbId, QList<int> types)
{
    Q_UNUSED(movie);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::moviePosters(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::movieBackdrops(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::movieLogos(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::movieBanners(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::movieThumbs(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::movieClearArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::movieCdArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::concertImages(Concert *concert, QString tmdbId, QList<int> types)
{
    Q_UNUSED(tmdbId);
    Q_UNUSED(concert);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::concertPosters(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::searchMovie(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusicArtists::concertClearArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::concertCdArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

bool FanartTvMusicArtists::hasSettings()
{
    return false;
}

void FanartTvMusicArtists::loadSettings(QSettings &settings)
{
    m_language = settings.value("Scrapers/FanartTv/Language", "en").toString();
    m_preferredDiscType = settings.value("Scrapers/FanartTv/DiscType", "BluRay").toString();
    m_personalApiKey = settings.value("Scrapers/FanartTv/PersonalApiKey").toString();
}

void FanartTvMusicArtists::saveSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

QWidget* FanartTvMusicArtists::settingsWidget()
{
    return 0;
}

QString FanartTvMusicArtists::keyParameter()
{
    return (!m_personalApiKey.isEmpty()) ? QString("api_key=%1&client_key=%2").arg(m_apiKey).arg(m_personalApiKey) : QString("api_key=%1").arg(m_apiKey);
}

void FanartTvMusicArtists::searchAlbum(QString artistName, QString searchStr, int limit)
{
    Q_UNUSED(artistName);
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusicArtists::searchArtist(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusicArtists::artistFanarts(QString mbId)
{
    Q_UNUSED(mbId);
}

void FanartTvMusicArtists::artistLogos(QString mbId)
{
    Q_UNUSED(mbId);
}

void FanartTvMusicArtists::artistThumbs(QString mbId)
{
    Q_UNUSED(mbId);
}

void FanartTvMusicArtists::albumCdArts(QString mbId)
{
    Q_UNUSED(mbId);
}

void FanartTvMusicArtists::albumThumbs(QString mbId)
{
    Q_UNUSED(mbId);
}

void FanartTvMusicArtists::artistImages(Artist *artist, QString mbId, QList<int> types)
{
    Q_UNUSED(artist);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::albumImages(Album *album, QString mbId, QList<int> types)
{
    Q_UNUSED(album);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::albumBooklets(QString mbId)
{
    Q_UNUSED(mbId);
}
