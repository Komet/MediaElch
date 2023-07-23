#include "scrapers/music/MusicBrainz.h"

#include "data/music/Album.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/ScraperUtils.h"
#include "scrapers/music/UniversalMusicScraper.h"
#include "utils/Meta.h"

#include <QDomDocument>
#include <QJsonDocument>
#include <QList>
#include <QNetworkCookie>

namespace mediaelch {
namespace scraper {

MusicBrainzApi::MusicBrainzApi(QObject* parent) : QObject(parent)
{
}

void MusicBrainzApi::sendGetRequest(const Locale& locale, const QUrl& url, MusicBrainzApi::ApiCallback callback)
{
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    request.setRawHeader("Accept", "application/xml");
    // The language cookie is only used for the artist's biography which is taken from Wikipedia, e.g.
    // https://musicbrainz.org/artist/65f4f0c5-ef9e-490c-aee3-909e7ae6b2ab/wikipedia-extract
    QList<QNetworkCookie> cookies;
    cookies << QNetworkCookie("lang", locale.toString().toUtf8());
    request.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue(cookies));

    if (m_network.cache().hasValidElement(request)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(0, this, [cb = std::move(callback), element = m_network.cache().getElement(request)]() { //
            cb(element, {});
        });
        return;
    }
    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), locale, request, this]() {
        auto dls = makeDeleteLaterScope(reply);

        QString data;
        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());

        } else {
            qCWarning(generic) << "[MusicBrainz] Network Error:" << reply->errorString() << "for URL" << reply->url();
            // For debugging: << reply->readAll();
        }

        if (!data.isEmpty()) {
            m_network.cache().addElement(request, data);
        }

        ScraperError error = makeScraperError(data, *reply, {});
        cb(data, error);
    });
}

void MusicBrainzApi::searchForArtist(const Locale& locale, const QString& query, MusicBrainzApi::ApiCallback callback)
{
    QUrl url = makeArtistSearchUrl(query);
    return sendGetRequest(locale, url, std::move(callback));
}

void MusicBrainzApi::searchForAlbum(const Locale& locale, const QString& query, MusicBrainzApi::ApiCallback callback)
{
    QUrl url = makeAlbumSearchUrl(query);
    return sendGetRequest(locale, url, std::move(callback));
}

void MusicBrainzApi::searchForAlbumWithArtist(const Locale& locale,
    const QString& albumQuery,
    const QString& artistName,
    MusicBrainzApi::ApiCallback callback)
{
    QUrl url = makeAlbumWithArtistSearchUrl(albumQuery, artistName);
    return sendGetRequest(locale, url, std::move(callback));
}


QVector<ScraperSearchResult> MusicBrainzApi::parseArtistSearchPage(const QString& html)
{
    QVector<ScraperSearchResult> results;
    QDomDocument domDoc;
    domDoc.setContent(html);
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
    return results;
}

QVector<ScraperSearchResult> MusicBrainzApi::parseAlbumSearchPage(const QString& html)
{
    QVector<ScraperSearchResult> results;

    QDomDocument domDoc;
    domDoc.setContent(html);
    for (int i = 0, releaseCount = domDoc.elementsByTagName("release").count(); i < releaseCount; ++i) {
        QDomElement elem = domDoc.elementsByTagName("release").at(i).toElement();
        QString name;
        if (!elem.elementsByTagName("title").isEmpty()) {
            name = elem.elementsByTagName("title").at(0).toElement().text();
        } else {
            continue;
        }

        QMap<QString, int> mediumList;
        if (!elem.elementsByTagName("medium-list").isEmpty()) {
            for (int j = 0, n = elem.elementsByTagName("medium-list").count(); j < n; ++j) {
                QDomElement mediumElem = elem.elementsByTagName("medium-list").at(j).toElement();
                if (mediumElem.elementsByTagName("format").isEmpty()) {
                    continue;
                }
                QString medium = mediumElem.elementsByTagName("format").at(0).toElement().text();
                mediumList.insert(medium, mediumList.value(medium, 0) + 1);
            }
        }
        QStringList media;
        QMapIterator<QString, int> it(mediumList);
        while (it.hasNext()) {
            it.next();
            if (it.value() > 1) {
                media << QString("%1x%2").arg(it.value()).arg(it.key());
            } else {
                media << it.key();
            }
        }

        if (!media.isEmpty()) {
            name += QString(", %1").arg(media.join(" + "));
        }

        if (!elem.elementsByTagName("disambiguation").isEmpty()) {
            name += QString(", %1").arg(elem.elementsByTagName("disambiguation").at(0).toElement().text());
        }

        if (!elem.elementsByTagName("date").isEmpty()) {
            name += QString(" (%1)").arg(elem.elementsByTagName("date").at(0).toElement().text());
        }

        if (!elem.elementsByTagName("country").isEmpty()) {
            name += QString(" %1").arg(elem.elementsByTagName("country").at(0).toElement().text());
        }

        ScraperSearchResult result;
        result.id = elem.attribute("id");
        result.name = name;
        if (!elem.elementsByTagName("release-group").isEmpty()) {
            result.id2 = elem.elementsByTagName("release-group").at(0).toElement().attribute("id");
        }
        results.append(result);
    }
    return results;
}

