#pragma once

#include "globals/Globals.h"
#include "tv_shows/SeasonNumber.h"
#include "tv_shows/model/TvShowBaseModelItem.h"

#include <QObject>
#include <QVariant>
#include <QVector>

class TvShow;
class TvShowEpisode;
class SeasonModelItem;

class EpisodeModelItem final : public TvShowBaseModelItem
{
    Q_OBJECT

public:
    explicit EpisodeModelItem(SeasonModelItem& parent) : TvShowBaseModelItem(), m_parentItem{parent} {}
    ~EpisodeModelItem() override;

    QVariant data(int column) const override;

    TvShowBaseModelItem* child(int number) const override
    {
        Q_UNUSED(number);
        return nullptr; // no-op
    }
    int childCount() const override { return 0; }
    bool removeChildren(int position, int count) override
    {
        Q_UNUSED(position);
        Q_UNUSED(count);
        return false; // no-op
    }

    TvShowBaseModelItem* parent() const override;
    int indexInParent() const override;

    TvShow* tvShow() override;
    const TvShow* tvShow() const override;

    TvShowType type() const override;

    void setTvShowEpisode(TvShowEpisode* episode);
    TvShowEpisode* tvShowEpisode();
    const TvShowEpisode* tvShowEpisode() const;

private:
    SeasonModelItem& m_parentItem;
    TvShowEpisode* m_tvShowEpisode = nullptr;
};
