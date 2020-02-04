#include "FanartTvMusicArtists.h"

#include <QDebug>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "data/Storage.h"
#include "scrapers/image/FanartTv.h"
#include "scrapers/movie/TMDb.h"

FanartTvMusicArtists::FanartTvMusicArtists(QObject* parent)
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
QString FanartTvMusicArtists::name() const
{
    return QString("Fanart.tv Music Artists");
}

QUrl FanartTvMusicArtists::siteUrl() const
{
    return QUrl("https://fanart.tv");
}

QString FanartTvMusicArtists::identifier() const
{
    return QString("images.fanarttv-music");
}

/**
 * @brief Returns a list of supported image types
 * @return List of supported image types
 */
QVector<ImageType> FanartTvMusicArtists::provides()
{
    return m_provides;
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager* FanartTvMusicArtists::qnam()
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
    QNetworkReply* reply = qnam()->get(request);
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusicArtists::onSearchArtistFinished);
}

void FanartTvMusicArtists::onSearchArtistFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit sigSearchDone({}, {ScraperSearchError::ErrorType::NetworkError, reply->errorString()});
        return;
    }

    QVector<ScraperSearchResult> results;
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
    emit sigSearchDone(results, {});
}

void FanartTvMusicArtists::concertBackdrops(TmdbId tmdbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("https://webservice.fanart.tv/v3/music/%1?%2").arg(tmdbId.toString(), keyParameter()));
    request.setUrl(url);
    QNetworkReply* reply = qnam()->get(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::ConcertBackdrop));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusicArtists::onLoadConcertFinished);
}

void FanartTvMusicArtists::concertLogos(TmdbId tmdbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("https://webservice.fanart.tv/v3/music/%1?%2").arg(tmdbId.toString(), keyParameter()));
    request.setUrl(url);
    QNetworkReply* reply = qnam()->get(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::ConcertLogo));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusicArtists::onLoadConcertFinished);
}

void FanartTvMusicArtists::onLoadConcertFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    ImageType info = ImageType(reply->property("infoToLoad").toInt());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit sigImagesLoaded({}, {ScraperLoadError::ErrorType::NetworkError, reply->errorString()});
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    QVector<Poster> posters = parseData(msg, info);
    emit sigImagesLoaded(posters, {});
}

QVector<Poster> FanartTvMusicArtists::parseData(QString json, ImageType type)
{
    QMap<ImageType, QStringList> map;
    map.insert(ImageType::ConcertBackdrop, QStringList() << "artistbackground");
    map.insert(ImageType::ConcertLogo, {"hdmusiclogo", "musiclogo"});

    QVector<Poster> posters;

    QJsonParseError parseError{};
    // The JSON contains one object with all URLs to fanart images
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing fanart music json: " << parseError.errorString();
        return posters;
    }

    for (const auto& section : map.value(type)) {
        const auto jsonPosters = parsedJson.value(section).toArray();

        for (const auto& it : jsonPosters) {
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
                }
                if (section == "musiclogo") {
                    return QStringLiteral("SD");
                }
                if (discType == "bluray") {
                    return QStringLiteral("BluRay");
                }
                if (discType == "dvd") {
                    return QStringLiteral("DVD");
                }
                if (discType == "3d") {
                    return QStringLiteral("3D");
                }
                return QStringLiteral("");
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

void FanartTvMusicArtists::tvShowImages(TvShow* show, TvDbId tvdbId, QVector<ImageType> types)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(show);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::tvShowPosters(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowBackdrops(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowLogos(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowThumbs(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowClearArts(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowCharacterArts(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowBanners(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowEpisodeThumb(TvDbId tvdbId, SeasonNumber season, EpisodeNumber episode)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
}

void FanartTvMusicArtists::tvShowSeason(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusicArtists::tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusicArtists::tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusicArtists::tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusicArtists::movieImages(Movie* movie, TmdbId tmdbId, QVector<ImageType> types)
{
    Q_UNUSED(movie);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::moviePosters(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::movieBackdrops(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::movieLogos(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::movieBanners(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::movieThumbs(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::movieClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::movieCdArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::concertImages(Concert* concert, TmdbId tmdbId, QVector<ImageType> types)
{
    Q_UNUSED(tmdbId);
    Q_UNUSED(concert);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::concertPosters(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::searchMovie(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusicArtists::concertClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusicArtists::concertCdArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

bool FanartTvMusicArtists::hasSettings() const
{
    return false;
}

void FanartTvMusicArtists::loadSettings(const ScraperSettings& settings)
{
    m_language = settings.language();
    m_preferredDiscType = settings.valueString("DiscType", "BluRay");
    m_personalApiKey = settings.valueString("PersonalApiKey");
}

void FanartTvMusicArtists::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QWidget* FanartTvMusicArtists::settingsWidget()
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

void FanartTvMusicArtists::artistImages(Artist* artist, QString mbId, QVector<ImageType> types)
{
    Q_UNUSED(artist);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::albumImages(Album* album, QString mbId, QVector<ImageType> types)
{
    Q_UNUSED(album);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::albumBooklets(QString mbId)
{
    Q_UNUSED(mbId);
}
