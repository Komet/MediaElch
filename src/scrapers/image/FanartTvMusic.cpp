#include "scrapers/image/FanartTvMusic.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/image/FanartTv.h"
#include "scrapers/image/FanartTvConfiguration.h"

#include <QApplication>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace mediaelch {
namespace scraper {

QString FanartTvMusic::ID = "images.fanarttv-music_lib";

FanartTvMusic::FanartTvMusic(FanartTvConfiguration& settings, QObject* parent) :
    ImageProvider(parent), m_settings{settings}
{
    m_meta.identifier = ID;
    m_meta.name = "Fanart.tv Music";
    m_meta.description = tr("FanartTV is a community-driven image provider.");
    m_meta.website = "https://fanart.tv";
    m_meta.termsOfService = "https://fanart.tv/terms-and-conditions/";
    m_meta.privacyPolicy = "https://fanart.tv/privacy-policy/";
    m_meta.help = "https://forum.fanart.tv/";
    m_meta.supportedImageTypes = {ImageType::AlbumCdArt,
        ImageType::AlbumThumb,
        ImageType::ArtistFanart,
        ImageType::ArtistLogo,
        ImageType::ArtistThumb,
        ImageType::ArtistExtraFanart};

    m_meta.supportedLanguages = FanartTvConfiguration::supportedLanguages();
    m_meta.defaultLocale = FanartTvConfiguration::defaultLocale();

    m_apiKey = "842f7a5d1cc7396f142b8dd47c4ba42b";
}

const ImageProvider::ScraperMeta& FanartTvMusic::meta() const
{
    return m_meta;
}

mediaelch::network::NetworkManager* FanartTvMusic::network()
{
    return &m_network;
}

void FanartTvMusic::searchAlbum(QString artistName, QString searchStr, int limit)
{
    Q_UNUSED(limit);

    QString searchQuery = "release:" + QString(QUrl::toPercentEncoding(searchStr));
    if (!artistName.isEmpty()) {
        searchQuery += "%20AND%20artist:" + QString(QUrl::toPercentEncoding(artistName));
    }
    QUrl url(QStringLiteral("https://www.musicbrainz.org/ws/2/release/?query=%1").arg(searchQuery));
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    QNetworkReply* reply = network()->get(request);
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onSearchAlbumFinished);
}

void FanartTvMusic::searchArtist(QString searchStr, int limit)
{
    Q_UNUSED(limit);
    QUrl url(QStringLiteral("https://www.musicbrainz.org/ws/2/artist/?query=artist:%1")
            .arg(QString(QUrl::toPercentEncoding(searchStr))));
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);

    QNetworkReply* reply = network()->getWithWatcher(request);
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onSearchArtistFinished);
}

void FanartTvMusic::artistFanarts(MusicBrainzId mbId)
{
    QUrl url = QStringLiteral("https://webservice.fanart.tv/v3/music/%1?%2").arg(mbId.toString()).arg(keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::ArtistFanart));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadArtistFinished);
}

void FanartTvMusic::artistLogos(MusicBrainzId mbId)
{
    QUrl url = QStringLiteral("https://webservice.fanart.tv/v3/music/%1?%2").arg(mbId.toString()).arg(keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::ArtistLogo));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadArtistFinished);
}

void FanartTvMusic::artistThumbs(MusicBrainzId mbId)
{
    QUrl url = QStringLiteral("https://webservice.fanart.tv/v3/music/%1?%2").arg(mbId.toString()).arg(keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::ArtistThumb));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadArtistFinished);
}

void FanartTvMusic::albumCdArts(MusicBrainzId mbId)
{
    QUrl url =
        QStringLiteral("https://webservice.fanart.tv/v3/music/albums/%1?%2").arg(mbId.toString()).arg(keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::AlbumCdArt));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadAlbumFinished);
}

void FanartTvMusic::albumThumbs(MusicBrainzId mbId)
{
    QUrl url =
        QStringLiteral("https://webservice.fanart.tv/v3/music/albums/%1?%2").arg(mbId.toString()).arg(keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::AlbumThumb));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadAlbumFinished);
}

