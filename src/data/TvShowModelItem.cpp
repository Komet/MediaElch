#include "TvShowModelItem.h"

#include <QApplication>
#include <QStringList>

#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "globals/Globals.h"
#include "globals/Manager.h"

TvShowModelItem::TvShowModelItem(TvShowModelItem* parent) : QObject(nullptr), m_parentItem{parent}
{
}

/// @brief Deletes all child items
TvShowModelItem::~TvShowModelItem()
{
    qDeleteAll(m_children);
}

/// @brief Returns a child or nullptr
/// @param number Child index
TvShowModelItem* TvShowModelItem::child(int number) const
{
    if (number < 0 || number >= m_children.size()) {
        return nullptr;
    }
    return m_children.at(number);
}

const QList<TvShowModelItem*>& TvShowModelItem::children() const
{
    return m_children;
}

/// @brief Get the item's index in its parent item.
int TvShowModelItem::indexInParent() const
{
    if (m_parentItem) {
        return m_parentItem->m_children.indexOf(const_cast<TvShowModelItem*>(this));
    }

    return 0;
}

int TvShowModelItem::columnCount() const
{
    return 1;
}

QVariant TvShowModelItem::data(int column) const
{
    // todo: magic numbers (columns)
    if (m_tvShow) {
        switch (column) {
        case 1: return m_tvShow->episodeCount();
        case 101: return m_tvShow->hasImage(ImageType::TvShowBanner);
        case 102: return m_tvShow->hasImage(ImageType::TvShowPoster);
        case 103: return m_tvShow->hasImage(ImageType::TvShowExtraFanart);
        case 104: return m_tvShow->hasImage(ImageType::TvShowBackdrop);
        case 105: return m_tvShow->hasImage(ImageType::TvShowLogos);
        case 106: return m_tvShow->hasImage(ImageType::TvShowThumb);
        case 107: return m_tvShow->hasImage(ImageType::TvShowClearArt);
        case 108: return m_tvShow->hasImage(ImageType::TvShowCharacterArt);
        case 109: return m_tvShow->hasDummyEpisodes();
        };
    }

    switch (column) {
    case 110:
        if (m_tvShow
            && !Manager::instance()
                    ->mediaCenterInterface()
                    ->imageFileName(m_tvShow, ImageType::TvShowLogos)
                    .isEmpty()) {
            return Manager::instance()->mediaCenterInterface()->imageFileName(m_tvShow, ImageType::TvShowLogos);
        }
        break;
    case 4:
        if (m_tvShow) {
            return m_tvShow->syncNeeded();
        } else if (m_tvShowEpisode) {
            return m_tvShowEpisode->syncNeeded();
        }
        break;

    case 3:
        if (!m_season.isEmpty() && m_tvShow) {
            bool conversionOk = false;
            SeasonNumber season = SeasonNumber(m_season.toInt(&conversionOk));
            return conversionOk && m_tvShow->hasNewEpisodesInSeason(season);
        } else if (m_tvShow) {
            return m_tvShow->hasNewEpisodes() || !m_tvShow->infoLoaded();
        } else if (m_tvShowEpisode) {
            return !m_tvShowEpisode->infoLoaded();
        }
        break;

    case 2:
        if (m_tvShow) {
            return m_tvShow->hasChanged();
        } else if (m_tvShowEpisode) {
            return m_tvShowEpisode->hasChanged();
        }
        break;

    default:
        if (m_tvShow && m_season.isEmpty()) {
            return m_tvShow->name();
        } else if (m_tvShowEpisode) {
            return m_tvShowEpisode->completeEpisodeName();
        } else if (!m_season.isEmpty()) {
            return tr("Season %1").arg(m_season);
        }
        break;
    }
    return QVariant();
}


/// @brief Appends a TV show to this model.
/// @return Constructed child item
TvShowModelItem* TvShowModelItem::appendShow(TvShow* show)
{
    auto* item = new TvShowModelItem(this);
    item->setTvShow(show);
    show->setModelItem(item);
    m_children.append(item);
    return item;
}

/// @brief Appends an episode to this model.
/// @return Constructed child item
TvShowModelItem* TvShowModelItem::appendEpisode(TvShowEpisode* episode)
{
    auto* item = new TvShowModelItem(this);
    item->setTvShowEpisode(episode);
    episode->setModelItem(item);
    m_children.append(item);
    connect(episode, &TvShowEpisode::sigChanged, this, &TvShowModelItem::onTvShowEpisodeChanged, Qt::UniqueConnection);
    return item;
}

/// @brief Appends a season to this model
/// @return Constructed child item
TvShowModelItem* TvShowModelItem::appendSeason(SeasonNumber seasonNumber, QString season, TvShow* show)
{
    auto* item = new TvShowModelItem(this);
    item->setSeason(season);
    item->setSeasonNumber(seasonNumber);
    item->setTvShow(show);
    m_children.append(item);
    connect(item, &TvShowModelItem::sigIntChanged, this, &TvShowModelItem::onSeasonChanged, Qt::UniqueConnection);
    return item;
}

TvShowModelItem* TvShowModelItem::parent() const
{
    return m_parentItem;
}

bool TvShowModelItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_children.size()) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        m_children.takeAt(position)->deleteLater();
    }

    return true;
}

void TvShowModelItem::setTvShow(TvShow* show)
{
    m_tvShow = show;
}

void TvShowModelItem::setTvShowEpisode(TvShowEpisode* episode)
{
    m_tvShowEpisode = episode;
}

void TvShowModelItem::setSeason(QString season)
{
    m_season = season;
}

void TvShowModelItem::setSeasonNumber(SeasonNumber seasonNumber)
{
    m_seasonNumber = std::move(seasonNumber);
}

TvShow* TvShowModelItem::tvShow()
{
    return m_tvShow;
}

const TvShow* TvShowModelItem::tvShow() const
{
    return m_tvShow;
}

TvShowEpisode* TvShowModelItem::tvShowEpisode()
{
    return m_tvShowEpisode;
}

const TvShowEpisode* TvShowModelItem::tvShowEpisode() const
{
    return m_tvShowEpisode;
}

QString TvShowModelItem::season() const
{
    return m_season;
}

SeasonNumber TvShowModelItem::seasonNumber() const
{
    return m_seasonNumber;
}

/**
 * @brief TvShowModelItem::type
 * @return Type of this item
 */
TvShowType TvShowModelItem::type() const
{
    if (m_tvShow && m_season.isEmpty()) {
        return TvShowType::TvShow;
    } else if (m_tvShowEpisode) {
        return TvShowType::Episode;
    } else if (!m_season.isEmpty()) {
        return TvShowType::Season;
    }

    return TvShowType::None;
}

void TvShowModelItem::onTvShowEpisodeChanged(TvShowEpisode* episode)
{
    emit sigIntChanged(this, episode->modelItem());
}

void TvShowModelItem::onSeasonChanged(TvShowModelItem* seasonItem, TvShowModelItem* episodeItem)
{
    emit sigChanged(this, seasonItem, episodeItem);
}
