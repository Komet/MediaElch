#include "FanartTvMusic.h"

#include <QApplication>
#include <QDebug>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "data/Storage.h"
#include "scrapers/image/FanartTv.h"
#include "scrapers/movie/TMDb.h"

FanartTvMusic::FanartTvMusic(QObject* parent)
{
    setParent(parent);
    m_provides = {ImageType::AlbumCdArt,
        ImageType::AlbumThumb,
        ImageType::ArtistFanart,
        ImageType::ArtistLogo,
        ImageType::ArtistThumb,
        ImageType::ArtistExtraFanart};
    m_apiKey = "842f7a5d1cc7396f142b8dd47c4ba42b";
    m_searchResultLimit = 0;
    m_language = "en";
}

QString FanartTvMusic::name() const
{
    return QString("Fanart.tv Music");
}

QUrl FanartTvMusic::siteUrl() const
{
    return QUrl("https://fanart.tv");
}

QString FanartTvMusic::identifier() const
{
    return QString("images.fanarttv-music_lib");
}

QVector<ImageType> FanartTvMusic::provides()
{
    return m_provides;
}

QNetworkAccessManager* FanartTvMusic::qnam()
{
    return &m_qnam;
}

void FanartTvMusic::searchAlbum(QString artistName, QString searchStr, int limit)
{
    Q_UNUSED(limit);

    QString searchQuery = "release:" + QString(QUrl::toPercentEncoding(searchStr));
    if (!artistName.isEmpty()) {
        searchQuery += "%20AND%20artist:" + QString(QUrl::toPercentEncoding(artistName));
    }
    QUrl url(QString("https://www.musicbrainz.org/ws/2/release/?query=%1").arg(searchQuery));
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent",
        QString("MediaElch/%1 (%2)").arg(QApplication::applicationVersion()).arg("support@mediaelch.de").toUtf8());
    QNetworkReply* reply = qnam()->get(request);
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onSearchAlbumFinished);
}

void FanartTvMusic::searchArtist(QString searchStr, int limit)
{
    Q_UNUSED(limit);
    QUrl url(QString("https://www.musicbrainz.org/ws/2/artist/?query=artist:%1")
                 .arg(QString(QUrl::toPercentEncoding(searchStr))));
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent",
        QString("MediaElch/%1 (%2)").arg(QApplication::applicationVersion()).arg("support@mediaelch.de").toUtf8());
    QNetworkReply* reply = qnam()->get(request);
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onSearchArtistFinished);
}

