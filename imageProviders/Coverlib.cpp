#include "Coverlib.h"
#include <QDebug>
#include <QSettings>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include "data/Storage.h"
#include "imageProviders/FanartTv.h"
#include "scrapers/TMDb.h"

Coverlib::Coverlib(QObject *parent)
{
    setParent(parent);
    m_provides << ImageType::AlbumCdArt << ImageType::AlbumThumb << ImageType::AlbumBooklet;
}

QString Coverlib::name()
{
    return QString("Coverlib");
}

QUrl Coverlib::siteUrl()
{
    return QUrl("http://coverlib.com");
}

QString Coverlib::identifier()
{
    return QString("images.coverlib");
}

QList<int> Coverlib::provides()
{
    return m_provides;
}

bool Coverlib::hasSettings()
{
    return false;
}

void Coverlib::loadSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

void Coverlib::saveSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

QWidget* Coverlib::settingsWidget()
{
    return 0;
}

QNetworkAccessManager *Coverlib::qnam()
{
    return &m_qnam;
}

void Coverlib::searchAlbum(QString artistName, QString searchStr, int limit)
{
    Q_UNUSED(limit);
    qDebug() << artistName << searchStr;
    QString searchQuery = QString(QUrl::toPercentEncoding(artistName + " " + searchStr));
    QUrl url(QString("http://coverlib.com/search/?Sektion=2&pm=on&q=%1").arg(searchQuery));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchAlbumFinished()));
}

void Coverlib::onSearchAlbumFinished()
{
    QList<ScraperSearchResult> results;
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        QRegExp rx;
        rx.setMinimal(true);
        rx.setPattern("<h5>[^<]*<a href=\"/entry/(.*)\">(.*)</a>([^<]*)</h5>");
        int pos = 0;
        while ((pos = rx.indexIn(msg, pos)) != -1) {
            ScraperSearchResult result;
            result.name = rx.cap(2);
            result.id = rx.cap(1);
            results.append(result);
            pos += rx.matchedLength();
        }
    }
    emit sigSearchDone(results);
}

void Coverlib::albumCdArts(QString id)
{
    QUrl url;
    QNetworkRequest request;
    url.setUrl(QString("http://coverlib.com/entry/%1").arg(id));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infoToLoad", ImageType::AlbumCdArt);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadAlbumFinished()));
}

void Coverlib::albumThumbs(QString id)
{
    QUrl url;
    QNetworkRequest request;
    url.setUrl(QString("http://coverlib.com/entry/%1").arg(id));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infoToLoad", ImageType::AlbumThumb);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadAlbumFinished()));
}

void Coverlib::albumBooklets(QString id)
{
    QUrl url;
    QNetworkRequest request;
    url.setUrl(QString("http://coverlib.com/entry/%1").arg(id));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infoToLoad", ImageType::AlbumBooklet);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadAlbumFinished()));
}

void Coverlib::onLoadAlbumFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    int info = reply->property("infoToLoad").toInt();
    reply->deleteLater();
    QList<Poster> posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString html = QString::fromUtf8(reply->readAll());
        posters = parseData(html, info);
    }
    emit sigImagesLoaded(posters);
}

QList<Poster> Coverlib::parseData(QString html, int type)
{
    QList<Poster> posters;
    QRegExp rx;
    rx.setMinimal(true);
    rx.setPattern("<div class=\"thumbnail\">[^<]*<a href=\"([^\"]*)\" class=\"gallerytwo-item\"><img .* src=\"([^\"]*)\" .*></a>[^<]*<div class=\"caption\">[^<]*<h3 class=\"elementTyp\">(.*)</h3>");
    int pos = 0;
    while ((pos = rx.indexIn(html, pos)) != -1) {
        pos += rx.matchedLength();
        if (type == ImageType::AlbumCdArt && rx.cap(3).trimmed() != "CD")
            continue;
        if (type == ImageType::AlbumThumb && rx.cap(3).trimmed() != "Front")
            continue;
        if (type == ImageType::AlbumBooklet && !rx.cap(3).trimmed().startsWith("Booklet"))
            continue;

        Poster p;
        p.thumbUrl = !rx.cap(2).startsWith("http") ? "http://coverlib.com" + rx.cap(2) : rx.cap(2);
        p.originalUrl = !rx.cap(1).startsWith("http") ? "http://coverlib.com" + rx.cap(1) : rx.cap(1);
        p.hint = rx.cap(3);
        posters.append(p);
    }
    return posters;
}

void Coverlib::albumImages(Album *album, QString mbId, QList<int> types)
{
    Q_UNUSED(album);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void Coverlib::artistImages(Artist *artist, QString mbId, QList<int> types)
{
    Q_UNUSED(artist);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void Coverlib::artistFanarts(QString mbId)
{
    Q_UNUSED(mbId)
}

void Coverlib::artistLogos(QString mbId)
{
    Q_UNUSED(mbId)
}

void Coverlib::artistThumbs(QString mbId)
{
    Q_UNUSED(mbId)
}

void Coverlib::movieImages(Movie *movie, QString tmdbId, QList<int> types)
{
    Q_UNUSED(movie);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

void Coverlib::moviePosters(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void Coverlib::movieBackdrops(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void Coverlib::movieLogos(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void Coverlib::movieBanners(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void Coverlib::movieThumbs(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void Coverlib::movieClearArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void Coverlib::movieCdArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void Coverlib::concertImages(Concert *concert, QString tmdbId, QList<int> types)
{
    Q_UNUSED(tmdbId);
    Q_UNUSED(concert);
    Q_UNUSED(types);
}

void Coverlib::concertPosters(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void Coverlib::concertBackdrops(QString mbId)
{
    Q_UNUSED(mbId);
}

void Coverlib::concertLogos(QString mbId)
{
    Q_UNUSED(mbId);
}

void Coverlib::concertClearArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void Coverlib::concertCdArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void Coverlib::tvShowImages(TvShow *show, QString tvdbId, QList<int> types)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(show);
    Q_UNUSED(types);
}

void Coverlib::tvShowPosters(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void Coverlib::tvShowBackdrops(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void Coverlib::tvShowLogos(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void Coverlib::tvShowClearArts(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void Coverlib::tvShowCharacterArts(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void Coverlib::tvShowBanners(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void Coverlib::tvShowEpisodeThumb(QString tvdbId, int season, int episode)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
}

void Coverlib::tvShowSeason(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void Coverlib::tvShowSeasonBanners(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void Coverlib::tvShowSeasonThumbs(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void Coverlib::tvShowSeasonBackdrops(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void Coverlib::tvShowThumbs(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void Coverlib::searchTvShow(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void Coverlib::searchMovie(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void Coverlib::searchConcert(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void Coverlib::searchArtist(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}
