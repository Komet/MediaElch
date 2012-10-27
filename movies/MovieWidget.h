#ifndef MOVIEWIDGET_H
#define MOVIEWIDGET_H

#include <QLabel>
#include <QMenu>
#include <QMutex>
#include <QPointer>
#include <QResizeEvent>
#include <QTableWidgetItem>
#include <QWidget>

#include "data/Movie.h"
#include "globals/DownloadManager.h"

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

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void actorDownloadStarted(QString, int);
    void actorDownloadProgress(int, int, int);
    void actorDownloadFinished(int);
    void setActionSearchEnabled(bool, MainWidgets);
    void setActionSaveEnabled(bool, MainWidgets);

private slots:
    void downloadActorsFinished(Movie *movie);
    void actorDownloadsLeft(int left, DownloadManagerElement elem);
    void infoLoadDone(Movie *movie);
    void loadDone(Movie *movie, QMap<int, QList<Poster> > posters);
    void chooseMoviePoster();
    void chooseMovieBackdrop();
    void chooseMovieLogo();
    void chooseMovieClearArt();
    void chooseMovieCdArt();
    void posterDownloadFinished(DownloadManagerElement elem);
    void movieNameChanged(QString text);
    void addGenre();
    void removeGenre();
    void addActor();
    void removeActor();
    void addStudio();
    void removeStudio();
    void addCountry();
    void removeCountry();
    void onPreviewPoster();
    void onPreviewBackdrop();
    void onPreviewLogo();
    void onPreviewClearArt();
    void onPreviewCdArt();
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
    void onReleasedChange(QDate date);
    void onRuntimeChange(int value);
    void onCertificationChange(QString text);
    void onTrailerChange(QString text);
    void onWatchedChange(int state);
    void onPlayCountChange(int value);
    void onLastWatchedChange(QDateTime dateTime);
    void onOverviewChange();

    void onActorEdited(QTableWidgetItem *item);
    void onCountryEdited(QTableWidgetItem *item);
    void onGenreEdited(QTableWidgetItem *item);
    void onStudioEdited(QTableWidgetItem *item);
    void onStreamDetailsEdited();
    void onReloadStreamDetails();
    void updateStreamDetails(bool reloadFromFile = false);

private:
    Ui::MovieWidget *ui;
    Movie *m_movie;
    DownloadManager *m_posterDownloadManager;
    QMovie *m_loadingMovie;
    QLabel *m_savingWidget;
    QImage m_currentPoster;
    QImage m_currentBackdrop;
    QImage m_currentLogo;
    QImage m_currentClearArt;
    QImage m_currentCdArt;
    QList<QWidget*> m_streamDetailsWidgets;
    QList< QList<QLineEdit*> > m_streamDetailsAudio;
    QList< QList<QLineEdit*> > m_streamDetailsSubtitles;
    void updateMovieInfo();
};

#endif // MOVIEWIDGET_H
