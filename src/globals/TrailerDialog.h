#pragma once

#include "globals/Globals.h"
#include "globals/ScraperResult.h"
#include "movies/Movie.h"

#include <QDialog>
#include <QFile>
#include <QMediaPlayer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTableWidgetItem>
#include <QVideoWidget>

namespace Ui {
class TrailerDialog;
}

class TrailerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrailerDialog(QWidget* parent = nullptr);
    ~TrailerDialog() override;
    static TrailerDialog* instance(QWidget* parent = nullptr);

public slots:
    int exec() override;
    int exec(Movie* movie);
    void reject() override;

private slots:
    void search();
    void showResults(QVector<ScraperSearchResult> results);
    void showTrailers(QVector<TrailerResult> trailers);
    void resultClicked(QTableWidgetItem* item);
    void trailerClicked(QTableWidgetItem* item);
    void backToResults();
    void backToTrailers();
    void startDownload();
    void cancelDownload();
    void downloadProgress(int received, int total);
    void downloadFinished();
    void downloadReadyRead();
    void onNewTotalTime(qint64 totalTime);
    void onStateChanged(QMediaPlayer::State newState);
    void onPlayPause();
    void onAnimationFinished();
    void onUpdateTime(qint64 currentTime);
    void onTrailerError(QMediaPlayer::Error error);
    void onSliderPositionChanged();

private:
    Ui::TrailerDialog* ui = nullptr;
    int m_providerNo = 0;
    QString m_providerId;
    Movie* m_currentMovie = nullptr;
    QVector<TrailerResult> m_currentTrailers;
    QNetworkAccessManager* m_qnam;
    QNetworkReply* m_downloadReply = nullptr;
    QTime m_downloadTime;
    QFile m_output;
    bool m_downloadInProgress = false;
    QString m_trailerFileName;
    QVideoWidget* m_videoWidget = nullptr;
    QMediaPlayer* m_mediaPlayer = nullptr;
    qint64 m_totalTime = 0;

    void clear();
};