void FanartTvMusic::onSearchArtistFinished()
{
    QVector<ScraperSearchResult> results;
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302
        || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301) {
        QUrl url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        qCDebug(generic) << "[FanartTvMusic] Got redirect" << url;
        QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
        reply = network()->getWithWatcher(request);
        connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onSearchArtistFinished);
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        emit sigSearchDone({}, mediaelch::replyToScraperError(*reply));
        return;
    }

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

void FanartTvMusic::onSearchAlbumFinished()
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
    QStringList searchIds;
    for (int i = 0, n = domDoc.elementsByTagName("release").count(); i < n; ++i) {
        QDomElement elem = domDoc.elementsByTagName("release").at(i).toElement();
        QString name;
        if (!elem.elementsByTagName("title").isEmpty()) {
            name = elem.elementsByTagName("title").at(0).toElement().text();
        } else {
            continue;
        }

        if (!elem.elementsByTagName("date").isEmpty()) {
            name += QString(" (%1)").arg(elem.elementsByTagName("date").at(0).toElement().text());
        }

        for (int x = 0, y = elem.elementsByTagName("release-group").count(); x < y; ++x) {
            QDomElement releaseGroupElem = elem.elementsByTagName("release-group").at(x).toElement();
            if (!releaseGroupElem.attribute("id").isEmpty()) {
                ScraperSearchResult result;
                result.id = releaseGroupElem.attribute("id");
                result.name = name;
                if (!searchIds.contains(result.id)) {
                    results.append(result);
                    searchIds.append(result.id);
                }
            }
        }
    }

    emit sigSearchDone(results, {});
}

void FanartTvMusic::onLoadArtistFinished()
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

void FanartTvMusic::onLoadAlbumFinished()
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

QVector<Poster> FanartTvMusic::parseData(QString json, ImageType type) const
{
    QMap<ImageType, QStringList> map;

    // clang-format off
    map.insert(ImageType::ArtistFanart,      QStringList() << "artistbackground");
    map.insert(ImageType::ArtistExtraFanart, QStringList() << "artistbackground");
    map.insert(ImageType::ArtistLogo,        QStringList() << "hdmusiclogo" << "musiclogo");
    map.insert(ImageType::ArtistThumb,       QStringList() << "artistthumb");
    map.insert(ImageType::AlbumCdArt,        QStringList() << "cdart");
    map.insert(ImageType::AlbumThumb,        QStringList() << "albumcover");
    // clang-format on

    QVector<Poster> posters;

    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(generic) << "Error parsing fanart music json: " << parseError.errorString();
        return posters;
    }

    // The JSON contains one object with all URLs to fanart images
    const auto jsonObject = [&parsedJson, &type] {
        auto jsonObj = parsedJson.object();

        // The JSON for CD Art and Thumbs has a different structure. This is a "hack"
        // to get a uniform one. We grab the last album, if there is one (sorted lexicographical by key).
        if (type == ImageType::AlbumCdArt || type == ImageType::AlbumThumb) {
            const auto albums = jsonObj.value("albums").toObject();
            if (!albums.isEmpty()) {
                const auto key = albums.keys().back();
                return albums.value(key).toObject();
            }
        }

        return jsonObj;
    }();

    for (const QString& section : map.value(type)) {
        const auto jsonValue = jsonObject.value(section);
        if (!jsonValue.isArray()) {
            continue;
        }

        for (QJsonValueRef it : jsonValue.toArray()) {
            const auto val = it.toObject();
            if (val.value("url").toString().isEmpty()) {
                continue;
            }
            Poster b;
            b.thumbUrl = val.value("url").toString().replace("/fanart/", "/preview/");
            b.originalUrl = val.value("url").toString();
            b.hint = [&section] {
                if (section == "hdmusiclogo") {
                    return QStringLiteral("HD");
                }
                if (section == "musiclogo") {
                    return QStringLiteral("SD");
                }
                return QStringLiteral("");
            }();
            b.language = val.value("lang").toString();
            FanartTv::insertPoster(posters, b, m_meta.languagePriority, "");
        }
    }

    return posters;
}

