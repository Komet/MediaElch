#pragma once

#include "globals/DownloadManager.h"
#include "tv_shows/TvShowEpisode.h"

#include <QLabel>
#include <QPointer>
#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class TvShowWidgetEpisode;
}

/**
 * \brief The TvShowWidgetEpisode class
 */
class TvShowWidgetEpisode : public QWidget
{
    Q_OBJECT

public:
    explicit TvShowWidgetEpisode(QWidget* parent = nullptr);
    ~TvShowWidgetEpisode() override;
    void setEpisode(TvShowEpisode* episode);
    void updateEpisodeInfo();

public slots:
    void onSetEnabled(bool enabled);
    void onClear();
    void onSaveInformation();
    void onStartScraperSearch();

signals:
    void sigSetActionSearchEnabled(bool, MainWidgets);
    void sigSetActionSaveEnabled(bool, MainWidgets);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onAddDirector();
    void onRemoveDirector();
    void onAddWriter();
    void onRemoveWriter();
    void onChooseThumbnail();
    void onDeleteThumbnail();
    void onImageDropped(ImageType imageType, QUrl imageUrl);
    void onPosterDownloadFinished(DownloadManagerElement elem);
    void onLoadDone();
    void onRevertChanges();
    void onCaptureImage(ImageType type);

    void onImdbIdChanged(QString imdbid);
    void onTvdbIdChanged(QString tvdbid);
    void onTvmazeIdChanged(QString tvmazeId);
    void onNameChange(QString text);
    void onShowTitleChange(QString text);
    void onSeasonChange(int value);
    void onEpisodeChange(int value);
    void onDisplaySeasonChange(int value);
    void onDisplayEpisodeChange(int value);
    void onRatingChange(double value);
    void onVotesChange(int value);
    void onTop250Change(int value);
    void onCertificationChange(QString text);
    void onFirstAiredChange(QDate date);
    void onPlayCountChange(int value);
    void onLastPlayedChange(QDateTime dateTime);
    void onStudioChange(QString text);
    void onEpBookmarkChange(QTime time);
    void onOverviewChange();
    void onDirectorEdited(QTableWidgetItem* item);
    void onWriterEdited(QTableWidgetItem* item);
    void onStreamDetailsEdited();
    void onReloadStreamDetails();
    void updateStreamDetails(bool reloadFromFile = false);

    void onAddTag(QString tag);
    void onRemoveTag(QString tag);
    void onAddActor();
    void onRemoveActor();
    void onActorChanged();
    void onChangeActorImage();
    void onActorEdited(QTableWidgetItem* item);

private:
    Ui::TvShowWidgetEpisode* ui;
    QPointer<TvShowEpisode> m_episode;
    QLabel* m_savingWidget;
    QMovie* m_loadingMovie;
    DownloadManager* m_posterDownloadManager;
    QVector<QWidget*> m_streamDetailsWidgets;
    QVector<QVector<QLineEdit*>> m_streamDetailsAudio;
    QVector<QVector<QLineEdit*>> m_streamDetailsSubtitles;
};
