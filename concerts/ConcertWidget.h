#ifndef CONCERTWIDGET_H
#define CONCERTWIDGET_H

#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>
#include <QResizeEvent>
#include <QTableWidgetItem>
#include <QWidget>

#include "data/Concert.h"
#include "globals/DownloadManager.h"

namespace Ui {
class ConcertWidget;
}

/**
 * @brief The ConcertWidget class
 */
class ConcertWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConcertWidget(QWidget *parent = 0);
    ~ConcertWidget();

public slots:
    void clear();
    void setConcert(Concert *concert);
    void onStartScraperSearch();
    void onSaveInformation();
    void onSaveAll();
    void setEnabledTrue(Concert *concert = 0);
    void setDisabledTrue();

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void setActionSearchEnabled(bool, MainWidgets);
    void setActionSaveEnabled(bool, MainWidgets);

private slots:
    void downloadActorsFinished(Concert *concert);
    void infoLoadDone(Concert *concert);
    void loadDone(Concert *concert, QMap<int, QList<Poster> > posters);
    void chooseConcertPoster();
    void chooseConcertBackdrop();
    void chooseConcertLogo();
    void chooseConcertClearArt();
    void chooseConcertCdArt();
    void posterDownloadFinished(DownloadManagerElement elem);
    void concertNameChanged(QString text);
    void addGenre();
    void removeGenre();
    void onPreviewPoster();
    void onPreviewBackdrop();
    void onPreviewLogo();
    void onPreviewClearArt();
    void onPreviewCdArt();
    void onRevertChanges();
    void onArtPageOne();
    void onArtPageTwo();

    void onNameChange(QString text);
    void onTaglineChange(QString text);
    void onRatingChange(double value);
    void onReleasedChange(QDate date);
    void onRuntimeChange(int value);
    void onCertificationChange(QString text);
    void onTrailerChange(QString text);
    void onWatchedChange(int state);
    void onPlayCountChange(int value);
    void onLastWatchedChange(QDateTime dateTime);
    void onOverviewChange();

    void onGenreEdited(QTableWidgetItem *item);

private:
    Ui::ConcertWidget *ui;
    Concert *m_concert;
    DownloadManager *m_posterDownloadManager;
    QMovie *m_loadingMovie;
    QLabel *m_savingWidget;
    QImage m_currentPoster;
    QImage m_currentBackdrop;
    QImage m_currentLogo;
    QImage m_currentClearArt;
    QImage m_currentCdArt;
    void updateConcertInfo();
};

#endif // CONCERTWIDGET_H
