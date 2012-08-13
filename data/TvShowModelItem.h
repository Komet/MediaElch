#ifndef TVSHOWMODELITEM_H
#define TVSHOWMODELITEM_H

#include <QList>
#include <QObject>
#include <QVariant>
#include <QVector>
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"

class TvShow;

class TvShowModelItem : public QObject
{
    Q_OBJECT
public:
    explicit TvShowModelItem(TvShowModelItem *parent = 0);
    ~TvShowModelItem();

    TvShowModelItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    TvShowModelItem *appendChild(TvShow *show);
    TvShowModelItem *appendChild(TvShowEpisode *episode);
    TvShowModelItem *appendChild(QString season);
    TvShowModelItem *parent();
    bool removeChildren(int position, int count);
    int childNumber() const;
    void setTvShow(TvShow *show);
    void setTvShowEpisode(TvShowEpisode *episode);
    void setSeason(QString season);
    TvShow *tvShow();
    TvShowEpisode *tvShowEpisode();
    QString season();
    int type();

signals:
    void sigIntChanged(TvShowModelItem*, TvShowModelItem*);
    void sigChanged(TvShowModelItem*, TvShowModelItem*, TvShowModelItem*);

private slots:
    void onTvShowEpisodeChanged(TvShowEpisode *episode);
    void onSeasonChanged(TvShowModelItem* seasonItem, TvShowModelItem* episodeItem);

private:
    QList<TvShowModelItem*> m_childItems;
    TvShowModelItem *m_parentItem;
    TvShow *m_tvShow;
    TvShowEpisode *m_tvShowEpisode;
    QString m_season;
};

#endif // TVSHOWMODELITEM_H
