#include "UniversalMusicScraper.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/music/MusicMerger.h"
#include "scrapers/music/UniversalMusicConfiguration.h"
#include "ui/main/MainWindow.h"

#include <QDomDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <algorithm>

namespace mediaelch {
namespace scraper {

UniversalArtistSearchJob::UniversalArtistSearchJob(UniversalMusicScraper* scraper, Config config, QObject* parent) :
    ArtistSearchJob(std::move(config), parent), m_scraper{scraper}
{
}

void UniversalArtistSearchJob::doStart()
{
    m_scraper->m_musicBrainzApi.searchForArtist(
        config().locale, config().query, [this](QString html, ScraperError error) {
            if (error.hasError()) {
                setScraperError(error);
                emitFinished();
                return;
            }

            QVector<ScraperSearchResult> results = m_scraper->m_musicBrainzApi.parseArtistSearchPage(html);
            for (const ScraperSearchResult& result : results) {
                MediaElch_Debug_Expects(result.id2.isEmpty());
                ArtistSearchJob::Result searchResult;
                searchResult.identifier = result.id;
                searchResult.title = result.name;
                m_results.push_back(searchResult);
            }
            emitFinished();
        });
}


UniversalAlbumSearchJob::UniversalAlbumSearchJob(UniversalMusicScraper* scraper, Config config, QObject* parent) :
    AlbumSearchJob(std::move(config), parent), m_scraper{scraper}
{
}

void UniversalAlbumSearchJob::doStart()
{
    QString year;
    QString cleanSearchStr = config().albumQuery;
    QRegularExpression rx("^(.*)([0-9]{4})\\)?$");
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption);

    QRegularExpressionMatch match = rx.match(config().albumQuery);
    if (match.hasMatch()) {
        year = match.captured(2);
        cleanSearchStr = match.captured(1);
    }

    rx.setPattern("^\\(?([0-9]{4})\\)?(.*)$");
    match = rx.match(config().albumQuery);
    if (match.hasMatch()) {
        year = match.captured(1);
        cleanSearchStr = match.captured(2);
    }
    cleanSearchStr.replace("(", "");
    cleanSearchStr.replace(")", "");
    cleanSearchStr.replace("[", "");
    cleanSearchStr.replace("]", "");
    cleanSearchStr.replace("-", "");
    cleanSearchStr = cleanSearchStr.trimmed();
    if (cleanSearchStr.isEmpty()) {
        cleanSearchStr = config().albumQuery;
    }

    const auto callback = [this](QString html, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
            return;
        }

        QVector<ScraperSearchResult> results = m_scraper->m_musicBrainzApi.parseAlbumSearchPage(html);
        for (const ScraperSearchResult& result : results) {
            AlbumSearchJob::Result searchResult;
            searchResult.identifier = result.id;
            searchResult.groupIdentifier = result.id2;
            searchResult.title = result.name;
            m_results.push_back(searchResult);
        }
        emitFinished();
    };

    if (config().artistName.isEmpty()) {
        m_scraper->m_musicBrainzApi.searchForAlbum(config().locale, cleanSearchStr, callback);
    } else {
        m_scraper->m_musicBrainzApi.searchForAlbumWithArtist(
            config().locale, cleanSearchStr, config().artistName, callback);
    }
}

UniversalArtistScrapeJob::UniversalArtistScrapeJob(UniversalMusicScraper& scraper,
    UniversalMusicConfiguration& settings,
    Config config,
    QObject* parent) :
    ArtistScrapeJob(std::move(config), parent), m_scraper{scraper}, m_settings{settings}
{
}

