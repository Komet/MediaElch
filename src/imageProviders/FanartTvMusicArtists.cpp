#include "FanartTvMusicArtists.h"

#include <QDebug>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>

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
QList<ImageType> FanartTvMusicArtists::provides()
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
    QUrl url(QString("https://www.musicbrainz.org/ws/2/artist/?query=artist:%1")
                 .arg(QString(QUrl::toPercentEncoding(searchStr))));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusicArtists::onSearchArtistFinished);
}

void FanartTvMusicArtists::onSearchArtistFinished()
{
    QList<ScraperSearchResult> results;
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        QDomDocument domDoc;
        domDoc.setContent(msg);
        for (int i = 0, n = domDoc.elementsByTagName("artist").count(); i < n; ++i) {
            QDomElement elem = domDoc.elementsByTagName("artist").at(i).toElement();
            QString name;
            if (!elem.elementsByTagName("name").isEmpty()) {
                name = elem.elementsByTagName("name").at(0).toElement().text();
            }
            if (!elem.elementsByTagName("disambiguation").isEmpty()) {
                name.append(QString(" (%1)").arg(elem.elementsByTagName("disambiguation").at(0).toElement().text()));
            }

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
    url.setUrl(QString("https://webservice.fanart.tv/v3/music/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::ConcertBackdrop));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusicArtists::onLoadConcertFinished);
}

void FanartTvMusicArtists::concertLogos(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("https://webservice.fanart.tv/v3/music/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::ConcertLogo));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusicArtists::onLoadConcertFinished);
}

void FanartTvMusicArtists::onLoadConcertFinished()
{
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    ImageType info = ImageType(reply->property("infoToLoad").toInt());
    reply->deleteLater();
    QList<Poster> posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        posters = parseData(msg, info);
    }
    emit sigImagesLoaded(posters);
}

QList<Poster> FanartTvMusicArtists::parseData(QString json, ImageType type)
{
    QMap<ImageType, QStringList> map;
    map.insert(ImageType::ConcertBackdrop, QStringList() << "artistbackground");
    map.insert(ImageType::ConcertLogo, {"hdmusiclogo", "musiclogo"});

    QList<Poster> posters;

    QJsonParseError parseError;
    // The JSON contains one object with all URLs to fanart images
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing fanart music json: " << parseError.errorString();
        return posters;
    }

    for (const auto &section : map.value(type)) {
        const auto jsonPosters = parsedJson.value(section).toArray();

        for (const auto &it : jsonPosters) {
            const auto poster = it.toObject();
            if (poster.value("url").toString().isEmpty()) {
                continue;
            }

            Poster b;
            b.thumbUrl = poster.value("url").toString().replace("/fanart/", "/preview/");
            b.originalUrl = poster.value("url").toString();

            const auto discType = poster.value("disc_type").toString();
            b.hint = [&section, &discType] {
                if (section == "hdmusiclogo") {
                    return QStringLiteral("HD");
                } else if (section == "musiclogo") {
                    return QStringLiteral("SD");
                } else if (discType == "bluray") {
                    return QStringLiteral("BluRay");
                } else if (discType == "dvd") {
                    return QStringLiteral("DVD");
                } else if (discType == "3d") {
                    return QStringLiteral("3D");
                } else {
                    return QStringLiteral("");
                }
            }();

            b.language = poster.value("lang").toString();
            FanartTv::insertPoster(posters, b, m_language, m_preferredDiscType);
        }
    }

    return posters;
}

void FanartTvMusicArtists::searchTvShow(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusicArtists::tvShowImages(TvShow *show, QString tvdbId, QList<ImageType> types)
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

void FanartTvMusicArtists::tvShowEpisodeThumb(QString tvdbId, int season, EpisodeNumber episode)
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

void FanartTvMusicArtists::movieImages(Movie *movie, QString tmdbId, QList<ImageType> types)
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

void FanartTvMusicArtists::concertImages(Concert *concert, QString tmdbId, QList<ImageType> types)
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

QWidget *FanartTvMusicArtists::settingsWidget()
{
    return nullptr;
}

QString FanartTvMusicArtists::keyParameter()
{
    return (!m_personalApiKey.isEmpty()) ? QString("api_key=%1&client_key=%2").arg(m_apiKey).arg(m_personalApiKey)
                                         : QString("api_key=%1").arg(m_apiKey);
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

void FanartTvMusicArtists::artistImages(Artist *artist, QString mbId, QList<ImageType> types)
{
    Q_UNUSED(artist);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::albumImages(Album *album, QString mbId, QList<ImageType> types)
{
    Q_UNUSED(album);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::albumBooklets(QString mbId)
{
    Q_UNUSED(mbId);
}
