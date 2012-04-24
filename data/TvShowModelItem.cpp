#include "TvShowModelItem.h"

#include <QStringList>
#include "Globals.h"

TvShowModelItem::TvShowModelItem(TvShowModelItem *parent)
{
    m_parentItem = parent;
    m_tvShow = 0;
    m_tvShowEpisode = 0;
}

TvShowModelItem::~TvShowModelItem()
{
    qDeleteAll(m_childItems);
}

TvShowModelItem *TvShowModelItem::child(int number)
{
    return m_childItems.value(number);
}

int TvShowModelItem::childCount() const
{
    return m_childItems.count();
}

int TvShowModelItem::childNumber() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<TvShowModelItem*>(this));

    return 0;
}

int TvShowModelItem::columnCount() const
{
    return 1;
}

QVariant TvShowModelItem::data(int column) const
{
    Q_UNUSED(column);
    switch (column)
    {
    case 1:
        if (m_tvShow)
            return m_tvShow->episodeCount();
        break;
    default:
        if (m_tvShow)
            return m_tvShow->name();
        else if (m_tvShowEpisode)
            return m_tvShowEpisode->completeEpisodeName();
        break;
    }
    return QVariant();
}

TvShowModelItem *TvShowModelItem::appendChild(TvShow *show)
{
    TvShowModelItem *item = new TvShowModelItem(this);
    item->setTvShow(show);
    m_childItems.append(item);
    return item;
}

TvShowModelItem *TvShowModelItem::appendChild(TvShowEpisode *episode)
{
    TvShowModelItem *item = new TvShowModelItem(this);
    item->setTvShowEpisode(episode);
    m_childItems.append(item);
    return item;
}

TvShowModelItem *TvShowModelItem::parent()
{
    return m_parentItem;
}

bool TvShowModelItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete m_childItems.takeAt(position);

    return true;
}

void TvShowModelItem::setTvShow(TvShow *show)
{
    m_tvShow = show;
}

void TvShowModelItem::setTvShowEpisode(TvShowEpisode *episode)
{
    m_tvShowEpisode = episode;
}

TvShow *TvShowModelItem::tvShow()
{
    return m_tvShow;
}

TvShowEpisode *TvShowModelItem::tvShowEpisode()
{
    return m_tvShowEpisode;
}

int TvShowModelItem::type()
{
    if (m_tvShow)
        return TypeTvShow;
    else if (m_tvShowEpisode)
        return TypeEpisode;

    return -1;
}
