#ifndef TVSHOWWIDGETTVSHOW_H
#define TVSHOWWIDGETTVSHOW_H

#include <QLabel>
#include <QMovie>
#include <QPointer>
#include <QTableWidgetItem>
#include <QWidget>
#include "data/TvShow.h"
#include "globals/DownloadManager.h"

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
    void updateTvShowInfo();

public slots:
    void onSetEnabled(bool enabled);
    void onClear();
    void onSaveInformation();
    void onStartScraperSearch();
    void setBigWindow(bool bigWindow);

signals:
    void sigSetActionSearchEnabled(bool, MainWidgets);
    void sigSetActionSaveEnabled(bool, MainWidgets);
    void sigDownloadsStarted(QString, int);
    void sigDownloadsProgress(int, int, int);
    void sigDownloadsFinished(int);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void onAddGenre(QString genre);
    void onRemoveGenre(QString removeGenre);
    void onAddTag(QString tag);
    void onRemoveTag(QString tag);
    void onAddActor();
    void onRemoveActor();
    void onInfoLoadDone(TvShow *show);
    void onLoadDone(TvShow *show, QMap<int, QList<Poster> > posters);

    void onChooseImage();
    void onDeleteImage();
    void onImageDropped(int imageType, QUrl imageUrl);

    void onPosterDownloadFinished(DownloadManagerElement elem);
    void onDownloadsFinished(TvShow *show);
    void onDownloadsLeft(int left, DownloadManagerElement elem);
    void onActorChanged();
    void onChangeActorImage();
    void onRevertChanges();
    void onArtPageOne();
    void onArtPageTwo();
    void onDownloadTune();

    void onNameChange(QString text);
    void onImdbIdChange(QString text);
    void onTvdbIdChange(QString text);
    void onStatusChange(int index);
    void onSortTitleChange(QString text);
    void onCertificationChange(QString text);
    void onRatingChange(double value);
    void onVotesChange(int value);
    void onTop250Change(int value);
    void onFirstAiredChange(QDate date);
    void onStudioChange(QString studio);
    void onOverviewChange();
    void onActorEdited(QTableWidgetItem *item);
    void onRuntimeChange(int runtime);

    void onRemoveExtraFanart(const QString &file);
    void onRemoveExtraFanart(const QByteArray &image);
    void onAddExtraFanart();
    void onExtraFanartDropped(QUrl imageUrl);

    void onShowScraperProgress(TvShow *show, int current, int max);

private:
    Ui::TvShowWidgetTvShow *ui;
    QPointer<TvShow> m_show;
    QLabel *m_savingWidget;
    QMovie *m_loadingMovie;
    DownloadManager *m_posterDownloadManager;
    void updateImages(QList<int> images);
};

#endif // TVSHOWWIDGETTVSHOW_H
