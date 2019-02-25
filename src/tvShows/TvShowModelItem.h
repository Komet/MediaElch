#pragma once

#include "data/SeasonNumber.h"
#include "globals/Globals.h"

#include <QObject>
#include <QVariant>
#include <QVector>

class TvShow;
class TvShowEpisode;

class TvShowModelItem : public QObject
{
    Q_OBJECT

public:
    explicit TvShowModelItem(TvShowModelItem* parent = nullptr);
    ~TvShowModelItem();

    TvShowModelItem* child(int number) const;
    int columnCount() const;
    QVariant data(int column) const;
    TvShowModelItem* appendShow(TvShow* show);
    TvShowModelItem* appendEpisode(TvShowEpisode* episode);
    TvShowModelItem* appendSeason(SeasonNumber seasonNumber, QString season, TvShow* show);
    TvShowModelItem* parent() const;
    bool removeChildren(int position, int count);
    int indexInParent() const;

    const QList<TvShowModelItem*>& children() const;

    void setTvShow(TvShow* show);
    void setTvShowEpisode(TvShowEpisode* episode);
    void setSeason(QString season);
    void setSeasonNumber(SeasonNumber seasonNumber);

    TvShow* tvShow();
    const TvShow* tvShow() const;

    TvShowEpisode* tvShowEpisode();
    const TvShowEpisode* tvShowEpisode() const;

    QString season() const;
    SeasonNumber seasonNumber() const;
    TvShowType type() const;

signals:
    void sigIntChanged(TvShowModelItem*, TvShowModelItem*);
    void sigChanged(TvShowModelItem*, TvShowModelItem*, TvShowModelItem*);

private slots:
    void onTvShowEpisodeChanged(TvShowEpisode* episode);
    void onSeasonChanged(TvShowModelItem* seasonItem, TvShowModelItem* episodeItem);

private:
    QList<TvShowModelItem*> m_children;
    TvShowModelItem* m_parentItem = nullptr;
    TvShow* m_tvShow = nullptr;
    TvShowEpisode* m_tvShowEpisode = nullptr;
    QString m_season;
    SeasonNumber m_seasonNumber = SeasonNumber::NoSeason;
};
