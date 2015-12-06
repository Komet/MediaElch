#ifndef TVSHOWMULTISCRAPEDIALOG_H
#define TVSHOWMULTISCRAPEDIALOG_H

#include <QDialog>
#include <QPointer>
#include <QQueue>
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "data/TvScraperInterface.h"
#include "globals/DownloadManager.h"

namespace Ui {
class TvShowMultiScrapeDialog;
}

class TvShowMultiScrapeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TvShowMultiScrapeDialog(QWidget *parent = 0);
    ~TvShowMultiScrapeDialog();

    static TvShowMultiScrapeDialog *instance(QWidget *parent = 0);

    QList<TvShow *> shows() const;
    void setShows(const QList<TvShow *> &shows);

    QList<TvShowEpisode *> episodes() const;
    void setEpisodes(const QList<TvShowEpisode *> &episodes);

public slots:
    int exec();
    void reject();
    void accept();

private slots:
    void onChkToggled();
    void onChkAllToggled();
    void onStartScraping();
    void onScrapingFinished();
    void onSearchFinished(QList<ScraperSearchResult> results);
    void scrapeNext();
    void onInfoLoadDone(TvShow *show);
    void onEpisodeLoadDone();
    void onLoadDone(TvShow *show, QMap<int, QList<Poster> > posters);
    void onDownloadFinished(DownloadManagerElement elem);
    void onDownloadsFinished();
    void onChkDvdOrderToggled();

private:
    Ui::TvShowMultiScrapeDialog *ui;
    QList<TvShow *> m_shows;
    QList<TvShowEpisode *> m_episodes;
    bool m_executed;
    QList<int> m_infosToLoad;
    QQueue<TvShow*> m_showQueue;
    QQueue<TvShowEpisode*> m_episodeQueue;
    QPointer<TvShow> m_currentShow;
    QPointer<TvShowEpisode> m_currentEpisode;
    TvScraperInterface *m_scraperInterface;
    DownloadManager *m_downloadManager;
    QMap<QString, QString> m_showIds;

    void setChkBoxesEnabled();
    void addDownload(int imageType, QUrl url, TvShow *show, int season = -1);
    void addDownload(int imageType, QUrl url, TvShow *show, Actor *actor);
    void addDownload(int imageType, QUrl url, TvShowEpisode *episode);
};

#endif // TVSHOWMULTISCRAPEDIALOG_H
