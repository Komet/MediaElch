#pragma once

#include "data/tv_show/SeasonNumber.h"
#include "globals/Globals.h"
#include "model/tv_show/TvShowBaseModelItem.h"

#include <QList>
#include <QObject>
#include <QVariant>

class TvShow;
class TvShowEpisode;
class EpisodeModelItem;
class SeasonModelItem;
class TvShowRootModelItem;

/// A TV show item.
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
class TvShowModelItem : public TvShowBaseModelItem
{
    Q_OBJECT

public:
    enum class Columns : int
    {
        Title = 0,
        HasChanged = 2,
        HasNewEpisodeOrInfoNotLoaded = 3,
        SyncNeeded = 4,
        EpisodeCount = 1,
        TvShowBanner = 101,
        TvShowPoster = 102,
        TvShowExtraFanart = 103,
        TvShowBackdrop = 104,
        TvShowLogos = 105,
        TvShowThumb = 106,
        TvShowClearArt = 107,
        TvShowCharacterArt = 108,
        HasDummyEpisodes = 109,
        Filename = 110
    };

public:
    explicit TvShowModelItem(TvShowRootModelItem& parent) : m_parentItem{parent} {}
    ~TvShowModelItem() override;

    QVariant data(int column) const override;

    TvShowBaseModelItem* child(int number) const override;
    int childCount() const override;
    bool removeChildren(int position, int count) override;

    TvShowBaseModelItem* parent() const override;
    int indexInParent() const override;

    TvShow* tvShow() override;
    const TvShow* tvShow() const override;

    TvShowType type() const override;

    SeasonModelItem* seasonAtIndex(int number) const;
    SeasonModelItem* appendSeason(SeasonNumber seasonNumber, QString season, TvShow* show);
    const QList<SeasonModelItem*>& seasons() const;

    void setTvShow(TvShow* show);

signals:
    void sigChanged(TvShowModelItem*, SeasonModelItem*, EpisodeModelItem*);

private slots:
    void onEpisodeChanged(SeasonModelItem* seasonItem, EpisodeModelItem* episodeItem);

private:
    QList<SeasonModelItem*> m_children;

    TvShowRootModelItem& m_parentItem;
    TvShow* m_tvShow = nullptr;
};
