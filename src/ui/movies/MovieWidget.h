#pragma once

#include "data/movie/Movie.h"

#include <QCompleter>
#include <QLabel>
#include <QMenu>
#include <QPointer>
#include <QResizeEvent>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVector>
#include <QWidget>

namespace Ui {
class MovieWidget;
}

class ClosableImage;

class MovieWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovieWidget(QWidget* parent = nullptr);
    ~MovieWidget() override;

public slots:
    void clear();
    void setMovie(Movie* movie);
    void startScraperSearch();
    void saveInformation();
    void saveAll();
    void setEnabledTrue(Movie* movie = nullptr);
    void setDisabledTrue();
    void setBigWindow(bool bigWindow);
    void updateMovieInfo();

protected:
    void resizeEvent(QResizeEvent* event) override;

signals:
    void actorDownloadStarted(QString, int);
    void actorDownloadProgress(int, int, int);
    void actorDownloadFinished(int);

    void setActionSearchEnabled(bool, MainWidgets);
    void setActionSaveEnabled(bool, MainWidgets);

private slots:
    void onInfoLoadDone(Movie* movie);
    void onLoadStarted(Movie* movie);
    void onLoadDone(Movie* movie);
    void onLoadImagesStarted(Movie* movie);
    void onLoadingImages(Movie* movie, QSet<ImageType> imageTypes);
    void onDownloadProgress(Movie* movie, int current, int maximum);
    void onSetImage(Movie* movie, ImageType type, QByteArray imageData);
    void onImageDropped(ImageType imageType, QUrl imageUrl);
    void onCaptureImage(ImageType type);
    void onExtraFanartsDropped(QVector<QUrl> imageUrls);

    void onChooseImage();
    void onDeleteImage();

    void movieNameChanged(QString text);

    void addGenre(QString genre);
    void removeGenre(QString genre);

    void addTag(QString tag);
    void removeTag(QString tag);

    void addStudio(QString studio);
    void removeStudio(QString studio);
    void addCountry(QString country);
    void removeCountry(QString country);
    void onRevertChanges();
    void onArtPageOne();
    void onArtPageTwo();

    void onNameChange(QString text);
    void onOriginalNameChange(QString text);
    void onSortTitleChange(QString text);
    void onSetChange(QString text);
    void onTaglineChange(QString text);
    void onWriterChange(QString text);
    void onDirectorChange(QString text);
    void onUserRatingChange(double value);
    void onTop250Change(int value);
    void onReleasedChange(QDate date);
    void onRuntimeChange(int value);
    void onCertificationChange(QString text);
    void onTrailerChange(QString text);
    void onTvShowLinksChange();
    void onWatchedClicked();
    void onPlayCountChange(int value);
    void onLastWatchedChange(QDateTime dateTime);
    void onOverviewChange();
    void onOutlineChange();
    void onImdbIdChange(QString text);
    void onTmdbIdChange(QString text);
    void onImdbIdOpen();
    void onTmdbIdOpen();

    void onSubtitleEdited(QTableWidgetItem* item);
    void onStreamDetailsEdited();
    /// \brief Forces a reload of stream details
    void onClickReloadStreamDetails();
    /// \brief Fills the widget with stream details
    /// \param reloadedFromFile If true, re-set the duration (non-stream details property)
    void updateStreamDetails(bool reloadedFromFile = false);
    void onDownloadTrailer();
    void onInsertYoutubeLink();
    void onPlayLocalTrailer();

    void onRemoveExtraFanart(QString file);
    void onRemoveExtraFanart(QByteArray image);
    void onAddExtraFanart();

private:
    void updateImage(ImageType imageType, ClosableImage* image);

private:
    Ui::MovieWidget* ui;
    QPointer<Movie> m_movie;
    QMovie* m_loadingMovie;
    QLabel* m_savingWidget;
    QVector<QWidget*> m_streamDetailsWidgets;
    QVector<QVector<QLineEdit*>> m_streamDetailsAudio;
    QVector<QVector<QLineEdit*>> m_streamDetailsSubtitles;
    QLabel* m_backgroundLabel;

    void updateImages(QSet<ImageType> images);
};