void FanartTvMusic::artistFanarts(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("https://webservice.fanart.tv/v3/music/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply* reply = qnam()->get(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::ArtistFanart));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadArtistFinished);
}

void FanartTvMusic::artistLogos(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("https://webservice.fanart.tv/v3/music/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply* reply = qnam()->get(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::ArtistLogo));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadArtistFinished);
}

void FanartTvMusic::artistThumbs(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("https://webservice.fanart.tv/v3/music/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply* reply = qnam()->get(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::ArtistThumb));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadArtistFinished);
}

void FanartTvMusic::albumCdArts(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("https://webservice.fanart.tv/v3/music/albums/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply* reply = qnam()->get(request);
    reply->setProperty("infoToLoad", static_cast<int>(ImageType::AlbumCdArt));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadAlbumFinished);
}

void FanartTvMusic::albumThumbs(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("https://webservice.fanart.tv/v3/music/albums/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply* reply = qnam()->get(request);
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
        qDebug() << "Got redirect" << reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        QNetworkRequest request(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl());
        request.setRawHeader("User-Agent",
            QString("MediaElch/%1 (%2)").arg(QApplication::applicationVersion()).arg("support@mediaelch.de").toUtf8());
        reply = qnam()->get(request);
        connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onSearchArtistFinished);
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        emit sigSearchDone({}, {ScraperSearchError::ErrorType::NetworkError, reply->errorString()});
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

    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302
        || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301) {
        qDebug() << "Got redirect" << reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        QNetworkRequest request(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl());
        request.setRawHeader("User-Agent",
            QString("MediaElch/%1 (%2)").arg(QApplication::applicationVersion()).arg("support@mediaelch.de").toUtf8());
        reply = qnam()->get(request);
        connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onSearchAlbumFinished);
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        emit sigSearchDone({}, {ScraperSearchError::ErrorType::NetworkError, reply->errorString()});
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
        emit sigImagesLoaded({}, {ScraperLoadError::ErrorType::NetworkError, reply->errorString()});
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
        emit sigImagesLoaded({}, {ScraperLoadError::ErrorType::NetworkError, reply->errorString()});
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    QVector<Poster> posters = parseData(msg, info);
    emit sigImagesLoaded(posters, {});
}

QVector<Poster> FanartTvMusic::parseData(QString json, ImageType type)
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
        qWarning() << "Error parsing fanart music json: " << parseError.errorString();
        return posters;
    }

    // The JSON contains one object with all URLs to fanart images
    const auto jsonObject = [&parsedJson, &type] {
        const auto jsonObj = parsedJson.object();

        // The JSON for CD Art and Thumbs has a different structure. This is a "hack"
        // to get a uniform one. We grab the last album (sorted lexicographical by key).
        if (type == ImageType::AlbumCdArt || type == ImageType::AlbumThumb) {
            const auto albums = jsonObj.value("albums").toObject();
            const auto key = albums.keys().back();
            return albums.value(key).toObject();
        }

        return jsonObj;
    }();

    for (const QString& section : map.value(type)) {
        const auto jsonValue = jsonObject.value(section);
        if (!jsonValue.isArray()) {
            continue;
        }

        for (const auto& it : jsonValue.toArray()) {
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
            FanartTv::insertPoster(posters, b, m_language, "");
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

void FanartTvMusic::searchTvShow(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusic::tvShowImages(TvShow* show, TvDbId tvdbId, QVector<ImageType> types)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(show);
    Q_UNUSED(types);
}

void FanartTvMusic::tvShowPosters(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowBackdrops(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowLogos(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowThumbs(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowClearArts(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowCharacterArts(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowBanners(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowEpisodeThumb(TvDbId tvdbId, SeasonNumber season, EpisodeNumber episode)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
}

void FanartTvMusic::tvShowSeason(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusic::tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusic::tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusic::tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusic::movieImages(Movie* movie, TmdbId tmdbId, QVector<ImageType> types)
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

void FanartTvMusic::concertImages(Concert* concert, TmdbId tmdbId, QVector<ImageType> types)
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

bool FanartTvMusic::hasSettings() const
{
    return false;
}

void FanartTvMusic::loadSettings(const ScraperSettings& settings)
{
    m_language = settings.language();
    m_personalApiKey = settings.valueString("PersonalApiKey");
}

void FanartTvMusic::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QWidget* FanartTvMusic::settingsWidget()
{
    return nullptr;
}

QString FanartTvMusic::keyParameter()
{
    return (!m_personalApiKey.isEmpty()) ? QString("api_key=%1&client_key=%2").arg(m_apiKey).arg(m_personalApiKey)
                                         : QString("api_key=%1").arg(m_apiKey);
}

void FanartTvMusic::searchConcert(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusic::artistImages(Artist* artist, QString mbId, QVector<ImageType> types)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("https://webservice.fanart.tv/v3/music/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply* reply = qnam()->get(request);
    reply->setProperty("storage", Storage::toVariant(reply, artist));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, types));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadAllArtistDataFinished);
}

void FanartTvMusic::albumImages(Album* album, QString mbId, QVector<ImageType> types)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("https://webservice.fanart.tv/v3/music/albums/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply* reply = qnam()->get(request);
    reply->setProperty("storage", Storage::toVariant(reply, album));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, types));
    connect(reply, &QNetworkReply::finished, this, &FanartTvMusic::onLoadAllAlbumDataFinished);
}

void FanartTvMusic::onLoadAllAlbumDataFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Album* album = reply->property("storage").value<Storage*>()->album();
    reply->deleteLater();
    QMap<ImageType, QVector<Poster>> posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        for (const auto type : reply->property("infosToLoad").value<Storage*>()->imageInfosToLoad()) {
            posters.insert(type, parseData(msg, type));
        }
    }
    emit sigAlbumImagesLoaded(album, posters);
}

void FanartTvMusic::onLoadAllArtistDataFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Artist* artist = reply->property("storage").value<Storage*>()->artist();
    reply->deleteLater();
    QMap<ImageType, QVector<Poster>> posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        for (const auto type : reply->property("infosToLoad").value<Storage*>()->imageInfosToLoad()) {
            posters.insert(type, parseData(msg, type));
        }
    }
    emit sigArtistImagesLoaded(artist, posters);
}

void FanartTvMusic::albumBooklets(QString mbId)
{
    Q_UNUSED(mbId);
}
