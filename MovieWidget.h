#ifndef MOVIEWIDGET_H
#define MOVIEWIDGET_H

#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>
#include <QResizeEvent>
#include <QWidget>

#include "data/Movie.h"
#include "DownloadManager.h"

namespace Ui {
class MovieWidget;
}

class MovieWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit MovieWidget(QWidget *parent = 0);
    ~MovieWidget();

public slots:
    void clear();
    void setMovie(Movie *movie);
    void setEnabledTrue();
    void setDisabledTrue();
    void showFirstTime();
    void hideFirstTime();
    void startScraperSearch();
    void saveInformation();

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void actorDownloadStarted(QString);
    void actorDownloadProgress(int, int);
    void actorDownloadFinished();
    void movieChangeCanceled();
    void setActionSearchEnabled(bool);
    void setActionSaveEnabled(bool);

private slots:
    void downloadActorsFinished();
    void actorDownloadsLeft(int left);
    void loadDone();
    void chooseMoviePoster();
    void chooseMovieBackdrop();
    void posterDownloadFinished(DownloadManagerElement elem);
    void posterDownloadProgress(DownloadManagerElement elem);
    void movieNameChanged(QString text);
    void addGenre();
    void removeGenre();
    void addActor();
    void removeActor();
    void addStudio();
    void removeStudio();
    void addCountry();
    void removeCountry();
    void groupBoxResized(QSize size);
    void markHasChanged();

private:
    Ui::MovieWidget *ui;
    Movie *m_movie;
    DownloadManager *m_posterDownloadManager;
    QMovie *m_loadingMovie;
    QImage m_chosenPoster;
    QImage m_chosenBackdrop;
    QLabel *m_savingWidget;
    QLabel *m_firstTimeLabel;
    bool m_loadedFromScraper;
    bool m_hasChanged;
    void updateMovieInfo();
};

#endif // MOVIEWIDGET_H
