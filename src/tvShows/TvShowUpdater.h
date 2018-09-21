#ifndef TVSHOWUPDATER_H
#define TVSHOWUPDATER_H

#include <QNetworkAccessManager>
#include <QObject>

class TvShow;
class TheTvDb;

class TvShowUpdater : public QObject
{
    Q_OBJECT
public:
    explicit TvShowUpdater(QObject *parent = nullptr);
    static TvShowUpdater *instance(QObject *parent = nullptr);
    void updateShow(TvShow *show, bool force = false);

private slots:
    void onLoadFinished();

private:
    QNetworkAccessManager m_qnam;
    TheTvDb *m_tvdb;
    QList<TvShow *> m_updatedShows;
    QString unzipContent(QByteArray content);
};

#endif // TVSHOWUPDATER_H