void UniversalArtistScrapeJob::doStart()
{
    // Otherwise deleted images are showing up again
    // TODO: What? What is this madness? Aaaaahhhh... Debug this and remove these lines.
    auto infos = config().details;
    infos.remove(MusicScraperInfo::ExtraFanarts);

    MusicBrainzId mbId{config().identifier};

    artist().clear(infos);
    artist().setMbId(mbId);
    artist().setAllMusicId(AllMusicId::NoId);

    m_scraper.m_musicBrainzApi.loadArtist(config().locale, mbId, [infos, this](QString html, ScraperError error) {
        QString discogsId;
        if (!error.hasError()) {
            QDomDocument domDoc;
            domDoc.setContent(html);
            for (int i = 0, n = domDoc.elementsByTagName("relation").count(); i < n; ++i) {
                QDomElement elem = domDoc.elementsByTagName("relation").at(i).toElement();
                if (elem.attribute("type") == "allmusic" && elem.elementsByTagName("target").count() > 0) {
                    QString url = elem.elementsByTagName("target").at(0).toElement().text();
                    QRegularExpression rx("allmusic\\.com/artist/(.*)$");
                    QRegularExpressionMatch match = rx.match(url);
                    if (match.hasMatch()) {
                        artist().setAllMusicId(AllMusicId(match.captured(1)));
                    }
                }
                if (elem.attribute("type") == "discogs" && elem.elementsByTagName("target").count() > 0) {
                    QString discogsUrl = elem.elementsByTagName("target").at(0).toElement().text();
                    static QRegularExpression discogsIdRegEx("/(\\d+)$");
                    MediaElch_Debug_Ensures(discogsIdRegEx.isValid());
                    QRegularExpressionMatch match = discogsIdRegEx.match(discogsUrl);
                    if (match.hasMatch()) {
                        discogsId = match.captured(1);
                    }
                }
            }
        }

        const auto& artistMbId = artist().mbId();

        appendDownloadElement(
            "musicbrainz", "musicbrainz_biography", m_scraper.m_musicBrainzApi.makeArtistBiographyUrl(artistMbId));

        appendDownloadElement("theaudiodb", "tadb_data", m_scraper.m_theAudioDbApi.makeArtistUrl(artistMbId));
        appendDownloadElement(
            "theaudiodb", "tadb_discography", m_scraper.m_theAudioDbApi.makeArtistDiscographyUrl(artistMbId));

        const auto& amId = artist().allMusicId();
        if (amId.isValid()) {
            appendDownloadElement("allmusic", "am_data", m_scraper.m_allMusicApi.makeArtistUrl(amId));
            appendDownloadElement("allmusic", "am_biography", m_scraper.m_allMusicApi.makeArtistBiographyUrl(amId));
            appendDownloadElement("allmusic", "am_moods", m_scraper.m_allMusicApi.makeArtistMoodsUrl(amId));
        }
        if (!discogsId.isEmpty()) {
            // TODO: This is currently a hack: The proper order in which fields are used is determined in
            // checkIfFinished(). We can't simply apply the artist's details here.
            DiscogsArtistScrapeJob::Config discogsConfig = config();
            discogsConfig.identifier = discogsId;
            auto* discogsJob = m_scraper.m_discogs.loadArtist(discogsConfig);
            connect(discogsJob, &DiscogsArtistScrapeJob::finished, this, [discogsJob, this]() {
                auto dls = makeDeleteLaterScope(discogsJob);
                copyDetailsToArtist(m_discogsArtist, discogsJob->artist(), allMusicScraperInfos());
                m_discogsFinished = true;
                checkIfFinished();
            });
            discogsJob->start();
        } else {
            m_discogsFinished = true;
        }

        for (const DownloadElement& elem : asConst(m_artistDownloads)) {
            QNetworkRequest request(elem.url);
            mediaelch::network::useFirefoxUserAgent(request);
            if (elem.source == "musicbrainz") {
                request.setRawHeader("Accept-Language", config().locale.toString().toUtf8());
            } else if (elem.source == "allmusic") {
                // TODO(refactor): For biography/moods we need a proper referrer header.
                QUrl referrer = m_scraper.m_allMusicApi.makeArtistUrl(amId);
                request.setRawHeader("Referer", referrer.toString().toUtf8());
            }

            QNetworkReply* elemReply = m_scraper.m_network.getWithWatcher(request);
            connect(elemReply, &QNetworkReply::finished, this, &UniversalArtistScrapeJob::onArtistLoadFinished);
        }
    });
}

