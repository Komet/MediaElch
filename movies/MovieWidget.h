#ifndef MOVIEWIDGET_H
#define MOVIEWIDGET_H

#include <QCompleter>
#include <QLabel>
#include <QMenu>
#include <QMutex>
#include <QPointer>
#include <QResizeEvent>
#include <QTableWidgetItem>
#include <QWidget>

#include "globals/DownloadManager.h"
#include "movies/Movie.h"
#include "smallWidgets/ClosableImage.h"

namespace Ui {
class MovieWidget;
}

/**
 * @brief The MovieWidget class
 */
class MovieWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovieWidget(QWidget *parent = 0);
    ~MovieWidget();

public slots:
    void clear();
    void setMovie(Movie *movie);
    void startScraperSearch();
    void saveInformation();
    void saveAll();
    void setEnabledTrue(Movie *movie = 0);
    void setDisabledTrue();
    void setBigWindow(bool bigWindow);
    void updateMovieInfo();

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void actorDownloadStarted(QString, int);
    void actorDownloadProgress(int, int, int);
    void actorDownloadFinished(int);
    void setActionSearchEnabled(bool, MainWidgets);
    void setActionSaveEnabled(bool, MainWidgets);

private slots:
    void onInfoLoadDone(Movie *movie);
    void onLoadDone(Movie *movie);
    void onLoadImagesStarted(Movie *movie);
    void onLoadingImages(Movie *movie, QList<int> imageTypes);
    void onDownloadProgress(Movie *movie, int current, int maximum);
    void onSetImage(Movie *movie, int type, QByteArray data);
    void onImageDropped(int imageType, QUrl imageUrl);
    void onExtraFanartDropped(QUrl imageUrl);

    void onChooseImage();
    void onDeleteImage();

    void movieNameChanged(QString text);
    void addGenre(QString genre);
    void removeGenre(QString genre);
    void addTag(QString tag);
    void removeTag(QString tag);
    void addActor();
    void removeActor();
    void addStudio(QString studio);
    void removeStudio(QString studio);
    void addCountry(QString country);
    void removeCountry(QString country);
    void onActorChanged();
    void onChangeActorImage();
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
    void onRatingChange(double value);
    void onVotesChange(int value);
    void onTop250Change(int value);
    void onReleasedChange(QDate date);
    void onRuntimeChange(int value);
    void onCertificationChange(QString text);
    void onTrailerChange(QString text);
    void onWatchedClicked();
    void onPlayCountChange(int value);
    void onLastWatchedChange(QDateTime dateTime);
    void onOverviewChange();
    void onOutlineChange();
    void onImdbIdChange(QString text);

    void onActorEdited(QTableWidgetItem *item);
    void onSubtitleEdited(QTableWidgetItem *item);
    void onStreamDetailsEdited();
    void onReloadStreamDetails();
    void updateStreamDetails(bool reloadFromFile = false);
    void onDownloadTrailer();
    void onInsertYoutubeLink();
    void onPlayLocalTrailer();

    void onRemoveExtraFanart(const QString &file);
    void onRemoveExtraFanart(const QByteArray &image);
    void onAddExtraFanart();

    void updateImage(const int &imageType, ClosableImage *image);

private:
    Ui::MovieWidget *ui;
    QPointer<Movie> m_movie;
    QMovie *m_loadingMovie;
    QLabel *m_savingWidget;
    QList<QWidget*> m_streamDetailsWidgets;
    QList< QList<QLineEdit*> > m_streamDetailsAudio;
    QList< QList<QLineEdit*> > m_streamDetailsSubtitles;
    QLabel *m_backgroundLabel;
    void updateImages(QList<int> images);
};

#endif // MOVIEWIDGET_H
