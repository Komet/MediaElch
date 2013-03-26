#ifndef TVTUNES_H
#define TVTUNES_H

#include <QObject>
#include <QNetworkAccessManager>
#include "globals/Globals.h"

class TvTunes : public QObject
{
    Q_OBJECT
public:
    explicit TvTunes(QObject *parent = 0);
    void search(QString searchStr);

signals:
    void sigSearchDone(QList<ScraperSearchResult>);

private slots:
    void onSearchFinished();

private:
    QNetworkAccessManager m_qnam;
    QList<ScraperSearchResult> parseSearch(QString html);
};

#endif // TVTUNES_H
