#pragma once

#include "globals/Globals.h"
#include "model/tv_show/TvShowBaseModelItem.h"

#include <QObject>
#include <QVariant>
#include <QVector>

class TvShow;
class TvShowEpisode;
class SeasonModelItem;

/// An episode item.
///
/// Note that this class is not actually a model according
/// to the QT model/view concept. They are solely data containers and do not
/// provide any functionality for signaling the addition or removal of seasons,
/// episodes, etc.
///
/// If you intend to add seasons or episodes, go through `TvShowModel`.
/// Special care needs to be taken when adding or removing rows from the model.
/// That is, `beginInsertRows` and similar need to be called whenever rows are
/// added or removed! None of the item classes in this directory so that.
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
