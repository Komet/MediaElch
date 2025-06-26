#include "scrapers/music/TheAudioDb.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/music/UniversalMusicScraper.h"
#include "utils/Meta.h"

#include <QJsonArray>

namespace mediaelch {
namespace scraper {

TheAudioDbApi::TheAudioDbApi(QObject* parent) : QObject(parent), m_tadbApiKey{"7490823590829082posuda"}
{
}

void TheAudioDbApi::sendGetRequest(const Locale& locale, const QUrl& url, TheAudioDbApi::ApiCallback callback)
{
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);

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
        }

        if (!data.isEmpty()) {
            m_network.cache().addElement(request, data);
        }

        ScraperError error = makeScraperError(data, *reply, {});
        cb(data, error);
    });
}

QUrl TheAudioDbApi::makeArtistUrl(const MusicBrainzId& artistId)
{
    return QUrl(QStringLiteral("https://www.theaudiodb.com/api/v1/json/%1/artist-mb.php?i=%2")
            .arg(m_tadbApiKey, artistId.toString()));
}

QUrl TheAudioDbApi::makeArtistUrl(const TheAudioDbId& artistId)
{
    return QUrl(QStringLiteral("https://www.theaudiodb.com/api/v1/json/%1/artist.php?i=%2")
            .arg(m_tadbApiKey, artistId.toString()));
}

QUrl TheAudioDbApi::makeArtistDiscographyUrl(const MusicBrainzId& artistId)
{
    return QUrl(QStringLiteral("https://www.theaudiodb.com/api/v1/json/%1/discography-mb.php?s=%2")
            .arg(m_tadbApiKey, artistId.toString()));
}

QUrl TheAudioDbApi::makeArtistDiscographyUrl(const TheAudioDbId& artistId)
{
    return QUrl(QStringLiteral("https://www.theaudiodb.com/api/v1/json/%1/discography.php?s=%2")
            .arg(m_tadbApiKey, artistId.toString()));
}

TheAudioDb::TheAudioDb(QObject* parent) : QObject(parent)
{
}

void TheAudioDb::parseAndAssignArtist(QJsonObject document,
    Artist& artist,
    const QSet<MusicScraperInfo>& infos,
    const QString& lang)
{
    // The JSON document contains an array "artists". We take the first one.
    const auto tadbArtist = document.value("artists").toArray().first().toObject();

    if (!tadbArtist.value("strMusicBrainzID").toString().isEmpty()) {
        artist.setMbId(MusicBrainzId(tadbArtist.value("strMusicBrainzID").toString()));
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Name, infos, artist)
        && !tadbArtist.value("strArtist").toString().isEmpty()) {
        artist.setName(tadbArtist.value("strArtist").toString());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Died, infos, artist)) {
        artist.setDied(tadbArtist.value("intDiedYear").toString());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Formed, infos, artist)) {
        artist.setFormed(tadbArtist.value("intFormedYear").toString());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Born, infos, artist)) {
        artist.setBorn(tadbArtist.value("intBornYear").toString());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Disbanded, infos, artist)) {
        artist.setDisbanded(tadbArtist.value("strDisbanded").toString());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Genres, infos, artist)
        && tadbArtist.value("strGenre").toString() != "...") {
        artist.addGenre(tadbArtist.value("strGenre").toString());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Styles, infos, artist)
        && tadbArtist.value("strStyle").toString() != "...") {
        artist.addStyle(tadbArtist.value("strStyle").toString());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Moods, infos, artist)
        && tadbArtist.value("strMood").toString() != "...") {
        artist.addMood(tadbArtist.value("strMood").toString());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Biography, infos, artist)) {
        const auto biography = tadbArtist.value("strBiography" + lang.toUpper()).toString();
        const auto biographyEN = tadbArtist.value("strBiographyEN").toString();
        if (!biography.isEmpty()) {
            artist.setBiography(biography);
        } else if (!biographyEN.isEmpty()) {
            artist.setBiography(biographyEN);
        }
    }
}

void TheAudioDb::parseAndAssignAlbum(QJsonObject document,
    Album& album,
    const QSet<MusicScraperInfo>& infos,
    const QString& lang)
{
    // The JSON document contains an array "album". We take the first one.
    const auto tadbAlbum = document.value("album").toArray().first().toObject();

    album.setMbReleaseGroupId(MusicBrainzId(tadbAlbum.value("strMusicBrainzID").toString()));

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Title, infos, album)) {
        album.setTitle(tadbAlbum.value("strAlbum").toString());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Artist, infos, album)) {
        album.setArtist(tadbAlbum.value("strArtist").toString());
    }

    // The rating is encoded as a string, even though the JSON property is called "int".
    // Example: <https://theaudiodb.com/api/v1/json/2/album.php?m=2115888>
    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Rating, infos, album)) {
        bool ok = false;
        const qreal score = tadbAlbum.value("intScore").toString().toDouble(&ok);
        if (ok && score > 0) {
            album.setRating(score);
        }
    }

    // The year is encoded as a string, even though the JSON property is called "int".
    // Example: <https://theaudiodb.com/api/v1/json/2/album.php?m=2115888>
    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Year, infos, album)) {
        bool ok = false;
        const int year = tadbAlbum.value("intYearReleased").toString().toInt(&ok);
        if (ok && year > 0) {
            album.setYear(year);
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Genres, infos, album)
        && tadbAlbum.value("strGenre").toString() != "...") {
        album.addGenre(tadbAlbum.value("strGenre").toString());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Styles, infos, album)
        && tadbAlbum.value("strStyle").toString() != "...") {
        album.addStyle(tadbAlbum.value("strStyle").toString());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Moods, infos, album)
        && tadbAlbum.value("strMood").toString() != "...") {
        album.addMood(tadbAlbum.value("strMood").toString());
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Review, infos, album)) {
        const auto review = tadbAlbum.value("strDescription" + lang.toUpper()).toString();
        const auto reviewEN = tadbAlbum.value("strDescriptionEN").toString();
        if (!review.isEmpty()) {
            album.setReview(review);
        } else if (!reviewEN.isEmpty()) {
            album.setReview(reviewEN);
        }
    }
}

void TheAudioDb::parseAndAssignArtistDiscography(QJsonObject document,
    Artist& artist,
    const QSet<MusicScraperInfo>& infos)
{
    if (!UniversalMusicScraper::shouldLoad(MusicScraperInfo::Discography, infos, artist)) {
        return;
    }

    const auto tadbAlbums = document.value("album").toArray();

    for (const auto& albumValue : tadbAlbums) {
        const auto album = albumValue.toObject();
        DiscographyAlbum a;
        a.title = album.value("strAlbum").toString();
        // Even though "int" is part of the property, the value is a string.
        a.year = album.value("intYearReleased").toString();
        if (!a.title.isEmpty() || !a.year.isEmpty()) {
            artist.addDiscographyAlbum(a);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
