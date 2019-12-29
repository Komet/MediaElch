#pragma once

#include "globals/Globals.h"
#include "globals/ScraperResult.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QQueue>

class TvTunes : public QObject
{
    Q_OBJECT
public:
    explicit TvTunes(QObject* parent = nullptr);
    void search(QString searchStr);

signals:
    void sigSearchDone(QVector<ScraperSearchResult>);

private slots:
    void onSearchFinished();
    void onDownloadUrlFinished();

private:
    QNetworkAccessManager m_qnam;
    QVector<ScraperSearchResult> m_results;
    QQueue<ScraperSearchResult> m_queue;
    QString m_searchStr;
    QVector<ScraperSearchResult> parseSearch(QString html);
    void getNextDownloadUrl(QString searchStr = "");
};
