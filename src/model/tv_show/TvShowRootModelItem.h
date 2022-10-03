#pragma once

#include "globals/Globals.h"
#include "model/tv_show/TvShowBaseModelItem.h"

#include <QObject>

class TvShow;
class TvShowModelItem;
class SeasonModelItem;
class EpisodeModelItem;

/// A root item.
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
class TvShowRootModelItem final : public TvShowBaseModelItem
{
    Q_OBJECT

public:
    explicit TvShowRootModelItem() = default;
    ~TvShowRootModelItem() override;

    QVariant data(int column) const override;

    TvShowBaseModelItem* child(int number) const override;
    int childCount() const override;
    bool removeChildren(int position, int count) override;

    TvShowBaseModelItem* parent() const override;
    int indexInParent() const override;

    TvShow* tvShow() override;
    const TvShow* tvShow() const override;

    TvShowType type() const override;

    TvShowModelItem* appendShow(TvShow* show);
    const QList<TvShowModelItem*>& shows() const;
    TvShowModelItem* showAtIndex(int number) const;

signals:
    void sigChanged(TvShowModelItem*, SeasonModelItem*, EpisodeModelItem*);

private slots:
    void onSigChanged(TvShowModelItem* show, SeasonModelItem* season, EpisodeModelItem* episode);

private:
    QList<TvShowModelItem*> m_children;
};