void FanartTvMusic::concertBackdrops(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::concertLogos(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::searchTvShow(QString searchStr, mediaelch::Locale locale, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
    Q_UNUSED(locale);
}

void FanartTvMusic::tvShowImages(TvShow* show, TvDbId tvdbId, QSet<ImageType> types, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(show);
    Q_UNUSED(types);
    Q_UNUSED(locale)
}

void FanartTvMusic::tvShowPosters(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void FanartTvMusic::tvShowBackdrops(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void FanartTvMusic::tvShowLogos(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void FanartTvMusic::tvShowThumbs(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void FanartTvMusic::tvShowClearArts(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void FanartTvMusic::tvShowCharacterArts(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void FanartTvMusic::tvShowBanners(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void FanartTvMusic::tvShowEpisodeThumb(TvDbId tvdbId,
    SeasonNumber season,
    EpisodeNumber episode,
    const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
    Q_UNUSED(locale)
}

void FanartTvMusic::tvShowSeason(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void FanartTvMusic::tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void FanartTvMusic::tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void FanartTvMusic::tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void FanartTvMusic::movieImages(Movie* movie, TmdbId tmdbId, QSet<ImageType> types)
{
    Q_UNUSED(movie);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

void FanartTvMusic::moviePosters(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::movieBackdrops(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::movieLogos(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::movieBanners(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::movieThumbs(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::movieClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::movieCdArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::concertImages(Concert* concert, TmdbId tmdbId, QSet<ImageType> types)
{
    Q_UNUSED(tmdbId);
    Q_UNUSED(concert);
    Q_UNUSED(types);
}

void FanartTvMusic::concertPosters(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::searchMovie(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusic::concertClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::concertCdArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

QString FanartTvMusic::keyParameter()
{
    QString personalKey = m_settings.personalApiKey();
    return (!personalKey.isEmpty()) ? QStringLiteral("api_key=%1&client_key=%2").arg(m_apiKey).arg(personalKey)
                                    : QStringLiteral("api_key=%1").arg(m_apiKey);
}

void FanartTvMusic::searchConcert(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusic::artistImages(Artist* artist, MusicBrainzId mbId, QSet<ImageType> types)
{
    QUrl url = QStringLiteral("https://webservice.fanart.tv/v3/music/%1?%2").arg(mbId.toString()).arg(keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("storage", QVariant::fromValue(artist));
    reply->setProperty("infosToLoad", QVariant::fromValue(types));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadAllArtistDataFinished);
}

void FanartTvMusic::albumImages(Album* album, MusicBrainzId mbId, QSet<ImageType> types)
{
    QUrl url =
        QStringLiteral("https://webservice.fanart.tv/v3/music/albums/%1?%2").arg(mbId.toString()).arg(keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("storage", QVariant::fromValue(album));
    reply->setProperty("infosToLoad", QVariant::fromValue(types));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadAllAlbumDataFinished);
}

void FanartTvMusic::onLoadAllAlbumDataFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Album* album = reply->property("storage").value<Album*>();
    reply->deleteLater();
    QMap<ImageType, QVector<Poster>> posters;
    if (reply->error() == QNetworkReply::NoError) {
        const QString msg = QString::fromUtf8(reply->readAll());
        const auto infosToLoad = reply->property("infosToLoad").value<QSet<ImageType>>();
        for (const ImageType type : infosToLoad) {
            posters.insert(type, parseData(msg, type));
        }
    }
    emit sigAlbumImagesLoaded(album, posters);
}

void FanartTvMusic::onLoadAllArtistDataFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Artist* artist = reply->property("storage").value<Artist*>();
    reply->deleteLater();
    QMap<ImageType, QVector<Poster>> posters;
    if (reply->error() == QNetworkReply::NoError) {
        const QString msg = QString::fromUtf8(reply->readAll());
        const auto infosToLoad = reply->property("infosToLoad").value<QSet<ImageType>>();
        for (const ImageType type : infosToLoad) {
            posters.insert(type, parseData(msg, type));
        }
    }
    emit sigArtistImagesLoaded(artist, posters);
}

void FanartTvMusic::albumBooklets(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

} // namespace scraper
} // namespace mediaelch
