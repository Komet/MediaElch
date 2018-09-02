#ifndef TVTUNES_H
#define TVTUNES_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QQueue>

#include "globals/Globals.h"

class TvTunes : public QObject
{
    Q_OBJECT
public:
    explicit TvTunes(QObject *parent = nullptr);
    void search(QString searchStr);

signals:
    void sigSearchDone(QList<ScraperSearchResult>);

private slots:
    void onSearchFinished();
    void onDownloadUrlFinished();

private:
    QNetworkAccessManager m_qnam;
    QList<ScraperSearchResult> m_results;
    QQueue<ScraperSearchResult> m_queue;
    QString m_searchStr;
    QList<ScraperSearchResult> parseSearch(QString html);
    void getNextDownloadUrl(QString searchStr = "");
};

#endif // TVTUNES_H
