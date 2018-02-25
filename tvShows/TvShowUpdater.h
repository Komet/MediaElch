#ifndef TVSHOWUPDATER_H
#define TVSHOWUPDATER_H

#include <QNetworkAccessManager>
#include <QObject>

#include "data/TvShow.h"
#include "scrapers/TheTvDb.h"

class TvShowUpdater : public QObject
{
    Q_OBJECT
public:
    explicit TvShowUpdater(QObject *parent = 0);
    static TvShowUpdater *instance(QObject *parent = 0);
    void updateShow(TvShow *show, bool force = false);

private slots:
    void onLoadFinished();

private:
    QNetworkAccessManager m_qnam;
    TheTvDb *m_tvdb;
    QList<TvShow*> m_updatedShows;
    QString unzipContent(QByteArray content);
};

#endif // TVSHOWUPDATER_H
