#include "TvShowDelegate.h"
#include <QDebug>
#include <QPainter>
#include "globals/Globals.h"
#include "globals/Manager.h"
#include "data/TvShow.h"
#include "data/TvShowModelItem.h"
#include "tvShows/ItemWidgetShow.h"

/**
 * @brief TvShowDelegate::TvShowDelegate
 * @param parent
 */
TvShowDelegate::TvShowDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    m_showItemHeight = 60;
}

/**
 * @brief Styles the types of items in the tv show list (show, season and episode)
 * @param painter
 * @param option
 * @param index
 */
void TvShowDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.model()->data(index, TvShowRoles::Type).toInt() == TypeTvShow ) {
        ItemWidgetShow *widget = new ItemWidgetShow();
        widget->setTitle(index.data(Qt::DisplayRole).toString());
        widget->setFixedHeight(m_showItemHeight);
        widget->setFixedWidth(option.rect.width());
        widget->setEpisodeCount(index.data(TvShowRoles::EpisodeCount).toInt());
        widget->setSyncNeeded(index.model()->data(index, TvShowRoles::SyncNeeded).toBool());
        widget->setNew(index.model()->data(index, TvShowRoles::IsNew).toBool());
        widget->setHasBanner(index.model()->data(index, TvShowRoles::HasBanner).toBool());
        widget->setHasFanart(index.model()->data(index, TvShowRoles::HasFanart).toBool());
        widget->setHasExtraFanart(index.model()->data(index, TvShowRoles::HasExtraFanart).toBool());
        widget->setHasPoster(index.model()->data(index, TvShowRoles::HasPoster).toBool());
        widget->setHasThumb(index.model()->data(index, TvShowRoles::HasThumb).toBool());
        widget->setHasLogo(index.model()->data(index, TvShowRoles::HasLogo).toBool());
        widget->setHasClearArt(index.model()->data(index, TvShowRoles::HasClearArt).toBool());
        widget->setHasCharacterArt(index.model()->data(index, TvShowRoles::HasCharacterArt).toBool());
        widget->deleteLater();

        painter->save();
        painter->translate(option.rect.topLeft());
        widget->render(painter);
        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize TvShowDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    if (index.data(TvShowRoles::Type) == TypeTvShow)
        size.setHeight(m_showItemHeight);
    else
        size.setHeight(size.height()+6);
    return size;
}
