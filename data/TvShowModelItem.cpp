#include "TvShowModelItem.h"

#include <QApplication>
#include <QStringList>

#include "globals/Globals.h"
#include "globals/Manager.h"

/**
 * @brief TvShowModelItem::TvShowModelItem
 * @param parent
 */
TvShowModelItem::TvShowModelItem(TvShowModelItem *parent) :
    QObject(nullptr),
    m_parentItem{parent},
    m_tvShow{nullptr},
    m_tvShowEpisode{nullptr},
    m_seasonNumber{0}
{
}

/**
 * @brief Deletes all child items
 */
TvShowModelItem::~TvShowModelItem()
{
    qDeleteAll(m_childItems);
}

/**
 * @brief Returns a child
 * @param number Child number
 * @return Child
 */
TvShowModelItem *TvShowModelItem::child(int number)
{
    return m_childItems.value(number);
}

/**
 * @brief TvShowModelItem::childCount
 * @return Number of child items
 */
int TvShowModelItem::childCount() const
{
    return m_childItems.count();
}

/**
 * @brief TvShowModelItem::childNumber
 * @return Child number of this object
 */
int TvShowModelItem::childNumber() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<TvShowModelItem *>(this));

    return 0;
}

/**
 * @brief TvShowModelItem::columnCount
 * @return Column count
 */
int TvShowModelItem::columnCount() const
{
    return 1;
}

/**
 * @brief TvShowModelItem::data
 * @param column
 * @return
 */
QVariant TvShowModelItem::data(int column) const
{
    switch (column) {
    case 101:
        if (m_tvShow)
            return m_tvShow->hasImage(ImageType::TvShowBanner);
        break;
    case 102:
        if (m_tvShow)
            return m_tvShow->hasImage(ImageType::TvShowPoster);
        break;
    case 103:
        if (m_tvShow)
            return m_tvShow->hasImage(ImageType::TvShowExtraFanart);
        break;
    case 104:
        if (m_tvShow)
            return m_tvShow->hasImage(ImageType::TvShowBackdrop);
        break;
    case 105:
        if (m_tvShow)
            return m_tvShow->hasImage(ImageType::TvShowLogos);
        break;
    case 106:
        if (m_tvShow)
            return m_tvShow->hasImage(ImageType::TvShowThumb);
        break;
    case 107:
        if (m_tvShow)
            return m_tvShow->hasImage(ImageType::TvShowClearArt);
        break;
    case 108:
        if (m_tvShow)
            return m_tvShow->hasImage(ImageType::TvShowCharacterArt);
        break;
    case 109:
        if (m_tvShow)
            return m_tvShow->hasDummyEpisodes();
        break;
    case 110:
        if (m_tvShow
            && !Manager::instance()->mediaCenterInterface()->imageFileName(m_tvShow, ImageType::TvShowLogos).isEmpty())
            return Manager::instance()->mediaCenterInterface()->imageFileName(m_tvShow, ImageType::TvShowLogos);
        break;
    case 4:
        if (m_tvShow)
            return m_tvShow->syncNeeded();
        else if (m_tvShowEpisode)
            return m_tvShowEpisode->syncNeeded();
        break;
    case 3:
        if (!m_season.isEmpty() && m_tvShow)
            return m_tvShow->hasNewEpisodesInSeason(m_season);
        else if (m_tvShow)
            return m_tvShow->hasNewEpisodes() || !m_tvShow->infoLoaded();
        else if (m_tvShowEpisode)
            return !m_tvShowEpisode->infoLoaded();
        break;
    case 2:
        if (m_tvShow)
            return m_tvShow->hasChanged();
        else if (m_tvShowEpisode)
            return m_tvShowEpisode->hasChanged();
        break;
    case 1:
        if (m_tvShow)
            return m_tvShow->episodeCount();
        break;
    default:
        if (m_tvShow && m_season.isEmpty())
            return m_tvShow->name();
        else if (m_tvShowEpisode)
            return m_tvShowEpisode->completeEpisodeName();
        else if (!m_season.isEmpty())
            return tr("Season %1").arg(m_season);
        break;
    }
    return QVariant();
}

/**
 * @brief Appends a tv show
 * @param show Show object to append
 * @return Constructed child item
 */
