#ifndef TVSHOW_H
#define TVSHOW_H

#include <QMetaType>
#include <QObject>
#include <QStringList>
#include "data/TvShowEpisode.h"

class TvShowEpisode;

class TvShow : public QObject
{
    Q_OBJECT
public:
    explicit TvShow(QString dir = QString(), QObject *parent = 0);
    void addEpisode(TvShowEpisode *episode);
    QString name();
    int episodeCount();

private:
    QList<TvShowEpisode*> m_episodes;
    QString m_dir;
};

Q_DECLARE_METATYPE(TvShow*)

#endif // TVSHOW_H
