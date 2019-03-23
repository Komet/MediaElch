#pragma once

#include "globals/DownloadManager.h"
#include "scrapers/tv_show/TvScraperInterface.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

#include <QDialog>
#include <QPointer>
#include <QQueue>

namespace Ui {
class TvShowMultiScrapeDialog;
}

class TvShowMultiScrapeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TvShowMultiScrapeDialog(QWidget* parent = nullptr);
    ~TvShowMultiScrapeDialog() override;

    static TvShowMultiScrapeDialog* instance(QWidget* parent = nullptr);

    QVector<TvShow*> shows() const;
    void setShows(const QVector<TvShow*>& shows);

    QVector<TvShowEpisode*> episodes() const;
    void setEpisodes(const QVector<TvShowEpisode*>& episodes);

public slots:
    int exec() override;
    void reject() override;
    void accept() override;

private slots:
    void onChkToggled();
    void onChkAllToggled();
    void onStartScraping();
    void onScrapingFinished();
    void onSearchFinished(QVector<ScraperSearchResult> results);
    void scrapeNext();
    void onInfoLoadDone(TvShow* show);
    void onEpisodeLoadDone();
    void onLoadDone(TvShow* show, QMap<ImageType, QVector<Poster>> posters);
    void onDownloadFinished(DownloadManagerElement elem);
    void onDownloadsFinished();
    void onChkDvdOrderToggled();

private:
    Ui::TvShowMultiScrapeDialog* ui;
    QVector<TvShow*> m_shows;
    QVector<TvShowEpisode*> m_episodes;
    bool m_executed;
    QVector<TvShowScraperInfos> m_infosToLoad;
    QQueue<TvShow*> m_showQueue;
    QQueue<TvShowEpisode*> m_episodeQueue;
    QPointer<TvShow> m_currentShow;
    QPointer<TvShowEpisode> m_currentEpisode;
    TvScraperInterface* m_scraperInterface;
    DownloadManager* m_downloadManager;
    QMap<QString, TvDbId> m_showIds;

    void setCheckBoxesEnabled();
    void addDownload(ImageType imageType, QUrl url, TvShow* show, SeasonNumber season = SeasonNumber::NoSeason);
    void addDownload(ImageType imageType, QUrl url, TvShow* show, Actor* actor);
    void addDownload(ImageType imageType, QUrl url, TvShowEpisode* episode);
};
