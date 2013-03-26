#ifndef TVTUNESDIALOG_H
#define TVTUNESDIALOG_H

#include <QDialog>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTableWidgetItem>
#include "phonon/MediaObject"
#include "phonon/VideoWidget"

#include "data/TvShow.h"

namespace Ui {
class TvTunesDialog;
}

class TvTunesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TvTunesDialog(QWidget *parent = 0);
    ~TvTunesDialog();
    static TvTunesDialog* instance(QWidget *parent = 0);
    void setTvShow(TvShow *show);

public slots:
    int exec();

private slots:
    void onSearch();
    void onShowResults(QList<ScraperSearchResult> results);
    void onTick(qint64 time);
    void onNewTotalTime(qint64 totalTime);
    void onStateChanged(Phonon::State newState);
    void onPlayPause();
    void onResultClicked(QTableWidgetItem *item);
    void startDownload();
    void cancelDownload();
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished();
    void downloadReadyRead();
    void onClose();

private:
    Ui::TvTunesDialog *ui;
    TvShow *m_show;
    Phonon::MediaObject *m_mediaObject;
    Phonon::VideoWidget *m_videoWidget;
    qint64 m_totalTime;
    QNetworkAccessManager *m_qnam;
    QNetworkReply *m_downloadReply;
    QTime m_downloadTime;
    QFile m_output;
    QUrl m_themeUrl;
    bool m_downloadInProgress;
    bool m_fileDownloaded;
    void updateTime(qint64 currentTime);
    void clear();
};

#endif // TVTUNESDIALOG_H
