#include "ConcertDelegate.h"

#include <QDebug>
#include <QPainter>
#include "globals/Globals.h"

/**
 * @brief ConcertDelegate::ConcertDelegate
 * @param parent
 */
ConcertDelegate::ConcertDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

/**
 * @brief Overrides the paint event: Show a "new" label in front of the concert name
 * @param painter
 * @param option
 * @param index
 */
void ConcertDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QFont newFont;
    #ifdef Q_OS_MAC
    newFont.setPixelSize(8);
    #else
    newFont.setPixelSize(10);
    #endif
    QString newInd = tr("new");
    int newWidth = 21;
    int newHeight = newFont.pixelSize();

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    int xMove = 6;

    if (!index.data(Qt::UserRole+1).toBool()) {
        // is new
        xMove += newWidth+6;
        QRect newRect(option.rect.x()+4, option.rect.y()+((option.rect.height()-newHeight-2)/2), newWidth+4, newHeight+4);
        painter->setPen(QColor(58, 135, 173));
        painter->setBrush(QBrush(QColor(58, 135, 173)));
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawRoundedRect(newRect, 4, 4);
        painter->setPen(QColor(255, 255, 255));
        painter->drawText(newRect.x(), newRect.y(), newRect.width(), newRect.height()-2, Qt::AlignCenter | Qt::AlignVCenter, newInd);
    }

    if (index.data(Qt::UserRole+3).toBool()) {
        // sync needed
        newWidth = newHeight;
        QRect newRect(option.rect.x()+4+xMove-6, option.rect.y()+((option.rect.height()-newHeight-2)/2), newWidth+4, newHeight+4);
        painter->setPen(QColor(248, 148, 6));
        painter->setBrush(QBrush(QColor(248, 148, 6)));
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawRoundedRect(newRect, 4, 4);
        painter->drawPixmap(newRect.x()+2, newRect.y()+2, newRect.width()-4, newRect.height()-4,
                            QPixmap(":/img/sync.png").scaled(newRect.width()-4, newRect.height()-4, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        xMove += newWidth+6;
    }

    QFont font;
    #ifdef Q_OS_MAC
    font.setPointSize(font.pointSize()-2);
    #endif

    painter->setPen(QPen(index.data(Qt::ForegroundRole).value<QColor>()));
    if (index.data(Qt::UserRole+2).toBool())
        font.setItalic(true);
    if (option.state & QStyle::State_Selected)
        painter->setPen(QPen(option.palette.highlightedText().color()));
    painter->setFont(font);
    painter->drawText(option.rect.x()+xMove, option.rect.y(), option.rect.width()-xMove, option.rect.height(), Qt::AlignVCenter, index.data().toString());

    painter->restore();
}
