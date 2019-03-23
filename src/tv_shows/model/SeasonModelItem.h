#pragma once

#include "globals/Globals.h"
#include "tv_shows/SeasonNumber.h"
#include "tv_shows/model/EpisodeModelItem.h"
#include "tv_shows/model/TvShowBaseModelItem.h"

#include <QObject>
#include <QVariant>
#include <QVector>

class TvShow;
class TvShowEpisode;
class TvShowModelItem;

class SeasonModelItem final : public TvShowBaseModelItem
{
    Q_OBJECT

public:
    explicit SeasonModelItem(TvShowModelItem& parent) : m_parentItem{parent} {}
    ~SeasonModelItem() override;

    QVariant data(int column) const override;

    TvShowBaseModelItem* child(int number) const override;
    int childCount() const override;
    bool removeChildren(int position, int count) override;

    TvShowBaseModelItem* parent() const override;
    int indexInParent() const override;

    TvShow* tvShow() override;
    const TvShow* tvShow() const override;

    TvShowType type() const override;

    EpisodeModelItem* appendEpisode(TvShowEpisode* episode);
    EpisodeModelItem* episodeAtIndex(int number) const;
    const QList<EpisodeModelItem*>& episodes() const;

    void setTvShow(TvShow* show);
    void setSeason(QString season);
    void setSeasonNumber(SeasonNumber seasonNumber);

    QString season() const;
    SeasonNumber seasonNumber() const;

signals:
    void sigEpisodeChanged(SeasonModelItem*, EpisodeModelItem*);

private slots:
    void onTvShowEpisodeChanged(TvShowEpisode* episode);

private:
    QList<EpisodeModelItem*> m_children;
    TvShowModelItem& m_parentItem;
    TvShow* m_tvShow = nullptr;
    QString m_season;
    SeasonNumber m_seasonNumber = SeasonNumber::NoSeason;
};
