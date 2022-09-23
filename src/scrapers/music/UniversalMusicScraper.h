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
#include <QMutex>
#include <QObject>
#include <QPointer>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class UniversalMusicScraper : public MusicScraper
{
    Q_OBJECT
public:
    explicit UniversalMusicScraper(QObject* parent = nullptr);
    ~UniversalMusicScraper() override;
    static constexpr const char* ID = "UniversalMusicScraper";

    QString name() const override;
    QString identifier() const override;
    QSet<MusicScraperInfo> scraperSupports() override;

    /// \brief Searches for an album using MusicBrainz
    /// \details If artist name is empty, only the albumSearchStr is used.
    ///          Otherwise the artist will be used for the result as well.
    void searchAlbum(QString artistName, QString albumSearchStr) override;
    void searchArtist(QString searchStr) override;

    void loadArtist(MusicBrainzId mbId, Artist* artist, QSet<MusicScraperInfo> infos) override;
    void loadAlbum(MusicBrainzId mbAlbumId,
        MusicBrainzId mbReleaseGroupId,
        Album* album,
        QSet<MusicScraperInfo> infos) override;

    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QWidget* settingsWidget() override;

public:
    /// \todo Remove
    static bool shouldLoad(MusicScraperInfo info, QSet<MusicScraperInfo> infos, Artist* artist);
    /// \todo Remove
    static bool shouldLoad(MusicScraperInfo info, QSet<MusicScraperInfo> infos, Album* album);

private slots:
    void onArtistLoadFinished();
    void onAlbumLoadFinished();

private:
    struct DownloadElement
    {
        QString source;
        QString type;
        QUrl url;
        bool downloaded;
        QString contents;
    };

    QString m_tadbApiKey;
    mediaelch::network::NetworkManager m_network;
    QString m_language;
    QString m_prefer;
    QPointer<QWidget> m_widget;
    QComboBox* m_box;
    QComboBox* m_preferBox;
    QMap<Artist*, QVector<DownloadElement>> m_artistDownloads;
    QMap<Album*, QVector<DownloadElement>> m_albumDownloads;
    QMutex m_artistMutex;
    QMutex m_albumMutex;

    mediaelch::scraper::MusicBrainzApi m_musicBrainzApi;
    mediaelch::scraper::MusicBrainz m_musicBrainz;
    mediaelch::scraper::TheAudioDbApi m_theAudioDbApi;
    mediaelch::scraper::TheAudioDb m_theAudioDb;
    mediaelch::scraper::AllMusicApi m_allMusicApi;
    mediaelch::scraper::AllMusic m_allMusic;
    mediaelch::scraper::Discogs m_discogs;

    mediaelch::network::NetworkManager* network();
    QString trim(QString text);

    bool infosLeft(QSet<MusicScraperInfo> infos, Artist* artist);
    bool infosLeft(QSet<MusicScraperInfo> infos, Album* album);
    void appendDownloadElement(Artist* artist, QString source, QString type, QUrl url);
    void appendDownloadElement(Album* album, QString source, QString type, QUrl url);
    void processDownloadElement(DownloadElement elem, Artist* artist, QSet<MusicScraperInfo> infos);
    void processDownloadElement(DownloadElement elem, Album* album, QSet<MusicScraperInfo> infos);
};

} // namespace scraper
} // namespace mediaelch
