#pragma once

#include "data/movie/Movie.h"
#include "network/NetworkManager.h"
#include "scrapers/ScraperResult.h"
#include "scrapers/trailer/TrailerResult.h"
#include "utils/Meta.h"

#include <QDialog>
#include <QElapsedTimer>
#include <QFile>
#include <QMediaPlayer>
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

public slots:
    int exec() override;
    int exec(Movie* movie);
    void reject() override;

private slots:
    void search();
    void searchIndex(int comboIndex);

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
    void onStateChanged(ELCH_MEDIA_PLAYBACK_STATE newState);
    void onPlayPause();
    void onAnimationFinished();
    void onUpdateTime(qint64 currentTime);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void onTrailerError(QMediaPlayer::Error error);
#else
    void onTrailerError(QMediaPlayer::Error error, const QString& errorString);
#endif

    void onSliderPositionChanged();

private:
    Ui::TrailerDialog* ui = nullptr;
    int m_providerNo = 0;
    QString m_providerId;
    Movie* m_currentMovie = nullptr;
    QVector<TrailerResult> m_currentTrailers;
    mediaelch::network::NetworkManager* m_network = nullptr;
    QNetworkReply* m_downloadReply = nullptr;
    QElapsedTimer m_downloadTime;
    QFile m_output;
    bool m_downloadInProgress = false;
    QString m_trailerFileName;
    QVideoWidget* m_videoWidget = nullptr;
    QMediaPlayer* m_mediaPlayer = nullptr;
    qint64 m_totalTime = 0;

    void clear();
};