void UniversalArtistScrapeJob::onArtistLoadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    elch_ssize_t index = -1;
    for (elch_ssize_t i = 0, n = m_artistDownloads.count(); i < n; ++i) {
        if (m_artistDownloads[i].url == reply->url()) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        m_artistDownloads[index].contents = QString::fromUtf8(reply->readAll());
    } else {
        qCWarning(generic) << "[UniversalMusicScraper] Network Error while loading artist:" << reply->errorString();
    }
    m_artistDownloads[index].downloaded = true;
    checkIfFinished();
}

void UniversalArtistScrapeJob::checkIfFinished()
{
    if (!m_discogsFinished) {
        return;
    }
    bool finished = true;
    for (elch_ssize_t i = 0, n = m_artistDownloads.count(); i < n; ++i) {
        if (!m_artistDownloads[i].downloaded) {
            finished = false;
            break;
        }
    }

    if (!finished) {
        return;
    }

    // First parse the preferred details
    QString preferred = m_settings.preferredScraper();
    for (const DownloadElement& elem : asConst(m_artistDownloads)) {
        if (elem.source == preferred) {
            processDownloadElement(elem);
        }
    }
    // Then parse the rest. Details are only updated, if fields are empty.
    for (const DownloadElement& elem : asConst(m_artistDownloads)) {
        if (elem.source != preferred) {
            processDownloadElement(elem);
        }
    }

    copyDetailsToArtistIfEmpty(artist(), m_discogsArtist, config().details);

    emitFinished();
}

void UniversalArtistScrapeJob::appendDownloadElement(QString source, QString type, QUrl url)
{
    DownloadElement elem;
    elem.type = std::move(type);
    elem.url = std::move(url);
    elem.downloaded = false;
    elem.source = std::move(source);
    m_artistDownloads.append(std::move(elem));
}

void UniversalArtistScrapeJob::processDownloadElement(DownloadElement elem)
{
    // If there is no content, i.e. an empty string, then there was
    // a network error, or we got an empty result.
    if (elem.contents.isEmpty()) {
        return;
    }

    if (elem.type.startsWith("tadb_")) {
        QJsonParseError parseError{};
        const auto parsedJson = QJsonDocument::fromJson(elem.contents.toUtf8(), &parseError).object();
        if (parseError.error != QJsonParseError::NoError) {
            qCWarning(generic) << "Error parsing music json: " << parseError.errorString();
            return;
        }

        if (elem.type == "tadb_data") {
            m_scraper.m_theAudioDb.parseAndAssignArtist(
                parsedJson, artist(), config().details, config().locale.toString());
        } else if (elem.type == "tadb_discography") {
            m_scraper.m_theAudioDb.parseAndAssignArtistDiscography(parsedJson, artist(), config().details);
        }
    } else if (elem.type == "musicbrainz_biography") {
        m_scraper.m_musicBrainz.parseAndAssignArtist(elem.contents, artist(), config().details);
    } else if (elem.type == "am_data") {
        m_scraper.m_allMusic.parseAndAssignArtist(elem.contents, artist(), config().details);
    } else if (elem.type == "am_biography") {
        m_scraper.m_allMusic.parseAndAssignArtistBiography(elem.contents, artist(), config().details);
    } else if (elem.type == "am_moods") {
        m_scraper.m_allMusic.parseAndAssignArtistMoods(elem.contents, artist(), config().details);
    }
}

UniversalAlbumScrapeJob::UniversalAlbumScrapeJob(UniversalMusicScraper& scraper,
    UniversalMusicConfiguration& settings,
    Config config,
    QObject* parent) :
    AlbumScrapeJob(std::move(config), parent), m_scraper{scraper}, m_settings{settings}
{
}

