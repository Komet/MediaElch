#ifndef TRAILERDIALOG_H
#define TRAILERDIALOG_H

#include <QDialog>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTableWidgetItem>
#include "globals/Globals.h"
#include "data/Movie.h"

namespace Ui {
class TrailerDialog;
}

class TrailerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrailerDialog(QWidget *parent = 0);
    ~TrailerDialog();
    static TrailerDialog *instance(QWidget *parent = 0);

public slots:
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

    void clear();
};

#endif // TRAILERDIALOG_H
