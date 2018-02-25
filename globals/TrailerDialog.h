#ifndef TRAILERDIALOG_H
#define TRAILERDIALOG_H

#include <QDialog>
#include <QFile>
#include <QMediaPlayer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTableWidgetItem>
#include <QVideoWidget>

#include "globals/Globals.h"
#include "movies/Movie.h"

namespace Ui {
class TrailerDialog;
}

class TrailerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrailerDialog(QWidget *parent = nullptr);
    ~TrailerDialog();
    static TrailerDialog *instance(QWidget *parent = nullptr);

public slots:
    int exec();
    int exec(Movie *movie);
    void reject();

private slots:
    void search();
    void showResults(QList<ScraperSearchResult> results);
    void showTrailers(QList<TrailerResult> trailers);
    void resultClicked(QTableWidgetItem *item);
    void trailerClicked(QTableWidgetItem *item);
    void backToResults();
    void backToTrailers();
    void startDownload();
    void cancelDownload();
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished();
    void downloadReadyRead();
    void onNewTotalTime(qint64 totalTime);
    void onStateChanged(QMediaPlayer::State newState);
    void onPlayPause();
    void onAnimationFinished();
    void onUpdateTime(qint64 currentTime);
    void onSliderPositionChanged();

private:
    Ui::TrailerDialog *ui;
    int m_providerNo;
    QString m_providerId;
    Movie *m_currentMovie;
    QList<TrailerResult> m_currentTrailers;
    QNetworkAccessManager *m_qnam;
    QNetworkReply *m_downloadReply;
    QTime m_downloadTime;
    QFile m_output;
    bool m_downloadInProgress;
    QString m_trailerFileName;
    QVideoWidget *m_videoWidget;
    QMediaPlayer *m_mediaPlayer;
    qint64 m_totalTime;
    void clear();
};

#endif // TRAILERDIALOG_H
