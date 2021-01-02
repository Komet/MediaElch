#include "scrapers/music/MusicBrainz.h"

#include "globals/Meta.h"
#include "music/Album.h"
#include "network/NetworkRequest.h"
#include "scrapers/music/UniversalMusicScraper.h"

#include <QDomDocument>
#include <QJsonDocument>

namespace mediaelch {
namespace scraper {

MusicBrainzApi::MusicBrainzApi(QObject* parent) : QObject(parent)
{
}

void MusicBrainzApi::sendGetRequest(const Locale& locale, const QUrl& url, MusicBrainzApi::ApiCallback callback)
{
    if (m_cache.hasValidElement(url, locale)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(0, [cb = std::move(callback), element = m_cache.getElement(url, locale)]() { //
            cb(element, {});
        });
        return;
    }

    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);

    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), locale, this]() {
        auto dls = makeDeleteLaterScope(reply);

        QString data;
        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());

        } else {
            qWarning() << "[MusicBrainz] Network Error:" << reply->errorString() << "for URL" << reply->url();
        }

        if (!data.isEmpty()) {
            m_cache.addElement(reply->url(), locale, data);
        }

        ScraperError error = makeScraperError(data, *reply, {});
        cb(data, error);
    });
}

void MusicBrainzApi::searchForArtist(const Locale& locale, const QString& query, MusicBrainzApi::ApiCallback callback)
{
    QUrl url(QString("https://musicbrainz.org/ws/2/artist/?query=artist:\"%1\"")
                 .arg(QString(QUrl::toPercentEncoding(query))));
    return sendGetRequest(locale, url, std::move(callback));
}

void MusicBrainzApi::searchForAlbum(const Locale& locale, const QString& query, MusicBrainzApi::ApiCallback callback)
{
    QUrl url(QString("https://musicbrainz.org/ws/2/release/?query=release:\"%1\"")
                 .arg(QString(QUrl::toPercentEncoding(query))));
    return sendGetRequest(locale, url, std::move(callback));
}

void MusicBrainzApi::searchForAlbumWithArtist(const Locale& locale,
    const QString& albumQuery,
    const QString& artistName,
    MusicBrainzApi::ApiCallback callback)
{
    QUrl url(QString("https://musicbrainz.org/ws/2/release/?query=release:\"%1\"%20AND%20artist:\"%2\"")
                 .arg(QString(QUrl::toPercentEncoding(albumQuery)), QString(QUrl::toPercentEncoding(artistName))));
    return sendGetRequest(locale, url, std::move(callback));
}

void MusicBrainzApi::loadArtist(const Locale& locale,
    const MusicBrainzId& artistId,
    MusicBrainzApi::ApiCallback callback)
{
    QUrl url(QString("https://musicbrainz.org/ws/2/artist/%1?inc=url-rels").arg(artistId.toString()));
    return sendGetRequest(locale, url, std::move(callback));
}

void MusicBrainzApi::loadAlbum(const Locale& locale, const MusicBrainzId& albumId, MusicBrainzApi::ApiCallback callback)
{
    QUrl url(
        QString("https://musicbrainz.org/ws/2/release/%1?inc=url-rels+labels+artist-credits").arg(albumId.toString()));
    return sendGetRequest(locale, url, std::move(callback));
}

MusicBrainz::MusicBrainz(QObject* parent) : QObject(parent)
{
}

void MusicBrainz::parseAndAssignAlbum(const QString& xml, Album* album, QSet<MusicScraperInfo> infos)
{
    QDomDocument domDoc;
    domDoc.setContent(xml);

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Title, infos, album)
        && !domDoc.elementsByTagName("title").isEmpty()) {
        album->setTitle(domDoc.elementsByTagName("title").at(0).toElement().text());
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
            album->setArtist(artist);
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
            album->setLabel(labels.join(", "));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::ReleaseDate, infos, album)
        && !domDoc.elementsByTagName("release-event-list").isEmpty()) {
        QDomNodeList releaseList =
            domDoc.elementsByTagName("release-event-list").at(0).toElement().elementsByTagName("release-event");
        if (!releaseList.isEmpty() && !releaseList.at(0).toElement().elementsByTagName("date").isEmpty()) {
            album->setReleaseDate(releaseList.at(0).toElement().elementsByTagName("date").at(0).toElement().text());
        }
    }
}

void MusicBrainz::parseAndAssignArtist(const QString& data, Artist* artist, QSet<MusicScraperInfo> infos)
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
        artist->setBiography(replaceCommonHtmlTags(biography));
    }
}

QString MusicBrainz::replaceCommonHtmlTags(QString text) const
{
    text.remove(QRegularExpression("<[apubi][^>]*?>"));
    text.remove(QRegularExpression("</[aubi]>"));
    text.remove("<small>");
    text.remove("</small>");
    text.remove(QRegularExpression("<span[^>]*?>"));
    text.remove("</span>");

    text.replace("</p>", " ");
    return text;
}

} // namespace scraper
} // namespace mediaelch