void UniversalAlbumScrapeJob::doStart()
{
    MusicBrainzId mbAlbumId{config().identifier};
    MusicBrainzId mbReleaseGroupId{config().groupIdentifier};

    album().setMbAlbumId(mbAlbumId);
    album().setMbReleaseGroupId(mbReleaseGroupId);
    album().setAllMusicId(AllMusicId::NoId);

    auto onLoadFinished = [this](QString html, ScraperError error) {
        QString discogsId;
        if (!error.hasError()) {
            m_scraper.m_musicBrainz.parseAndAssignAlbum(html, album(), config().details);
            auto ids = MusicBrainz::extractAllMusicIdAndDiscogsUrl(html);
            if (ids.first.isValid()) {
                album().setAllMusicId(ids.first);
            }
            QString discogsUrl = ids.second;
            static QRegularExpression discogsIdRegEx("/(\\d+)$");
            MediaElch_Debug_Ensures(discogsIdRegEx.isValid());
            QRegularExpressionMatch match = discogsIdRegEx.match(discogsUrl);
            if (match.hasMatch()) {
                discogsId = match.captured(1);
            }
        }

        appendDownloadElement("theaudiodb",
            "tadb_data",
            QUrl(QStringLiteral("https://www.theaudiodb.com/api/v1/json/%1/album-mb.php?i=%2")
                     .arg(m_scraper.m_tadbApiKey, album().mbReleaseGroupId().toString())));

        const auto& amId = album().allMusicId();
        if (amId.isValid()) {
            appendDownloadElement("allmusic", "am_data", m_scraper.m_allMusicApi.makeAlbumUrl(amId));
            appendDownloadElement("allmusic", "am_review", m_scraper.m_allMusicApi.makeAlbumReviewUrl(amId));
            appendDownloadElement("allmusic", "am_moods", m_scraper.m_allMusicApi.makeAlbumMoodsUrl(amId));
        }

        if (!discogsId.isEmpty()) {
            // TODO: This is currently a hack: The proper order in which fields are used is determined in
            // checkIfFinished(). We can't simply apply the artist's details here.
            DiscogsAlbumScrapeJob::Config discogsConfig = config();
            discogsConfig.identifier = discogsId;
            discogsConfig.groupIdentifier.clear();
            auto* discogsJob = m_scraper.m_discogs.loadAlbum(discogsConfig);
            connect(discogsJob, &DiscogsAlbumScrapeJob::finished, this, [discogsJob, this]() {
                auto dls = makeDeleteLaterScope(discogsJob);
                copyDetailsToAlbum(m_discogsAlbum, discogsJob->album(), allMusicScraperInfos());
                m_discogsFinished = true;
                checkIfFinished();
            });
            discogsJob->start();
        } else {
            m_discogsFinished = true;
        }

        for (const DownloadElement& elem : asConst(m_albumDownloads)) {
            QNetworkRequest request(elem.url);
            mediaelch::network::useFirefoxUserAgent(request);

            if (elem.source == "musicbrainz") {
                request.setRawHeader("Accept-Language", config().locale.toString().toUtf8());
            } else if (elem.source == "allmusic") {
                // TODO(refactor): For review/moods we need a proper referrer header.
                QUrl referrer = m_scraper.m_allMusicApi.makeAlbumUrl(amId);
                request.setRawHeader("Referer", referrer.toString().toUtf8());
            }

            QNetworkReply* elemReply = m_scraper.m_network.getWithWatcher(request);
            connect(elemReply, &QNetworkReply::finished, this, &UniversalAlbumScrapeJob::onAlbumLoadFinished);
        }
    };

    m_scraper.m_musicBrainzApi.loadAlbum(
        config().locale, mbAlbumId, [onLoadFinished, this](QString html, ScraperError error) {
            // MusicBrainz only provides a direct AllMusicId ID for _very few_ albums.
            // But for the release group, it provides an ID.  The release group is good enough
            // for loading the album from AllMusic.
            if (album().allMusicId().isValid()) {
                onLoadFinished(html, error);
                return;
            }
            m_scraper.m_musicBrainzApi.loadReleaseGroup(config().locale,
                album().mbReleaseGroupId(),
                [this, onLoadFinished, html, error](QString releaseGroupHtml, ScraperError releaseGroupError) { //
                    if (!releaseGroupError.hasError()) {
                        const auto ids = MusicBrainz::extractAllMusicIdAndDiscogsUrl(releaseGroupHtml);
                        if (ids.first.isValid()) {
                            album().setAllMusicId(ids.first);
                        }
                    }
                    onLoadFinished(html, error);
                });
        });
}

