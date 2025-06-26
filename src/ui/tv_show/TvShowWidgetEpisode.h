#pragma once

#include "data/tv_show/TvShowEpisode.h"
#include "network/DownloadManager.h"

#include <QLabel>
#include <QPointer>
#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class TvShowWidgetEpisode;
}

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
    void onPlayEpisode();

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
    void onAllPosterDownloadFinished();
    void onLoadDone(QSet<EpisodeScraperInfo> details);
    void onRevertChanges();
    void onCaptureImage(ImageType type);

    void onImdbIdChanged(QString imdbid);
    void onTvdbIdChanged(QString tvdbid);
    void onTmdbIdChanged(QString tmdbId);
    void onTvmazeIdChanged(QString tvmazeId);
    void onNameChange(QString text);
    void onShowTitleChange(QString text);
    void onSeasonChange(int value);
    void onEpisodeChange(int value);
    void onDisplaySeasonChange(int value);
    void onDisplayEpisodeChange(int value);
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

    /// \brief Forces a reload of stream details
    void onReloadStreamDetails();

    /// \brief Fills the widget with streamdetails
    /// \param reloadedFromFile If true, re-set the duration (non-streamdetails property)
    void updateStreamDetails(bool reloadedFromFile = false);

    void onAddTag(QString tag);
    void onRemoveTag(QString tag);
    void onAddActor();
    void onRemoveActor();
    void onActorChanged();
    void onChangeActorImage();
    void onActorEdited(QTableWidgetItem* item);

    void onImdbIdOpen();
    void onTvMazeIdOpen();

private:
    Ui::TvShowWidgetEpisode* ui;
    QPointer<TvShowEpisode> m_episode;
    QLabel* m_savingWidget;
    QMovie* m_loadingMovie;
    DownloadManager* m_imageDownloadManager;
    QVector<QWidget*> m_streamDetailsWidgets;
    QVector<QVector<QLineEdit*>> m_streamDetailsAudio;
    QVector<QVector<QLineEdit*>> m_streamDetailsSubtitles;
};
