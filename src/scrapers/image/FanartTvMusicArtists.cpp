#include "FanartTvMusicArtists.h"

#include <QDebug>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "data/Storage.h"
#include "network/NetworkRequest.h"
#include "scrapers/image/FanartTv.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"

namespace mediaelch {
namespace scraper {

QString FanartTvMusicArtists::ID = "images.fanarttv-music";

FanartTvMusicArtists::FanartTvMusicArtists(QObject* parent) : ImageProvider(parent)
{
    m_meta.identifier = ID;
    m_meta.name = "Fanart.tv Music Artists";
    m_meta.description = tr("FanartTV is a community-driven image provider.");
    m_meta.website = "https://fanart.tv";
    m_meta.termsOfService = "https://fanart.tv/terms-and-conditions/";
    m_meta.privacyPolicy = "https://fanart.tv/privacy-policy/";
    m_meta.help = "https://forum.fanart.tv/";
    m_meta.supportedImageTypes = {ImageType::ConcertBackdrop, ImageType::ConcertLogo};
    // Multiple languages, but no way to query for it and also no offical list of languages.
    m_meta.supportedLanguages = {
        "bg",
        "zh",
        "hr",
        "cs",
        "da",
        "nl",
        "en",
        "fi",
        "fr",
        "de",
        "el",
        "he",
        "hu",
        "it",
        "ja",
        "ko",
        "no",
        "pl",
        "pt",
        "ru",
        "sl",
        "es",
        "sv",
        "tr",
    };
    m_meta.defaultLocale = "en";

    m_apiKey = "842f7a5d1cc7396f142b8dd47c4ba42b";
    m_searchResultLimit = 0;
    m_preferredDiscType = "BluRay";
}

const ImageProvider::ScraperMeta& FanartTvMusicArtists::meta() const
{
    return m_meta;
}

mediaelch::network::NetworkManager* FanartTvMusicArtists::network()
{
    return &m_network;
}

/**
 * \brief Searches for an artist
 * \param searchStr The Artist name/search string
 * \param limit Number of results, if zero, all results are returned
 * \see FanartTvMusicArtists::searchMovie
 */
void FanartTvMusicArtists::searchConcert(QString searchStr, int limit)
{
    Q_UNUSED(limit);
    QUrl url(QStringLiteral("https://www.musicbrainz.org/ws/2/artist/?query=artist:%1")
                 .arg(QString(QUrl::toPercentEncoding(searchStr))));
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    QNetworkReply* reply = network()->getWithWatcher(request);
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusicArtists::onSearchArtistFinished);
}

void FanartTvMusicArtists::onSearchArtistFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit sigSearchDone({}, mediaelch::replyToScraperError(*reply));
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
    QUrl url = QStringLiteral("https://webservice.fanart.tv/v3/music/%1?%2").arg(tmdbId.toString(), keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::ConcertBackdrop));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusicArtists::onLoadConcertFinished);
}

void FanartTvMusicArtists::concertLogos(TmdbId tmdbId)
{
    QUrl url = QStringLiteral("https://webservice.fanart.tv/v3/music/%1?%2").arg(tmdbId.toString(), keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::ConcertLogo));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusicArtists::onLoadConcertFinished);
}

void FanartTvMusicArtists::onLoadConcertFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    ImageType info = ImageType(reply->property("infoToLoad").toInt());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit sigImagesLoaded({}, mediaelch::replyToScraperError(*reply));
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
            FanartTv::insertPoster(posters, b, m_meta.defaultLocale.toString(), m_preferredDiscType);
        }
    }

    return posters;
}

void FanartTvMusicArtists::searchTvShow(QString searchStr, mediaelch::Locale locale, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
    Q_UNUSED(locale);
}

void FanartTvMusicArtists::tvShowImages(TvShow* show,
    TvDbId tvdbId,
    QVector<ImageType> types,
    const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(show);
    Q_UNUSED(types);
    Q_UNUSED(locale)
}

void FanartTvMusicArtists::tvShowPosters(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId)
    Q_UNUSED(locale)
}

void FanartTvMusicArtists::tvShowBackdrops(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void FanartTvMusicArtists::tvShowLogos(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(locale)
    Q_UNUSED(tvdbId);
}

void FanartTvMusicArtists::tvShowThumbs(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void FanartTvMusicArtists::tvShowClearArts(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void FanartTvMusicArtists::tvShowCharacterArts(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void FanartTvMusicArtists::tvShowBanners(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void FanartTvMusicArtists::tvShowEpisodeThumb(TvDbId tvdbId,
    SeasonNumber season,
    EpisodeNumber episode,
    const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId)
    Q_UNUSED(season)
    Q_UNUSED(episode)
    Q_UNUSED(locale)
}

void FanartTvMusicArtists::tvShowSeason(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void FanartTvMusicArtists::tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
    Q_UNUSED(season);
}

void FanartTvMusicArtists::tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void FanartTvMusicArtists::tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
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

void FanartTvMusicArtists::loadSettings(ScraperSettings& settings)
{
    m_meta.defaultLocale = settings.language(m_meta.defaultLocale);
    m_preferredDiscType = settings.valueString("DiscType", "BluRay");
    m_personalApiKey = settings.valueString("PersonalApiKey", "");
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

void FanartTvMusicArtists::artistFanarts(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void FanartTvMusicArtists::artistLogos(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void FanartTvMusicArtists::artistThumbs(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void FanartTvMusicArtists::albumCdArts(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void FanartTvMusicArtists::albumThumbs(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void FanartTvMusicArtists::artistImages(Artist* artist, MusicBrainzId mbId, QVector<ImageType> types)
{
    Q_UNUSED(artist);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::albumImages(Album* album, MusicBrainzId mbId, QVector<ImageType> types)
{
    Q_UNUSED(album);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void FanartTvMusicArtists::albumBooklets(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

} // namespace scraper
} // namespace mediaelch
