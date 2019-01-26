#pragma once

#include "data/TvShow.h"

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
    explicit TvTunesDialog(QWidget *parent = nullptr);
    ~TvTunesDialog() override;
    static TvTunesDialog *instance(QWidget *parent = nullptr);
    void setTvShow(TvShow *show);

public slots:
    int exec() override;

private slots:
    void onSearch();
    void onShowResults(QList<ScraperSearchResult> results);
    void onStateChanged(QMediaPlayer::State newState);
    void onPlayPause();
    void onResultClicked(QTableWidgetItem *item);
    void startDownload();
    void cancelDownload();
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished();
    void downloadReadyRead();
    void onClose();
    void onUpdateTime(qint64 currentTime);
    void onNewTotalTime(qint64 totalTime);

private:
    Ui::TvTunesDialog *ui;
    TvShow *m_show;
    qint64 m_totalTime;
    QMediaPlayer *m_mediaPlayer;
    QNetworkAccessManager *m_qnam;
    QNetworkReply *m_downloadReply;
    QTime m_downloadTime;
    QFile m_output;
    QUrl m_themeUrl;
    bool m_downloadInProgress;
    bool m_fileDownloaded;
    void clear();
};
