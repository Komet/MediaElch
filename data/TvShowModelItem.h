#ifndef TVSHOWMODELITEM_H
#define TVSHOWMODELITEM_H

#include <QList>
#include <QVariant>
#include <QVector>
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"

class TvShowModelItem
{
public:
    TvShowModelItem(TvShowModelItem *parent = 0);
    ~TvShowModelItem();

    TvShowModelItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    TvShowModelItem *appendChild(TvShow *show);
    TvShowModelItem *appendChild(TvShowEpisode *episode);
    TvShowModelItem *parent();
    bool removeChildren(int position, int count);
    int childNumber() const;
    void setTvShow(TvShow *show);
    void setTvShowEpisode(TvShowEpisode *episode);
    TvShow *tvShow();
    TvShowEpisode *tvShowEpisode();
    int type();

private:
    QList<TvShowModelItem*> m_childItems;
    TvShowModelItem *m_parentItem;
    TvShow *m_tvShow;
    TvShowEpisode *m_tvShowEpisode;
};

#endif // TVSHOWMODELITEM_H