TvShowModelItem *TvShowModelItem::appendChild(TvShow *show)
{
    TvShowModelItem *item = new TvShowModelItem(this);
    item->setTvShow(show);
    show->setModelItem(item);
    m_childItems.append(item);
    return item;
}

/**
 * @brief Appends an episode
 * @param episode Episode object to append
 * @return Constructed child item
 */
TvShowModelItem *TvShowModelItem::appendChild(TvShowEpisode *episode)
{
    TvShowModelItem *item = new TvShowModelItem(this);
    item->setTvShowEpisode(episode);
    episode->setModelItem(item);
    m_childItems.append(item);
    connect(episode,
        SIGNAL(sigChanged(TvShowEpisode *)),
        this,
        SLOT(onTvShowEpisodeChanged(TvShowEpisode *)),
        Qt::UniqueConnection);
    return item;
}

/**
 * @brief Appends a season-child
 * @param season Number of the season
 * @param show Tv Show object
 * @return Constructed child item
 */
TvShowModelItem *TvShowModelItem::appendChild(int seasonNumber, QString season, TvShow *show)
{
    TvShowModelItem *item = new TvShowModelItem(this);
    item->setSeason(season);
    item->setSeasonNumber(seasonNumber);
    item->setTvShow(show);
    m_childItems.append(item);
    connect(item,
        SIGNAL(sigIntChanged(TvShowModelItem *, TvShowModelItem *)),
        this,
        SLOT(onSeasonChanged(TvShowModelItem *, TvShowModelItem *)),
        Qt::UniqueConnection);
    return item;
}

/**
 * @brief TvShowModelItem::parent
 * @return Parent item
 */
TvShowModelItem *TvShowModelItem::parent()
{
    return m_parentItem;
}

/**
 * @brief TvShowModelItem::removeChildren
 * @param position
 * @param count
 * @return
 */
bool TvShowModelItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete m_childItems.takeAt(position);

    return true;
}

/**
 * @brief TvShowModelItem::setTvShow
 * @param show Sets the TV Show object
 */
void TvShowModelItem::setTvShow(TvShow *show)
{
    m_tvShow = show;
}

/**
 * @brief TvShowModelItem::setTvShowEpisode
 * @param episode Sets the episode object
 */
void TvShowModelItem::setTvShowEpisode(TvShowEpisode *episode)
{
    m_tvShowEpisode = episode;
}

/**
 * @brief TvShowModelItem::setSeason
 * @param season Sets the season number
 */
void TvShowModelItem::setSeason(QString season)
{
    m_season = season;
}

void TvShowModelItem::setSeasonNumber(int seasonNumber)
{
    m_seasonNumber = seasonNumber;
}

/**
 * @brief TvShowModelItem::tvShow
 * @return Tv Show object of this item
 */
TvShow *TvShowModelItem::tvShow()
{
    return m_tvShow;
}

/**
 * @brief TvShowModelItem::tvShowEpisode
 * @return Episode object of this item
 */
TvShowEpisode *TvShowModelItem::tvShowEpisode()
{
    return m_tvShowEpisode;
}

/**
 * @brief TvShowModelItem::season
 * @return Seasonnumber
 */
QString TvShowModelItem::season()
{
    return m_season;
}

int TvShowModelItem::seasonNumber()
{
    return m_seasonNumber;
}

/**
 * @brief TvShowModelItem::type
 * @return Type of this item
 */
int TvShowModelItem::type()
{
    if (m_tvShow && m_season.isEmpty())
        return TypeTvShow;
    else if (m_tvShowEpisode)
        return TypeEpisode;
    else if (!m_season.isEmpty())
        return TypeSeason;

    return -1;
}

/**
 * @brief TvShowModelItem::onTvShowEpisodeChanged
 * @param episode
 */
void TvShowModelItem::onTvShowEpisodeChanged(TvShowEpisode *episode)
{
    emit sigIntChanged(this, episode->modelItem());
}

/**
 * @brief TvShowModelItem::onSeasonChanged
 * @param seasonItem
 * @param episodeItem
 */
void TvShowModelItem::onSeasonChanged(TvShowModelItem *seasonItem, TvShowModelItem *episodeItem)
{
    emit sigChanged(this, seasonItem, episodeItem);
}
