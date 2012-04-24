#ifndef TVSHOWWIDGETTVSHOW_H
#define TVSHOWWIDGETTVSHOW_H

#include <QLabel>
#include <QMovie>
#include <QWidget>
#include "data/TvShow.h"
#include "DownloadManager.h"

namespace Ui {
class TvShowWidgetTvShow;
}

class TvShowWidgetTvShow : public QWidget
{
    Q_OBJECT
    
public:
    explicit TvShowWidgetTvShow(QWidget *parent = 0);
    ~TvShowWidgetTvShow();
    void setTvShow(TvShow *show);

public slots:
    void onSetEnabled(bool enabled);
    void onClear();
    void onSaveInformation();
    void onStartScraperSearch();

signals:
    void sigSetActionSearchEnabled(bool, MainWidgets);
    void sigSetActionSaveEnabled(bool, MainWidgets);
    void sigDownloadsStarted(QString, int);
    void sigDownloadsProgress(int, int, int);
    void sigDownloadsFinished(int);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void onAddGenre();
    void onRemoveGenre();
    void onAddActor();
    void onRemoveActor();
    void onLoadDone();
    void onChoosePoster();
    void onChooseBackdrop();
    void onChooseSeasonPoster(int season);
    void onPosterDownloadFinished(DownloadManagerElement elem);
    void onDownloadsFinished();
    void onDownloadsLeft(int left);

private:
    Ui::TvShowWidgetTvShow *ui;
    TvShow *m_show;
    QLabel *m_savingWidget;
    QMovie *m_loadingMovie;
    QImage m_chosenBackdrop;
    QImage m_chosenPoster;
    QMap<int, QImage> m_chosenSeasonPosters;
    DownloadManager *m_posterDownloadManager;
    QMap<int, QList<QWidget*> > m_seasonLayoutWidgets;
    bool m_loadedFromScraper;
    int m_progressMessageId;
    int m_currentDownloadsSize;

    void updateTvShowInfo();
};

#endif // TVSHOWWIDGETTVSHOW_H
