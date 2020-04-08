#pragma once

#include "globals/ScraperResult.h"
#include "tv_shows/TvShow.h"

#include <QDialog>
#include <QElapsedTimer>
#include <QFile>
#include <QMediaPlayer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTableWidgetItem>

namespace Ui {
class TvTunesDialog;
}

class TvTunes;

class TvTunesDialog : public QDialog
{
    Q_OBJECT

public:
    TvTunesDialog(TvShow& show, QWidget* parent = nullptr);
    ~TvTunesDialog() override;

public slots:
    int exec() override;

private slots:
    void onSearch();
    void onShowResults(QVector<ScraperSearchResult> results);
    void onStateChanged(QMediaPlayer::State newState);
    void onPlayPause();
    void onResultClicked(QTableWidgetItem* item);
    void startDownload();
    void cancelDownload();
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished();
    void downloadReadyRead();
    void onClose();
    void onUpdateTime(qint64 currentTime);
    void onNewTotalTime(qint64 totalTime);

private:
    Ui::TvTunesDialog* ui = nullptr;
    TvTunes* m_tvTunes;
    TvShow& m_show;
    qint64 m_totalTime = 0;
    QMediaPlayer* m_mediaPlayer = nullptr;
    QNetworkAccessManager* m_qnam = nullptr;
    QNetworkReply* m_downloadReply = nullptr;
    QElapsedTimer m_downloadTime;
    QFile m_output;
    QUrl m_themeUrl;
    bool m_downloadInProgress = false;
    bool m_fileDownloaded = false;
    void clear();
};