QUrl MusicBrainzApi::makeArtistSearchUrl(const QString& query)
{
    QUrl url(QString("https://musicbrainz.org/ws/2/artist/?query=artist:\"%1\"")
                 .arg(QString(QUrl::toPercentEncoding(query))));
    return url;
}

QUrl MusicBrainzApi::makeAlbumSearchUrl(const QString& query)
{
    QUrl url(QStringLiteral("https://musicbrainz.org/ws/2/release/?query=release:\"%1\"")
                 .arg(QString(QUrl::toPercentEncoding(query))));
    return url;
}

QUrl MusicBrainzApi::makeAlbumWithArtistSearchUrl(const QString& albumQuery, const QString& artistName)
{
    QUrl url(QStringLiteral("https://musicbrainz.org/ws/2/release/?query=release:\"%1\"%20AND%20artist:\"%2\"")
                 .arg(QString(QUrl::toPercentEncoding(albumQuery)), QString(QUrl::toPercentEncoding(artistName))));
    return url;
}

QUrl MusicBrainzApi::makeArtistBiographyUrl(const MusicBrainzId& artistId)
{
    // Response is JSON
    QUrl url(QStringLiteral("https://musicbrainz.org/artist/%1/wikipedia-extract").arg(artistId.toString()));
    return url;
}

void MusicBrainzApi::loadArtist(const Locale& locale,
    const MusicBrainzId& artistId,
    MusicBrainzApi::ApiCallback callback)
{
    QUrl url(QStringLiteral("https://musicbrainz.org/ws/2/artist/%1?inc=url-rels").arg(artistId.toString()));
    return sendGetRequest(locale, url, std::move(callback));
}

void MusicBrainzApi::loadAlbum(const Locale& locale, const MusicBrainzId& albumId, MusicBrainzApi::ApiCallback callback)
{
    QUrl url(QStringLiteral("https://musicbrainz.org/ws/2/release/%1?inc=url-rels+labels+artist-credits")
                 .arg(albumId.toString()));
    return sendGetRequest(locale, url, std::move(callback));
}

void MusicBrainzApi::loadReleaseGroup(const Locale& locale,
    const MusicBrainzId& groupId,
    MusicBrainzApi::ApiCallback callback)
{
    QUrl url(QStringLiteral("https://musicbrainz.org/ws/2/release-group/%1?inc=url-rels+artist-credits")
                 .arg(groupId.toString()));
    return sendGetRequest(locale, url, std::move(callback));
}

MusicBrainz::MusicBrainz(QObject* parent) : QObject(parent)
{
}

