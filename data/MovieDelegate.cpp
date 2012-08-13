#include "MovieDelegate.h"

#include <QDebug>
#include <QPainter>

MovieDelegate::MovieDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void MovieDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QFont newFont;
    #ifdef Q_WS_MAC
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

    QFont font;
    #ifdef Q_WS_MAC
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
