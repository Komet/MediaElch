#pragma once

#include "globals/ScraperResult.h"
#include "tv_shows/TvShow.h"

#include <QDialog>
#include <QFile>
#include <QMediaPlayer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTableWidgetItem>

namespace Ui {
class TvTunesDialog;
}

class TvTunesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TvTunesDialog(QWidget* parent = nullptr);
    ~TvTunesDialog() override;
    static TvTunesDialog* instance(QWidget* parent = nullptr);
    void setTvShow(TvShow* show);

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
    TvShow* m_show = nullptr;
    qint64 m_totalTime = 0;
    QMediaPlayer* m_mediaPlayer = nullptr;
    QNetworkAccessManager* m_qnam = nullptr;
    QNetworkReply* m_downloadReply = nullptr;
    QTime m_downloadTime;
    QFile m_output;
    QUrl m_themeUrl;
    bool m_downloadInProgress = false;
    bool m_fileDownloaded = false;
    void clear();
};
