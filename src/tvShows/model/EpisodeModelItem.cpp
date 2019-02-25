#include "tvShows/model/EpisodeModelItem.h"

#include <QApplication>
#include <QStringList>

#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "globals/Globals.h"
#include "globals/Manager.h"
#include "tvShows/model/SeasonModelItem.h"

EpisodeModelItem::~EpisodeModelItem()
{
}

TvShowBaseModelItem* EpisodeModelItem::parent() const
{
    return &m_parentItem;
}

/// @brief Get the item's index in its parent item.
int EpisodeModelItem::indexInParent() const
{
    return m_parentItem.episodes().indexOf(const_cast<EpisodeModelItem*>(this));
}

TvShow* EpisodeModelItem::tvShow()
{
    if (m_tvShowEpisode == nullptr) {
        return nullptr;
    }
    return m_tvShowEpisode->tvShow();
}

const TvShow* EpisodeModelItem::tvShow() const
{
    if (m_tvShowEpisode == nullptr) {
        return nullptr;
    }
    return m_tvShowEpisode->tvShow();
}
TvShowType EpisodeModelItem::type() const
{
    return TvShowType::Episode;
}

void EpisodeModelItem::setTvShowEpisode(TvShowEpisode* episode)
{
    m_tvShowEpisode = episode;
}

TvShowEpisode* EpisodeModelItem::tvShowEpisode()
{
    return m_tvShowEpisode;
}

const TvShowEpisode* EpisodeModelItem::tvShowEpisode() const
{
    return m_tvShowEpisode;
}

QVariant EpisodeModelItem::data(int column) const
{
    if (m_tvShowEpisode == nullptr) {
        return QVariant{};
    }

    switch (column) {
    case 4: return m_tvShowEpisode->syncNeeded();
    case 3: return !m_tvShowEpisode->infoLoaded();
    case 2: return m_tvShowEpisode->hasChanged();
    default: return m_tvShowEpisode->completeEpisodeName();
    }
}