void MusicBrainz::parseAndAssignAlbum(const QString& xml, Album& album, const QSet<MusicScraperInfo>& infos)
{
    QDomDocument domDoc;
    domDoc.setContent(xml);

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Title, infos, album)
        && !domDoc.elementsByTagName("title").isEmpty()) {
        album.setTitle(domDoc.elementsByTagName("title").at(0).toElement().text());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Artist, infos, album)
        && !domDoc.elementsByTagName("artist-credit").isEmpty()) {
        QString artist;
        QString joinPhrase;
        QDomNodeList artistList =
            domDoc.elementsByTagName("artist-credit").at(0).toElement().elementsByTagName("name-credit");
        for (int i = 0, n = artistList.count(); i < n; ++i) {
            QDomElement artistElem = artistList.at(i).toElement();
            joinPhrase = artistElem.attribute("joinphrase");
            if (artistElem.elementsByTagName("artist").isEmpty()) {
                continue;
            }
            if (artistElem.elementsByTagName("artist").at(0).toElement().elementsByTagName("name").isEmpty()) {
                continue;
            }

            if (!artist.isEmpty()) {
                artist.append(joinPhrase.isEmpty() ? ", " : joinPhrase);
            }
            artist.append(artistElem.elementsByTagName("artist")
                              .at(0)
                              .toElement()
                              .elementsByTagName("name")
                              .at(0)
                              .toElement()
                              .text());
        }
        if (!artist.isEmpty()) {
            album.setArtist(artist);
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Label, infos, album)
        && !domDoc.elementsByTagName("label-info-list").isEmpty()) {
        QStringList labels;
        QDomNodeList labelList =
            domDoc.elementsByTagName("label-info-list").at(0).toElement().elementsByTagName("label-info");
        for (int i = 0, n = labelList.count(); i < n; ++i) {
            QDomElement labelElem = labelList.at(i).toElement();
            if (labelElem.elementsByTagName("label").isEmpty()) {
                continue;
            }
            if (labelElem.elementsByTagName("label").at(0).toElement().elementsByTagName("name").isEmpty()) {
                continue;
            }
            labels << labelElem.elementsByTagName("label")
                          .at(0)
                          .toElement()
                          .elementsByTagName("name")
                          .at(0)
                          .toElement()
                          .text();
        }
        if (!labels.isEmpty()) {
            album.setLabel(labels.join(", "));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::ReleaseDate, infos, album)
        && !domDoc.elementsByTagName("release-event-list").isEmpty()) {
        QDomNodeList releaseList =
            domDoc.elementsByTagName("release-event-list").at(0).toElement().elementsByTagName("release-event");
        if (!releaseList.isEmpty() && !releaseList.at(0).toElement().elementsByTagName("date").isEmpty()) {
            album.setReleaseDate(releaseList.at(0).toElement().elementsByTagName("date").at(0).toElement().text());
        }
    }
}

void MusicBrainz::parseAndAssignArtist(const QString& data, Artist& artist, const QSet<MusicScraperInfo>& infos)
{
    if (data.isEmpty()) {
        return;
    }

    if (!UniversalMusicScraper::shouldLoad(MusicScraperInfo::Biography, infos, artist)) {
        return;
    }

    QJsonParseError parseError{};
    QJsonDocument json = QJsonDocument::fromJson(data.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        return;
    }

    QString biography = json.object()["wikipediaExtract"].toObject()["content"].toString();
    if (!biography.isEmpty()) {
        artist.setBiography(removeHtmlEntities(biography));
    }
}

QPair<AllMusicId, QString> MusicBrainz::extractAllMusicIdAndDiscogsUrl(const QString& xml)
{
    QString discogsUrl;
    AllMusicId allMusicId;
    QDomDocument domDoc;
    domDoc.setContent(xml);

    for (int i = 0, n = domDoc.elementsByTagName("relation").count(); i < n; ++i) {
        QDomElement elem = domDoc.elementsByTagName("relation").at(i).toElement();
        if (elem.attribute("type") == "allmusic" && elem.elementsByTagName("target").count() > 0) {
            QString url = elem.elementsByTagName("target").at(0).toElement().text();
            QRegularExpression rx("allmusic\\.com/album/(.*)$");
            QRegularExpressionMatch match = rx.match(url);
            if (match.hasMatch()) {
                allMusicId = AllMusicId(match.captured(1));
            }
        }
        if (elem.attribute("type") == "discogs" && elem.elementsByTagName("target").count() > 0) {
            discogsUrl = elem.elementsByTagName("target").at(0).toElement().text();
        }
    }
    return {allMusicId, discogsUrl};
}

} // namespace scraper
} // namespace mediaelch