void UniversalAlbumScrapeJob::onAlbumLoadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    elch_ssize_t index = -1;
    for (elch_ssize_t i = 0, n = m_albumDownloads.count(); i < n; ++i) {
        if (m_albumDownloads[i].url == reply->url()) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        m_albumDownloads[index].contents = QString::fromUtf8(reply->readAll());
    } else {
        qCWarning(generic) << "[UniversalMusicScraper] Network Error while loading album:" << reply->errorString();
    }
    m_albumDownloads[index].downloaded = true;

    checkIfFinished();
}

void UniversalAlbumScrapeJob::checkIfFinished()
{
    if (!m_discogsFinished) {
        return;
    }

    bool finished = true;
    for (elch_ssize_t i = 0, n = m_albumDownloads.count(); i < n; ++i) {
        if (!m_albumDownloads[i].downloaded) {
            finished = false;
            break;
        }
    }

    if (!finished) {
        return;
    }

    // First apply the values of the preferred scraper,…
    QString preferred = m_settings.preferredScraper();
    if (preferred == "discogs") {
        copyDetailsToAlbumIfEmpty(album(), m_discogsAlbum, config().details);
    } else {
        for (const DownloadElement& elem : asConst(m_albumDownloads)) {
            if (elem.source != preferred) {
                continue;
            }
            processDownloadElement(elem);
        }
    }

    // …then the others.
    for (const DownloadElement& elem : asConst(m_albumDownloads)) {
        if (elem.source == preferred) {
            continue;
        }
        processDownloadElement(elem);
    }
    if (preferred != "discogs") {
        copyDetailsToAlbumIfEmpty(album(), m_discogsAlbum, config().details);
    }

    emitFinished();
}

void UniversalAlbumScrapeJob::processDownloadElement(DownloadElement elem)
{
    if (elem.type == "tadb_data") {
        if (elem.contents.isEmpty()) {
            return;
        }
        QJsonParseError parseError{};
        const auto parsedJson = QJsonDocument::fromJson(elem.contents.toUtf8(), &parseError).object();
        if (parseError.error != QJsonParseError::NoError) {
            qCWarning(generic) << "Error parsing music json: " << parseError.errorString();
            return;
        }

        m_scraper.m_theAudioDb.parseAndAssignAlbum(parsedJson, album(), config().details, config().locale.toString());

    } else if (elem.type == "am_data") {
        m_scraper.m_allMusic.parseAndAssignAlbum(elem.contents, album(), config().details);
    } else if (elem.type == "am_review") {
        m_scraper.m_allMusic.parseAndAssignAlbumReview(elem.contents, album(), config().details);
    } else if (elem.type == "am_moods") {
        m_scraper.m_allMusic.parseAndAssignAlbumMoods(elem.contents, album(), config().details);
    }
}

void UniversalAlbumScrapeJob::appendDownloadElement(QString source, QString type, QUrl url)
{
    DownloadElement elem;
    elem.type = std::move(type);
    elem.url = std::move(url);
    elem.downloaded = false;
    elem.source = std::move(source);
    m_albumDownloads.append(std::move(elem));
}

