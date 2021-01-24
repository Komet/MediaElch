#pragma once

#include "globals/Globals.h"
#include "tv_shows/SeasonNumber.h"
#include "tv_shows/model/TvShowBaseModelItem.h"

#include <QObject>

class TvShow;
class TvShowModelItem;
class SeasonModelItem;
class EpisodeModelItem;

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
