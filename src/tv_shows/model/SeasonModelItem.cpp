#include "tv_shows/model/SeasonModelItem.h"

#include <QApplication>
#include <QStringList>

#include "globals/Globals.h"
#include "globals/Manager.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"
#include "tv_shows/model/EpisodeModelItem.h"
#include "tv_shows/model/TvShowModelItem.h"

SeasonModelItem::~SeasonModelItem()
{
    qDeleteAll(m_children);
}

TvShowBaseModelItem* SeasonModelItem::child(int number) const
{
    return episodeAtIndex(number);
}

int SeasonModelItem::childCount() const
{
    return m_children.size();
}

/// @brief Returns a child or nullptr
/// @param number Child index
EpisodeModelItem* SeasonModelItem::episodeAtIndex(int number) const
{
    if (number < 0 || number >= m_children.size()) {
        return nullptr;
    }
    return m_children.at(number);
}

const QList<EpisodeModelItem*>& SeasonModelItem::episodes() const
{
    return m_children;
}

/// @brief Get the item's index in its parent item.
int SeasonModelItem::indexInParent() const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return m_parentItem.seasons().indexOf(const_cast<SeasonModelItem*>(this));
}

/// @brief Appends an episode to this model.
/// @return Constructed child item
EpisodeModelItem* SeasonModelItem::appendEpisode(TvShowEpisode* episode)
{
    auto* item = new EpisodeModelItem(*this);
    item->setTvShowEpisode(episode);
    episode->setModelItem(item);
    m_children.append(item);
    connect(episode, &TvShowEpisode::sigChanged, this, &SeasonModelItem::onTvShowEpisodeChanged, Qt::UniqueConnection);
    return item;
}


TvShowBaseModelItem* SeasonModelItem::parent() const
{
    return &m_parentItem;
}

bool SeasonModelItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_children.size()) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        m_children.takeAt(position)->deleteLater();
    }

    return true;
}

void SeasonModelItem::setTvShow(TvShow* show)
{
    m_tvShow = show;
}

void SeasonModelItem::setSeason(QString season)
{
    m_season = season;
}

void SeasonModelItem::setSeasonNumber(SeasonNumber seasonNumber)
{
    m_seasonNumber = seasonNumber;
}

TvShow* SeasonModelItem::tvShow()
{
    return m_tvShow;
}

const TvShow* SeasonModelItem::tvShow() const
{
    return m_tvShow;
}

QString SeasonModelItem::season() const
{
    return m_season;
}

SeasonNumber SeasonModelItem::seasonNumber() const
{
    return m_seasonNumber;
}

TvShowType SeasonModelItem::type() const
{
    return TvShowType::Season;
}

void SeasonModelItem::onTvShowEpisodeChanged(TvShowEpisode* episode)
{
    emit sigEpisodeChanged(this, episode->modelItem());
}

QVariant SeasonModelItem::data(int column) const
{
    // todo: magic numbers (columns)
    if (m_tvShow == nullptr || m_seasonNumber == SeasonNumber::NoSeason) {
        return QVariant{};
    }

    switch (column) {
    case 1: return m_tvShow->episodeCount();
    case 2: return m_tvShow->hasChanged();
    case 3: {
        bool conversionOk = false;
        SeasonNumber season = SeasonNumber(m_season.toInt(&conversionOk));
        return conversionOk && m_tvShow->hasNewEpisodesInSeason(season);
    }
    case 4: return m_tvShow->syncNeeded();
    default: return tr("Season %1").arg(m_season);
    }
}
