#include "scrapers/music/Discogs.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "log/Log.h"
#include "scrapers/ScraperUtils.h"
#include "scrapers/music/UniversalMusicScraper.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>

namespace mediaelch {
namespace scraper {

void DiscogsApi::loadArtist(const QString& artistId, ApiCallback callback)
{
    sendGetRequest(makeArtistUrl(artistId), callback);
}

void DiscogsApi::loadArtistReleases(const QString& artistId, ApiCallback callback)
{
    sendGetRequest(makeArtistReleasesUrl(artistId), callback);
}

void DiscogsApi::loadAlbum(const QString& albumId, ApiCallback callback)
{
    sendGetRequest(makeAlbumUrl(albumId), callback);
}

QUrl DiscogsApi::makeAlbumUrl(const QString& artistId)
{
    return makeFullUrl(QStringLiteral("/masters/%1").arg(artistId));
}

QUrl DiscogsApi::makeArtistUrl(const QString& artistId)
{
    return makeFullUrl(QStringLiteral("/artists/%1").arg(artistId));
}

QUrl DiscogsApi::makeArtistReleasesUrl(const QString& artistId)
{
    QUrlQuery queries;
    queries.addQueryItem("sort", "year");
    queries.addQueryItem("sort_order", "asc");
    queries.addQueryItem("per_page", "100");
    return makeFullUrl(QStringLiteral("/artists/%1/releases?%2").arg(artistId, queries.toString()));
}

QUrl DiscogsApi::makeFullUrl(const QString& suffix)
{
    MediaElch_Debug_Expects(suffix.startsWith('/'));
    return {"https://api.discogs.com" + suffix};
}

void DiscogsApi::sendGetRequest(const QUrl& url, ApiCallback callback)
{
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    request.setRawHeader("Accept", "application/vnd.discogs.v2.plaintext+json");

    if (m_network.cache().hasValidElement(request)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(0, this, [cb = std::move(callback), element = m_network.cache().getElement(request)]() { //
            QJsonParseError parseError{};
            QJsonObject parsedJson = QJsonDocument::fromJson(element.toUtf8(), &parseError).object();
            ScraperError error = makeScraperError(element, parseError);
            MediaElch_Debug_Ensures(!error.hasError()); // cache should not have any errors
            cb(parsedJson, error);
        });
        return;
    }

    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), request, this]() {
        auto dls = makeDeleteLaterScope(reply);
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(generic) << "[Discogs][Api] Network Error:" << reply->errorString() << "for URL" << reply->url();
            ScraperError error = makeScraperError(QString::fromUtf8(reply->readAll()), *reply, {});
            cb({}, error);

        } else {
            QByteArray jsonBytes = reply->readAll();
            QString jsonStr = QString::fromUtf8(jsonBytes);
            QJsonParseError parseError{};
            QJsonObject parsedJson = QJsonDocument::fromJson(jsonBytes, &parseError).object();
            if (parseError.error == QJsonParseError::NoError && !parsedJson.isEmpty()) {
                m_network.cache().addElement(request, jsonStr);
            }
            ScraperError error = makeScraperError(jsonStr, parseError);
            cb(parsedJson, error);
        }
    });
}

void DiscogsArtistScrapeJob::doStart()
{
    m_api.loadArtist(config().identifier, [this](QJsonObject doc, ScraperError error) {
        if (error.hasError()) {
            setError(static_cast<int>(error.error));
            setErrorString(error.message);
            setErrorText(error.technical);
        } else {
            parseAndAssignArtist(doc);
        }
        m_artistLoaded = true;
        checkIfFinished();
    });
    m_api.loadArtistReleases(config().identifier, [this](QJsonObject doc, ScraperError error) {
        if (error.hasError()) {
            setError(static_cast<int>(error.error));
            setErrorString(error.message);
            setErrorText(error.technical);
        } else {
            parseAndAssignReleases(doc);
        }
        m_releasesLoaded = true;
        checkIfFinished();
    });
}

