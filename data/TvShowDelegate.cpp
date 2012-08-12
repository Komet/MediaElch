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
    m_star = QPixmap(":/img/star_24.png");
    QPainter p;
    p.begin(&m_star);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(m_star.rect(), QColor(255, 150, 0, 100));
    p.end();
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

        if (index.model()->data(index, TvShowRoles::IsNew).toBool()) {
            painter->drawPixmap(rect.x(), rect.y()+showNameHeight, rect.height()-showNameHeight-8, rect.height()-showNameHeight-8,
                                m_star.scaled(rect.height()-showNameHeight-8, rect.height()-showNameHeight-8, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            painter->drawText(rect.x()+rect.height()-showNameHeight-4, rect.y()+showNameHeight, rect.width()-rect.height()+showNameHeight, rect.height()-showNameHeight,
                              Qt::AlignTop, QString(tr("%n Episodes", "", episodeCount)));
        } else {
            painter->drawText(rect.x(), rect.y()+showNameHeight, rect.width(), rect.height()-showNameHeight,
                              Qt::AlignTop, QString(tr("%n Episodes", "", episodeCount)));
        }

        painter->restore();

    } else if (index.model()->data(index, TvShowRoles::Type).toInt() == TypeEpisode) {
        painter->save();

        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
            painter->setPen(QPen(option.palette.highlightedText().color()));
        }

        QFont font;
        #ifdef Q_WS_MAC
        font.setPointSize(font.pointSize()-2);
        #endif

        painter->setFont(font);

        if (index.model()->data(index, TvShowRoles::IsNew).toBool()) {
            painter->translate(option.rect.height()-3, 3);
            painter->drawText(option.rect, Qt::AlignTop,  index.data().toString());
            painter->translate(-option.rect.height()+3, -1);
            painter->drawPixmap(option.rect.x(), option.rect.y(), option.rect.height()-6, option.rect.height()-6,
                                m_star.scaled(option.rect.height()-6, option.rect.height()-6, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            painter->translate(0, 3);
            painter->drawText(option.rect, Qt::AlignTop,  index.data().toString());
        }

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