UniversalMusicScraper::UniversalMusicScraper(UniversalMusicConfiguration& settings, QObject* parent) :
    MusicScraper(parent), m_settings{settings}
{
    m_meta.identifier = ID;
    m_meta.name = "Universal Music Scraper";
    m_meta.description = tr("The Universal Music Scraper combines multiple scapers into one, including TheAudioDb, "
                            "MusicBrainz, and Discogs.");
    m_meta.website = "https://mediaelch.github.io/mediaelch-doc/";
    m_meta.termsOfService = "";
    m_meta.privacyPolicy = "";
    m_meta.help = "https://mediaelch.github.io/mediaelch-doc/movie/index.html";
    m_meta.supportedDetails = allMusicScraperInfos();
    m_meta.supportedLanguages = UniversalMusicConfiguration::supportedLanguages();
    m_meta.defaultLocale = UniversalMusicConfiguration::defaultLocale();
}

const UniversalMusicScraper::ScraperMeta& UniversalMusicScraper::meta() const
{
    return m_meta;
}

UniversalMusicScraper::~UniversalMusicScraper()
{
}

ArtistSearchJob* UniversalMusicScraper::searchArtist(ArtistSearchJob::Config config)
{
    return new UniversalArtistSearchJob(this, std::move(config), this);
}

ArtistScrapeJob* UniversalMusicScraper::loadArtist(ArtistScrapeJob::Config config)
{
    return new UniversalArtistScrapeJob(*this, m_settings, std::move(config), this);
}

AlbumSearchJob* UniversalMusicScraper::searchAlbum(AlbumSearchJob::Config config)
{
    return new UniversalAlbumSearchJob(this, std::move(config), this);
}

AlbumScrapeJob* UniversalMusicScraper::loadAlbum(AlbumScrapeJob::Config config)
{
    return new UniversalAlbumScrapeJob(*this, m_settings, std::move(config), this);
}

bool UniversalMusicScraper::shouldLoad(MusicScraperInfo info, const QSet<MusicScraperInfo>& infos, const Album& album)
{
    if (!infos.contains(info)) {
        return false;
    }

    switch (info) {
    case MusicScraperInfo::Title: return album.title().isEmpty();
    case MusicScraperInfo::Artist: return album.artist().isEmpty();
    case MusicScraperInfo::Review: return album.review().isEmpty();
    case MusicScraperInfo::ReleaseDate: return album.releaseDate().isEmpty();
    case MusicScraperInfo::Label: return album.label().isEmpty();
    case MusicScraperInfo::Rating: return album.rating() < 0.01 && album.rating() > -0.01;
    case MusicScraperInfo::Year: return album.year() == 0;
    case MusicScraperInfo::Genres: return album.genres().isEmpty();
    case MusicScraperInfo::Styles: return album.styles().isEmpty();
    case MusicScraperInfo::Moods: return album.moods().isEmpty();
    default: return false;
    }
}

bool UniversalMusicScraper::shouldLoad(MusicScraperInfo info, const QSet<MusicScraperInfo>& infos, const Artist& artist)
{
    if (!infos.contains(info)) {
        return false;
    }

    switch (info) {
    case MusicScraperInfo::Name: return artist.name().isEmpty();
    case MusicScraperInfo::YearsActive: return artist.yearsActive().isEmpty();
    case MusicScraperInfo::Formed: return artist.formed().isEmpty();
    case MusicScraperInfo::Born: return artist.born().isEmpty();
    case MusicScraperInfo::Died: return artist.died().isEmpty();
    case MusicScraperInfo::Disbanded: return artist.disbanded().isEmpty();
    case MusicScraperInfo::Biography: return artist.biography().isEmpty();
    case MusicScraperInfo::Genres: return artist.genres().isEmpty();
    case MusicScraperInfo::Styles: return artist.styles().isEmpty();
    case MusicScraperInfo::Moods: return artist.moods().isEmpty();
    case MusicScraperInfo::Discography: return artist.discographyAlbums().isEmpty();
    default: break;
    }

    return false;
}

} // namespace scraper
} // namespace mediaelch
