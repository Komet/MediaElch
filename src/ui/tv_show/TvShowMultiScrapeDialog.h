#pragma once

#include "globals/DownloadManager.h"
#include "scrapers/tv_show/TvScraper.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

#include <QDialog>
#include <QPointer>
#include <QQueue>

namespace Ui {
class TvShowMultiScrapeDialog;
}

/// \brief Dialog for scraping multiple episodes or TV shows.
/// \details Create a dialog which scrapes either the given shows (and episodes) or just the episodes.
///          exec() must only be called once. Create a fresh dialog after scraping shows.
class TvShowMultiScrapeDialog : public QDialog
{
    Q_OBJECT

public:
    TvShowMultiScrapeDialog(QVector<TvShow*> shows, QVector<TvShowEpisode*> episodes, QWidget* parent = nullptr);
    ~TvShowMultiScrapeDialog() override;

    QVector<TvShow*> shows() const;
    QVector<TvShowEpisode*> episodes() const;

public slots:
    int exec() override;
    void reject() override;
    void accept() override;

private slots:
    void onShowInfoToggled();
    void onEpisodeInfoToggled();
    void onChkAllShowInfosToggled();
    void onChkAllEpisodeInfosToggled();
    void onStartScraping();
    void onScrapingFinished();
    void onSearchFinished(mediaelch::scraper::ShowSearchJob* searchJob);
    void scrapeNext();
    void onInfoLoadDone(TvShow* show, QSet<ShowScraperInfo> details);
    void onEpisodeLoadDone();
    void onLoadDone(TvShow* show, QMap<ImageType, QVector<Poster>> posters);
    void onDownloadFinished(DownloadManagerElement elem);

    void onScraperChanged(int index);
    void onLanguageChanged();
    void onSeasonOrderChanged(int index);

private:
    Ui::TvShowMultiScrapeDialog* ui;
    QVector<TvShow*> m_shows;
    QVector<TvShowEpisode*> m_episodes;
    SeasonOrder m_seasonOrder = SeasonOrder::Aired;
    QSet<ShowScraperInfo> m_showDetailsToLoad;
    QSet<EpisodeScraperInfo> m_episodeDetailsToLoad;
    QQueue<TvShow*> m_showQueue;
    QQueue<TvShowEpisode*> m_episodeQueue;
    QPointer<TvShow> m_currentShow = nullptr;
    QPointer<TvShowEpisode> m_currentEpisode = nullptr;
    mediaelch::scraper::TvScraper* m_currentScraper = nullptr;
    mediaelch::Locale m_locale = mediaelch::Locale::English;
    DownloadManager* m_downloadManager;
    QMap<QString, mediaelch::scraper::ShowIdentifier> m_showIds;

private:
    void setupLanguageDropdown();
    void setupScraperDropdown();
    void setupSeasonOrderComboBox();
    void updateCheckBoxes();
    void saveCurrentItem();

    TvShowUpdateType updateType() const;

    void logToUser(const QString& msg);

    void addDownload(ImageType imageType, QUrl url, TvShow* show, SeasonNumber season = SeasonNumber::NoSeason);
    void addDownload(ImageType imageType, QUrl url, TvShow* show, Actor* actor);
    void addDownload(ImageType imageType, QUrl url, TvShowEpisode* episode);

    void showError(const QString& message);
};
