#include "FanartTvMusic.h"
#include <QDebug>
#include <QSettings>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include "data/Storage.h"
#include "imageProviders/FanartTv.h"
#include "scrapers/TMDb.h"

FanartTvMusic::FanartTvMusic(QObject *parent)
{
    setParent(parent);
    m_provides << ImageType::AlbumCdArt << ImageType::AlbumThumb <<
                  ImageType::ArtistFanart << ImageType::ArtistLogo << ImageType::ArtistThumb <<
                  ImageType::ArtistExtraFanart;
    m_apiKey = "842f7a5d1cc7396f142b8dd47c4ba42b";
    m_searchResultLimit = 0;
    m_language = "en";
}

QString FanartTvMusic::name()
{
    return QString("Fanart.tv Music");
}

QUrl FanartTvMusic::siteUrl()
{
    return QUrl("https://fanart.tv");
}

QString FanartTvMusic::identifier()
{
    return QString("images.fanarttv-music_lib");
}

QList<int> FanartTvMusic::provides()
{
    return m_provides;
}

QNetworkAccessManager *FanartTvMusic::qnam()
{
    return &m_qnam;
}

void FanartTvMusic::searchAlbum(QString artistName, QString searchStr, int limit)
{
    Q_UNUSED(limit);

    QString searchQuery = "release:" + QString(QUrl::toPercentEncoding(searchStr));
    if (!artistName.isEmpty())
        searchQuery += "%20AND%20artist:" + QString(QUrl::toPercentEncoding(artistName));
    QUrl url(QString("http://www.musicbrainz.org/ws/2/release/?query=%1").arg(searchQuery));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchAlbumFinished()));
}

void FanartTvMusic::searchArtist(QString searchStr, int limit)
{
    Q_UNUSED(limit);
    QUrl url(QString("http://www.musicbrainz.org/ws/2/artist/?query=artist:%1").arg(QString(QUrl::toPercentEncoding(searchStr))));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchArtistFinished()));
}

void FanartTvMusic::artistFanarts(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/music/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infoToLoad", ImageType::ArtistFanart);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadArtistFinished()));
}

void FanartTvMusic::artistLogos(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/music/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infoToLoad", ImageType::ArtistLogo);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadArtistFinished()));
}

void FanartTvMusic::artistThumbs(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/music/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infoToLoad", ImageType::ArtistThumb);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadArtistFinished()));
}

void FanartTvMusic::albumCdArts(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/music/albums/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infoToLoad", ImageType::AlbumCdArt);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadAlbumFinished()));
}

void FanartTvMusic::albumThumbs(QString mbId)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/music/albums/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infoToLoad", ImageType::AlbumThumb);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadAlbumFinished()));
}

void FanartTvMusic::onSearchArtistFinished()
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

void FanartTvMusic::onSearchAlbumFinished()
{
    QList<ScraperSearchResult> results;
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        QDomDocument domDoc;
        domDoc.setContent(msg);
        QStringList searchIds;
        for (int i=0, n=domDoc.elementsByTagName("release").count() ; i<n ; ++i) {
            QDomElement elem = domDoc.elementsByTagName("release").at(i).toElement();
            QString name;
            if (!elem.elementsByTagName("title").isEmpty())
                name = elem.elementsByTagName("title").at(0).toElement().text();
            else
                continue;

            if (!elem.elementsByTagName("date").isEmpty())
                name += QString(" (%1)").arg(elem.elementsByTagName("date").at(0).toElement().text());

            for (int x=0, y=elem.elementsByTagName("release-group").count() ; x<y ; ++x) {
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
    }
    emit sigSearchDone(results);
}

void FanartTvMusic::onLoadArtistFinished()
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

void FanartTvMusic::onLoadAlbumFinished()
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

QList<Poster> FanartTvMusic::parseData(QString json, int type)
{
    QMap<int, QStringList> map;
    map.insert(ImageType::ArtistFanart, QStringList() << "artistbackground");
    map.insert(ImageType::ArtistExtraFanart, QStringList() << "artistbackground");
    map.insert(ImageType::ArtistLogo, QStringList() << "hdmusiclogo" << "musiclogo");
    map.insert(ImageType::ArtistThumb, QStringList() << "artistthumb");
    map.insert(ImageType::AlbumCdArt, QStringList() << "cdart");
    map.insert(ImageType::AlbumThumb, QStringList() << "albumcover");
    QList<Poster> posters;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    if (type == ImageType::AlbumCdArt || type == ImageType::AlbumThumb) {
        QScriptValueIterator it(sc.property("albums"));
        while (it.hasNext()) {
            it.next();
            sc = it.value();
            break;
        }
    }

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
                b.language = vB.property("lang").toString();
                FanartTv::insertPoster(posters, b, m_language, "");
            }
        }
    }

    return posters;
}