void DiscogsArtistScrapeJob::checkIfFinished()
{
    if (m_artistLoaded && m_releasesLoaded) {
        emitFinished();
    }
}

void DiscogsArtistScrapeJob::parseAndAssignArtist(const QJsonObject& artistObj)
{
    artist().setName(artistObj.value("name").toString());
    QString profile = artistObj.value("profile_plaintext").toString();
    if (!profile.isEmpty()) {
        mediaelch::removeUnicodeSpaces(profile);
        artist().setBiography(profile);
    }
}

void DiscogsArtistScrapeJob::parseAndAssignReleases(const QJsonObject& artistObj)
{
    QJsonArray releases = artistObj.value("releases").toArray();
    for (const QJsonValue& releaseVal : releases) {
        QJsonObject release = releaseVal.toObject();
        DiscographyAlbum album;
        album.title = release.value("title").toString();
        int year = release.value("year").toInt(-1);
        if (year > -1) {
            album.year = QString::number(year);
        }
        if (!album.title.isEmpty()) {
            artist().addDiscographyAlbum(album);
        }
    }
}

void DiscogsAlbumScrapeJob::doStart()
{
    m_api.loadAlbum(config().identifier, [this](QJsonObject doc, ScraperError error) {
        if (error.hasError()) {
            setError(static_cast<int>(error.error));
            setErrorString(error.message);
            setErrorText(error.technical);
        } else {
            parseAndAssignAlbum(doc);
        }
        emitFinished();
    });
}

void DiscogsAlbumScrapeJob::parseAndAssignAlbum(const QJsonObject& albumObj)
{
    // TODO: There are way more details

    // Example: https://api.discogs.com/releases/6495
    album().setTitle(albumObj.value("title").toString());

    QJsonArray artistsArray = albumObj.value("artists").toArray();
    QStringList artists;
    for (const QJsonValue& artistVal : artistsArray) {
        QJsonObject artistObj = artistVal.toObject();
        artists << artistObj.value("name").toString();
    }
    album().setArtist(artists.join(", "));

    QJsonArray genresArray = albumObj.value("genres").toArray();
    for (const QJsonValue& genre : genresArray) {
        album().addGenre(genre.toString());
    }

    QJsonArray stylesArray = albumObj.value("styles").toArray();
    for (const QJsonValue& style : stylesArray) {
        album().addStyle(style.toString());
    }

    int year = albumObj.value("year").toInt(-1);
    if (year > -1) {
        album().setYear(year);
    }
}

Discogs::Discogs(QObject* parent) : MusicScraper(parent)
{
    m_meta.identifier = ID;
    m_meta.name = "Discogs";
    m_meta.description = tr("Discogs is a database of information about audio recordings, including commercial "
                            "releases, promotional releases, and bootleg or off-label releases. ");
    m_meta.website = "https://www.discogs.com/";
    m_meta.termsOfService = "https://support.discogs.com/hc/articles/360009334333";
    m_meta.privacyPolicy = "https://support.discogs.com/hc/articles/360009334513";
    m_meta.help = "https://support.discogs.com/hc/";
    // TODO: Not true, Discogs does not support everything!
    m_meta.supportedDetails = allMusicScraperInfos();
    m_meta.supportedLanguages = {Locale::English};
    m_meta.defaultLocale = Locale::English;
}

const Discogs::ScraperMeta& Discogs::meta() const
{
    return m_meta;
}

ArtistScrapeJob* Discogs::loadArtist(ArtistScrapeJob::Config config)
{
    return new DiscogsArtistScrapeJob(m_api, std::move(config), this);
}

AlbumScrapeJob* Discogs::loadAlbum(AlbumScrapeJob::Config config)
{
    return new DiscogsAlbumScrapeJob(m_api, std::move(config), this);
}

} // namespace scraper
} // namespace mediaelch
