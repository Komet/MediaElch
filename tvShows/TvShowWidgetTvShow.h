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
    void onChoosePoster();
    void onChooseBackdrop();
    void onChooseBanner();
    void onChooseLogo();
    void onChooseClearArt();
    void onChooseCharacterArt();
    void onPosterDownloadFinished(DownloadManagerElement elem);
    void onDownloadsFinished(TvShow *show);
    void onDownloadsLeft(int left, DownloadManagerElement elem);
    void onDeletePoster();
    void onDeleteBackdrop();
    void onDeleteBanner();
    void onDeleteLogo();
    void onDeleteClearArt();
    void onDeleteCharacterArt();
    void onActorChanged();
    void onChangeActorImage();
    void onRevertChanges();
    void onArtPageOne();
    void onArtPageTwo();

    void onNameChange(QString text);
    void onCertificationChange(QString text);
    void onRatingChange(double value);
    void onFirstAiredChange(QDate date);
    void onStudioChange(QString studio);
    void onOverviewChange();
    void onActorEdited(QTableWidgetItem *item);

    void onRemoveExtraFanart(const QString &file);
    void onRemoveExtraFanart(const QByteArray &image);
    void onAddExtraFanart();

private:
    Ui::TvShowWidgetTvShow *ui;
    QPointer<TvShow> m_show;
    QLabel *m_savingWidget;
    QMovie *m_loadingMovie;
    DownloadManager *m_posterDownloadManager;
    void updateTvShowInfo();
    void updateImages(QList<ImageType> images);
};

#endif // TVSHOWWIDGETTVSHOW_H
