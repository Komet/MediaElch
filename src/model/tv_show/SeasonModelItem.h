#pragma once

#include "data/tv_show/SeasonNumber.h"
#include "globals/Globals.h"
#include "model/tv_show/EpisodeModelItem.h"
#include "model/tv_show/TvShowBaseModelItem.h"

#include <QObject>
#include <QVariant>
#include <QVector>

class TvShow;
class TvShowEpisode;
class TvShowModelItem;

/// A season item.
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
