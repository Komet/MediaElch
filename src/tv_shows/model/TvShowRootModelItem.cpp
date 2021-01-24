#include "tv_shows/model/TvShowRootModelItem.h"

#include "tv_shows/TvShow.h"
#include "tv_shows/model/EpisodeModelItem.h"
#include "tv_shows/model/SeasonModelItem.h"
#include "tv_shows/model/TvShowModelItem.h"

TvShowRootModelItem::~TvShowRootModelItem()
{
    qDeleteAll(m_children);
}

QVariant TvShowRootModelItem::data(int column) const
{
    Q_UNUSED(column);
    return QVariant{};
}

TvShowBaseModelItem* TvShowRootModelItem::child(int number) const
{
    return showAtIndex(number);
}

int TvShowRootModelItem::childCount() const
{
    return m_children.size();
}

bool TvShowRootModelItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_children.size()) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        m_children.takeAt(position)->deleteLater();
    }

    return true;
}

TvShowBaseModelItem* TvShowRootModelItem::parent() const
{
    return nullptr;
}

int TvShowRootModelItem::indexInParent() const
{
    return 0;
}

TvShow* TvShowRootModelItem::tvShow()
{
    return nullptr;
}

const TvShow* TvShowRootModelItem::tvShow() const
{
    return nullptr;
}

TvShowType TvShowRootModelItem::type() const
{
    return TvShowType::None;
}

/// \brief Appends a TV show to this model.
/// \return Constructed child item
TvShowModelItem* TvShowRootModelItem::appendShow(TvShow* show)
{
    auto* item = new TvShowModelItem(*this);
    item->setTvShow(show);
    show->setModelItem(item);
    m_children.append(item);
    connect(item, &TvShowModelItem::sigChanged, this, &TvShowRootModelItem::onSigChanged, Qt::UniqueConnection);
    return item;
}

const QList<TvShowModelItem*>& TvShowRootModelItem::shows() const
{
    return m_children;
}

TvShowModelItem* TvShowRootModelItem::showAtIndex(int number) const
{
    if (number < 0 || number >= m_children.size()) {
        return nullptr;
    }
    return m_children.at(number);
}

void TvShowRootModelItem::onSigChanged(TvShowModelItem* show, SeasonModelItem* season, EpisodeModelItem* episode)
{
    emit sigChanged(show, season, episode);
}
