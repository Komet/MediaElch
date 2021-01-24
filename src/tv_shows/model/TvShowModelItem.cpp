#include "tv_shows/model/TvShowModelItem.h"

#include <QApplication>
#include <QStringList>

#include "globals/Globals.h"
#include "globals/Manager.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"
#include "tv_shows/model/EpisodeModelItem.h"
#include "tv_shows/model/SeasonModelItem.h"
#include "tv_shows/model/TvShowRootModelItem.h"

TvShowModelItem::~TvShowModelItem()
{
    qDeleteAll(m_children);
}

/// \brief Returns a child or nullptr
/// \param number Child index
TvShowBaseModelItem* TvShowModelItem::child(int number) const
{
    return seasonAtIndex(number);
}

int TvShowModelItem::childCount() const
{
    return m_children.size();
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

TvShowBaseModelItem* TvShowModelItem::parent() const
{
    return &m_parentItem;
}

/// \brief Get the item's index in its parent item.
int TvShowModelItem::indexInParent() const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return m_parentItem.shows().indexOf(const_cast<TvShowModelItem*>(this));
}

TvShow* TvShowModelItem::tvShow()
{
    return m_tvShow;
}

const TvShow* TvShowModelItem::tvShow() const
{
    return m_tvShow;
}

TvShowType TvShowModelItem::type() const
{
    return TvShowType::TvShow;
}

SeasonModelItem* TvShowModelItem::seasonAtIndex(int number) const
{
    if (number < 0 || number >= m_children.size()) {
        return nullptr;
    }
    return m_children.at(number);
}


/// \brief Appends a season to this model
/// \return Constructed child item
SeasonModelItem* TvShowModelItem::appendSeason(SeasonNumber seasonNumber, QString season, TvShow* show)
{
    auto* item = new SeasonModelItem(*this);
    item->setSeason(season);
    item->setSeasonNumber(seasonNumber);
    item->setTvShow(show);
    m_children.append(item);
    connect(item, &SeasonModelItem::sigEpisodeChanged, this, &TvShowModelItem::onEpisodeChanged, Qt::UniqueConnection);
    return item;
}

const QList<SeasonModelItem*>& TvShowModelItem::seasons() const
{
    return m_children;
}

void TvShowModelItem::setTvShow(TvShow* show)
{
    m_tvShow = show;
}

void TvShowModelItem::onEpisodeChanged(SeasonModelItem* seasonItem, EpisodeModelItem* episodeItem)
{
    emit sigChanged(this, seasonItem, episodeItem);
}

QVariant TvShowModelItem::data(int column) const
{
    if (m_tvShow == nullptr) {
        return QVariant{};
    }

    const auto columnType = Columns(column);

    switch (columnType) {
    case Columns::Title: return m_tvShow->title();
    case Columns::EpisodeCount: return m_tvShow->episodeCount();
    case Columns::TvShowBanner: return m_tvShow->hasImage(ImageType::TvShowBanner);
    case Columns::TvShowPoster: return m_tvShow->hasImage(ImageType::TvShowPoster);
    case Columns::TvShowExtraFanart: return m_tvShow->hasImage(ImageType::TvShowExtraFanart);
    case Columns::TvShowBackdrop: return m_tvShow->hasImage(ImageType::TvShowBackdrop);
    case Columns::TvShowLogos: return m_tvShow->hasImage(ImageType::TvShowLogos);
    case Columns::TvShowThumb: return m_tvShow->hasImage(ImageType::TvShowThumb);
    case Columns::TvShowClearArt: return m_tvShow->hasImage(ImageType::TvShowClearArt);
    case Columns::TvShowCharacterArt: return m_tvShow->hasImage(ImageType::TvShowCharacterArt);
    case Columns::HasDummyEpisodes: return m_tvShow->hasDummyEpisodes();
    case Columns::Filename: {
        QString filename = Manager::instance()->mediaCenterInterface()->imageFileName(m_tvShow, ImageType::TvShowLogos);
        if (!filename.isEmpty()) {
            return filename;
        }
        return QVariant{};
    }
    case Columns::SyncNeeded: return m_tvShow->syncNeeded();
    case Columns::HasNewEpisodeOrInfoNotLoaded: return m_tvShow->hasNewEpisodes() || !m_tvShow->infoLoaded();
    case Columns::HasChanged: return m_tvShow->hasChanged();
    }

    return m_tvShow->title();
}
