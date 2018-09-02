#ifndef TVSHOWMODELITEM_H
#define TVSHOWMODELITEM_H

#include <QList>
#include <QObject>
#include <QVariant>
#include <QVector>

#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "globals/Globals.h"

class TvShow;
class TvShowEpisode;

/**
 * @brief The TvShowModelItem class
 */
class TvShowModelItem : public QObject
{
    Q_OBJECT
public:
    explicit TvShowModelItem(TvShowModelItem *parent = nullptr);
    ~TvShowModelItem();

    TvShowModelItem *child(int number) const;
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    TvShowModelItem *appendChild(TvShow *show);
    TvShowModelItem *appendChild(TvShowEpisode *episode);
    TvShowModelItem *appendChild(int seasonNumber, QString season, TvShow *show);
    TvShowModelItem *parent() const;
    bool removeChildren(int position, int count);
    int childNumber() const;
    void setTvShow(TvShow *show);
    void setTvShowEpisode(TvShowEpisode *episode);
    void setSeason(QString season);
    void setSeasonNumber(int seasonNumber);
    TvShow *tvShow();
    TvShowEpisode *tvShowEpisode();
    QString season();
    int seasonNumber();
    TvShowType type();

signals:
    void sigIntChanged(TvShowModelItem *, TvShowModelItem *);
    void sigChanged(TvShowModelItem *, TvShowModelItem *, TvShowModelItem *);

private slots:
    void onTvShowEpisodeChanged(TvShowEpisode *episode);
    void onSeasonChanged(TvShowModelItem *seasonItem, TvShowModelItem *episodeItem);

private:
    QList<TvShowModelItem *> m_childItems;
    TvShowModelItem *m_parentItem;
    TvShow *m_tvShow;
    TvShowEpisode *m_tvShowEpisode;
    QString m_season;
    int m_seasonNumber;
};

#endif // TVSHOWMODELITEM_H
