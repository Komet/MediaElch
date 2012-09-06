#ifndef TVSHOWWIDGETTVSHOW_H
#define TVSHOWWIDGETTVSHOW_H

#include <QLabel>
#include <QMovie>
#include <QTableWidgetItem>
#include <QWidget>
#include "data/TvShow.h"
#include "DownloadManager.h"

namespace Ui {
class TvShowWidgetTvShow;
}

/**
 * @brief The TvShowWidgetTvShow class
 */
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
    void onLoadDone(TvShow *show);
    void onChoosePoster();
    void onChooseBackdrop();
    void onChooseBanner();
    void onChooseSeasonPoster(int season);
    void onPosterDownloadFinished(DownloadManagerElement elem);
    void onDownloadsFinished(TvShow *show);
    void onDownloadsLeft(int left, DownloadManagerElement elem);
    void onPreviewPoster();
    void onPreviewBackdrop();
    void onPreviewBanner();
    void onActorChanged();
    void onChangeActorImage();
    void onRevertChanges();

    void onNameChange(QString text);
    void onCertificationChange(QString text);
    void onRatingChange(double value);
    void onFirstAiredChange(QDate date);
    void onStudioChange(QString studio);
    void onOverviewChange();
    void onActorEdited(QTableWidgetItem *item);
    void onGenreEdited(QTableWidgetItem *item);

private:
    Ui::TvShowWidgetTvShow *ui;
    TvShow *m_show;
    QLabel *m_savingWidget;
    QMovie *m_loadingMovie;
    DownloadManager *m_posterDownloadManager;
    QMap<int, QList<QWidget*> > m_seasonLayoutWidgets;
    QImage m_currentPoster;
    QImage m_currentBackdrop;
    QImage m_currentBanner;

    void updateTvShowInfo();
};

#endif // TVSHOWWIDGETTVSHOW_H