void FanartTvMusic::concertBackdrops(QString mbId)
{
    Q_UNUSED(mbId);
}

void FanartTvMusic::concertLogos(QString mbId)
{
    Q_UNUSED(mbId);
}

void FanartTvMusic::searchTvShow(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusic::tvShowImages(TvShow *show, QString tvdbId, QList<int> types)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(show);
    Q_UNUSED(types);
}

void FanartTvMusic::tvShowPosters(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowBackdrops(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowLogos(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowThumbs(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowClearArts(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowCharacterArts(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowBanners(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void FanartTvMusic::tvShowEpisodeThumb(QString tvdbId, int season, int episode)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
}

void FanartTvMusic::tvShowSeason(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusic::tvShowSeasonBanners(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusic::tvShowSeasonThumbs(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusic::tvShowSeasonBackdrops(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTvMusic::movieImages(Movie *movie, QString tmdbId, QList<int> types)
{
    Q_UNUSED(movie);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

void FanartTvMusic::moviePosters(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::movieBackdrops(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::movieLogos(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::movieBanners(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::movieThumbs(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::movieClearArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::movieCdArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::concertImages(Concert *concert, QString tmdbId, QList<int> types)
{
    Q_UNUSED(tmdbId);
    Q_UNUSED(concert);
    Q_UNUSED(types);
}

void FanartTvMusic::concertPosters(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::searchMovie(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusic::concertClearArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void FanartTvMusic::concertCdArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

bool FanartTvMusic::hasSettings()
{
    return false;
}

void FanartTvMusic::loadSettings(QSettings &settings)
{
    m_language = settings.value("Scrapers/FanartTv/Language", "en").toString();
    m_personalApiKey = settings.value("Scrapers/FanartTv/PersonalApiKey").toString();
}

void FanartTvMusic::saveSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

QWidget* FanartTvMusic::settingsWidget()
{
    return 0;
}

QString FanartTvMusic::keyParameter()
{
    return (!m_personalApiKey.isEmpty()) ? QString("api_key=%1&client_key=%2").arg(m_apiKey).arg(m_personalApiKey) : QString("api_key=%1").arg(m_apiKey);
}

void FanartTvMusic::searchConcert(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTvMusic::artistImages(Artist *artist, QString mbId, QList<int> types)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/music/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("storage", Storage::toVariant(reply, artist));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, types));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadAllArtistDataFinished()));
}

void FanartTvMusic::albumImages(Album *album, QString mbId, QList<int> types)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/music/albums/%1?%2").arg(mbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("storage", Storage::toVariant(reply, album));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, types));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadAllAlbumDataFinished()));
}

void FanartTvMusic::onLoadAllAlbumDataFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Album *album = reply->property("storage").value<Storage*>()->album();
    reply->deleteLater();
    QMap<int, QList<Poster> > posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        foreach (int type, reply->property("infosToLoad").value<Storage*>()->infosToLoad())
            posters.insert(type, parseData(msg, type));
    }
    emit sigImagesLoaded(album, posters);
}

void FanartTvMusic::onLoadAllArtistDataFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Artist *artist = reply->property("storage").value<Storage*>()->artist();
    reply->deleteLater();
    QMap<int, QList<Poster> > posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        foreach (int type, reply->property("infosToLoad").value<Storage*>()->infosToLoad())
            posters.insert(type, parseData(msg, type));
    }
    emit sigImagesLoaded(artist, posters);
}

void FanartTvMusic::albumBooklets(QString mbId)
{
    Q_UNUSED(mbId);
}
