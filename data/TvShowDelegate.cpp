#include "TvShowDelegate.h"
#include <QDebug>
#include <QPainter>
#include "Globals.h"
#include "Manager.h"
#include "data/TvShow.h"
#include "data/TvShowModelItem.h"

TvShowDelegate::TvShowDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void TvShowDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.model()->data(index, TvShowRoles::Type).toInt() == TypeTvShow ) {
        painter->save();
        QFont font;

        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
            painter->setPen(QPen(option.palette.highlightedText().color()));
        }
        painter->translate(5, 3);
        QRect rect = option.rect;
        rect.setWidth(option.rect.width()-5);

        int showNameHeight = 16;

        font.setBold(true);
        #ifdef Q_WS_MAC
            font.setPointSize(12);
        #else
            font.setPixelSize(12);
        #endif
        if (index.data(TvShowRoles::HasChanged).toBool()) {
            font.setItalic(true);
            if (option.state & QStyle::State_Selected) {
            } else {
                painter->setPen(QPen(index.data(Qt::ForegroundRole).value<QColor>()));
            }
        }

        painter->setFont(font);
        painter->drawText(rect, Qt::AlignTop, index.data().toString());

        if (option.state & QStyle::State_Selected)
            painter->setPen(QPen(option.palette.highlightedText().color()));
        else
            painter->setPen(QPen(option.palette.text().color()));
        int episodeCount = index.data(TvShowRoles::EpisodeCount).toInt();
        font.setBold(false);
        #ifdef Q_WS_MAC
            font.setPointSize(10);
        #else
            font.setPixelSize(11);
        #endif
        painter->setFont(font);
        painter->drawText(rect.x(), rect.y()+showNameHeight, rect.width(), rect.height()-showNameHeight,
                          Qt::AlignTop, QString(tr("%n Episodes", "", episodeCount)));

        painter->restore();

    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize TvShowDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    if (index.data(TvShowRoles::Type) == TypeTvShow )
        size.setHeight(36);
    else
        size.setHeight(size.height()+6);
    return size;
}
