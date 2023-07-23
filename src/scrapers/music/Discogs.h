#pragma once

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "network/NetworkManager.h"
#include "network/NetworkRequest.h"
#include "scrapers/ScraperError.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/music/MusicScraper.h"

#include <QJsonObject>
#include <QObject>

class Artist;
class Album;

namespace mediaelch {
namespace scraper {

/// \brief Central Discogs API class. Contains knowledge about the Discogs API endpoint.
/// \details
///   This class provides methods for common API requests against the Discogs API.
///   For documentation, check out <https://www.discogs.com/developers/>.
class DiscogsApi final : public QObject
{
    Q_OBJECT

public:
    explicit DiscogsApi(QObject* parent = nullptr) : QObject(parent) {}
    ~DiscogsApi() override = default;

    void initialize() {}
    ELCH_NODISCARD bool isInitialized() const { return true; }

signals:
    void initialized();

public:
    using ApiCallback = std::function<void(QJsonObject, ScraperError)>;

    void loadArtist(const QString& artistId, ApiCallback callback);
    void loadArtistReleases(const QString& artistId, ApiCallback callback);
    void loadAlbum(const QString& albumId, ApiCallback callback);

private:
    ELCH_NODISCARD QUrl makeArtistUrl(const QString& artistId);
    ELCH_NODISCARD QUrl makeArtistReleasesUrl(const QString& artistId);
    ELCH_NODISCARD QUrl makeAlbumUrl(const QString& artistId);
    ELCH_NODISCARD QUrl makeFullUrl(const QString& suffix);

private:
    void sendGetRequest(const QUrl& url, ApiCallback callback);

private:
    mediaelch::network::NetworkManager m_network;
};

class DiscogsArtistScrapeJob final : public ArtistScrapeJob
{
    Q_OBJECT

public:
    explicit DiscogsArtistScrapeJob(DiscogsApi& api, Config config, QObject* parent = nullptr) :
        ArtistScrapeJob(std::move(config), parent), m_api{api}
    {
    }

private:
    void doStart() override;

    void parseAndAssignArtist(const QJsonObject& artist);
    void parseAndAssignReleases(const QJsonObject& artistObj);

    void checkIfFinished();

private:
    DiscogsApi& m_api;
    bool m_artistLoaded{false};
    bool m_releasesLoaded{false};
};

class DiscogsAlbumScrapeJob final : public AlbumScrapeJob
{
    Q_OBJECT

public:
    explicit DiscogsAlbumScrapeJob(DiscogsApi& api, Config config, QObject* parent = nullptr) :
        AlbumScrapeJob(std::move(config), parent), m_api{api}
    {
    }

private:
    void doStart() override;

    void parseAndAssignAlbum(const QJsonObject& albumObj);

private:
    DiscogsApi& m_api;
};

/// \brief Discogs scraper for artists and albums.
///
/// \details
///   This scraper can be used to load albums and artists from Discogs.  It uses
///   their official API described at https://www.discogs.com/developers/.
class Discogs final : public MusicScraper
{
    Q_OBJECT
public:
    static constexpr const char* ID = "Discogs";

public:
    explicit Discogs(QObject* parent = nullptr);
    ~Discogs() override = default;

public:
    /// \brief Information about the scraper.
    ELCH_NODISCARD virtual const ScraperMeta& meta() const override;

    void initialize() override {}
    ELCH_NODISCARD bool isInitialized() const override { return true; }

    ELCH_NODISCARD ArtistSearchJob* searchArtist(ArtistSearchJob::Config config) override
    {
        // TODO: Implement
        Q_UNUSED(config)
        MediaElch_Assert(false);
        return nullptr;
    }

    ELCH_NODISCARD AlbumSearchJob* searchAlbum(AlbumSearchJob::Config config) override
    {
        // TODO: Implement
        Q_UNUSED(config)
        MediaElch_Assert(false);
        return nullptr;
    }

    ELCH_NODISCARD ArtistScrapeJob* loadArtist(ArtistScrapeJob::Config config) override;
    ELCH_NODISCARD AlbumScrapeJob* loadAlbum(AlbumScrapeJob::Config config) override;

    // This scraper has no settings; it's all provided by the universal music scraper

    bool hasSettings() const override { return false; }
    void loadSettings(ScraperSettings& settings) override { Q_UNUSED(settings); }
    void saveSettings(ScraperSettings& settings) override { Q_UNUSED(settings); }
    QWidget* settingsWidget() override { return nullptr; }

private:
    ScraperMeta m_meta;
    DiscogsApi m_api;
};

} // namespace scraper
} // namespace mediaelch
