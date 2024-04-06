#pragma once

#include "data/MusicBrainzId.h"
#include "network/NetworkManager.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/music/AllMusic.h"
#include "scrapers/music/Discogs.h"
#include "scrapers/music/MusicBrainz.h"
#include "scrapers/music/MusicScraper.h"
#include "scrapers/music/TheAudioDb.h"

#include <QComboBox>
#include <QObject>
#include <QPointer>
#include <QWidget>

class LanguageCombo;


namespace mediaelch {
namespace scraper {

class UniversalMusicScraper;

class UniversalArtistSearchJob final : public ArtistSearchJob
{
    Q_OBJECT
public:
    // TODO: get rid of scraper parameter
    explicit UniversalArtistSearchJob(UniversalMusicScraper* scraper, Config config, QObject* parent = nullptr);

private:
    void doStart() override;

private:
    UniversalMusicScraper* m_scraper;
};


class UniversalAlbumSearchJob final : public AlbumSearchJob
{
    Q_OBJECT
public:
    // TODO: get rid of scraper parameter
    explicit UniversalAlbumSearchJob(UniversalMusicScraper* scraper, Config config, QObject* parent = nullptr);

private:
    void doStart() override;

private:
    UniversalMusicScraper* m_scraper;
};


class UniversalArtistScrapeJob final : public ArtistScrapeJob
{
    Q_OBJECT
public:
    // TODO: get rid of scraper parameter
    explicit UniversalArtistScrapeJob(UniversalMusicScraper* scraper, Config config, QObject* parent = nullptr);

private:
    void doStart() override;

private:
    struct DownloadElement
    {
        QString source;
        QString type;
        QUrl url;
        bool downloaded;
        QString contents;
    };

    void appendDownloadElement(QString source, QString type, QUrl url);
    void processDownloadElement(DownloadElement elem);
    void checkIfFinished();

private slots:
    void onArtistLoadFinished();

private:
    UniversalMusicScraper* m_scraper;
    QVector<DownloadElement> m_artistDownloads;
    bool m_discogsFinished{false}; // TODO: Remove
    Artist m_discogsArtist;        // TODO: Remove
};

class UniversalAlbumScrapeJob final : public AlbumScrapeJob
{
    Q_OBJECT
public:
    // TODO: get rid of scraper parameter
    explicit UniversalAlbumScrapeJob(UniversalMusicScraper* scraper, Config config, QObject* parent = nullptr);

private:
    void doStart() override;

private:
    struct DownloadElement
    {
        QString source;
        QString type;
        QUrl url;
        bool downloaded;
        QString contents;
    };

    void appendDownloadElement(QString source, QString type, QUrl url);
    void processDownloadElement(DownloadElement elem);
    void checkIfFinished();

private slots:
    void onAlbumLoadFinished();

private:
    UniversalMusicScraper* m_scraper;
    QVector<DownloadElement> m_albumDownloads;
    bool m_discogsFinished{false}; // TODO: Remove
    Album m_discogsAlbum;          // TODO: Remove
};

class UniversalMusicScraper final : public MusicScraper
{
    Q_OBJECT
public:
    static constexpr const char* ID = "UniversalMusicScraper";

public:
    explicit UniversalMusicScraper(QObject* parent = nullptr);
    ~UniversalMusicScraper() override;

    ELCH_NODISCARD const ScraperMeta& meta() const override;

    void initialize() override {};
    ELCH_NODISCARD bool isInitialized() const override { return true; };

    ELCH_NODISCARD ArtistSearchJob* searchArtist(ArtistSearchJob::Config config) override;
    ELCH_NODISCARD AlbumSearchJob* searchAlbum(AlbumSearchJob::Config config) override;

    ELCH_NODISCARD ArtistScrapeJob* loadArtist(ArtistScrapeJob::Config config) override;
    ELCH_NODISCARD AlbumScrapeJob* loadAlbum(AlbumScrapeJob::Config config) override;

    ELCH_NODISCARD bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QWidget* settingsWidget() override;

public:
    /// \todo Remove
    ELCH_NODISCARD static bool
    shouldLoad(MusicScraperInfo info, const QSet<MusicScraperInfo>& infos, const Album& album);
    /// \todo Remove
    ELCH_NODISCARD static bool
    shouldLoad(MusicScraperInfo info, const QSet<MusicScraperInfo>& infos, const Artist& artist);

    /// \todo Remove
    friend UniversalArtistSearchJob;
    /// \todo Remove
    friend UniversalArtistScrapeJob;
    /// \todo Remove
    friend UniversalAlbumSearchJob;
    /// \todo Remove
    friend UniversalAlbumScrapeJob;

private:
    ScraperMeta m_meta;
    QPointer<QWidget> m_widget;
    QString m_prefer;
    LanguageCombo* m_box{nullptr};
    QComboBox* m_preferBox{nullptr};

    mediaelch::scraper::MusicBrainzApi m_musicBrainzApi;
    mediaelch::scraper::MusicBrainz m_musicBrainz;
    mediaelch::scraper::TheAudioDbApi m_theAudioDbApi;
    mediaelch::scraper::TheAudioDb m_theAudioDb;
    mediaelch::scraper::AllMusicApi m_allMusicApi;
    mediaelch::scraper::AllMusic m_allMusic;
    mediaelch::scraper::Discogs m_discogs;

    QString m_tadbApiKey{"7490823590829082posuda"};

    mediaelch::network::NetworkManager m_network;
};

} // namespace scraper
} // namespace mediaelch
