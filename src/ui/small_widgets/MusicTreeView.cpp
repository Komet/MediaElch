#include "MusicTreeView.h"

#include <QDebug>

#include "globals/Globals.h"
#include "globals/Manager.h"

MusicTreeView::MusicTreeView(QWidget* parent) : QTreeView(parent)
{
}

void MusicTreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
#ifdef Q_OS_WIN
    QTreeView::drawBranches(painter, rect, index);
    return;
#endif
    if (!isArtistRow(index)) {
        return;
    }

    const int drawSize = qRound(rect.height() * 0.8);
    const QColor grey(180, 180, 180);
    const QString text = (isExpanded(index)) ? QString(QChar(icon_angle_down)) : QString(QChar(icon_angle_right));

    painter->save();
    painter->setPen(grey);
    painter->setFont(Manager::instance()->iconFont()->font(drawSize));
    painter->drawText(rect, text, QTextOption(Qt::AlignCenter | Qt::AlignVCenter));

    painter->restore();
}

void MusicTreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
#ifdef Q_OS_WIN
    QTreeView::drawRow(painter, option, index);
    return;
#endif
    painter->save();

    QStyleOptionViewItem opt = option;
    if (isAlbumRow(index)) {
        opt.rect.setX(opt.rect.x() + m_albumIndent - 4);
    }
    setAlternateRowColors(opt, index);

    if (selectionModel()->isSelected(index)) {
        opt.state |= QStyle::State_Selected;
    }
    style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, painter, this);

    if (isArtistRow(index)) {
        drawArtistRow(painter, option, index);
    } else {
        drawAlbumRow(painter, option, index);
    }

    painter->restore();
}

void MusicTreeView::setAlternateRowColors(QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (alternatingRowColors() && isAlbumRow(index)) {
        if (index.row() % 2 == 0) {
            option.features |= QStyleOptionViewItem::Alternate;
        } else {
            option.features &= ~QStyleOptionViewItem::Alternate;
        }
    }
}

void MusicTreeView::drawArtistRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QRect branches(option.rect.x() + 5, option.rect.y() + 5, 20, option.rect.height() - 10);
    drawBranches(painter, branches, index);

    const bool isSelected = selectionModel()->isSelected(index);
    const int rowPadding = 4;
    const int itemIndent = drawNewIcon(painter, option, index, m_branchIndent);
    const int textRowHeight = (option.rect.height() - 2 * rowPadding) / 2;
    const int textRowWidth = option.rect.width() - m_branchIndent - itemIndent;
    const int posX = option.rect.x() + m_branchIndent + itemIndent;
    const int posY = option.rect.y() + rowPadding;

    QRect artistRect(posX, posY + 1, textRowWidth, textRowHeight);
    QRect albumsRect(posX, posY + textRowHeight, textRowWidth, textRowHeight);

    QFont font = index.data(Qt::FontRole).value<QFont>();
    painter->setPen(index.data(isSelected ? MusicRoles::SelectionForeground : Qt::ForegroundRole).value<QColor>());
    painter->setFont(font);

    const QFontMetrics metrics(font);
    const QString itemStr = metrics.elidedText(index.data().toString(), Qt::ElideRight, artistRect.width());
    painter->drawText(artistRect, itemStr, QTextOption(Qt::AlignVCenter));

#ifdef Q_OS_MAC
    font.setPointSize(font.pointSize() - 2);
#else
    font.setPointSize(font.pointSize() - 1);
#endif
    font.setBold(false);
    painter->setFont(font);
    painter->drawText(
        albumsRect, tr("%n albums", "", index.data(MusicRoles::NumOfAlbums).toInt()), QTextOption(Qt::AlignVCenter));

    const QPoint lineStart(option.rect.x(), option.rect.y());
    const QPoint lineEnd(option.rect.x() + option.rect.width() - 1, option.rect.y());
    painter->setPen(QColor(220, 220, 220));
    painter->drawLine(lineStart, lineEnd);
}


void MusicTreeView::drawAlbumRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const bool isSelected = selectionModel()->isSelected(index);
    const int itemIndent = drawNewIcon(painter, option, index, m_albumIndent);

    QRect albumRect(option.rect.x() + m_albumIndent + itemIndent,
        option.rect.y(),
        option.rect.width() - m_albumIndent - itemIndent,
        option.rect.height() - 1);
    const QFont font = index.data(Qt::FontRole).value<QFont>();
    painter->setFont(font);
    painter->setPen(index.data(isSelected ? MusicRoles::SelectionForeground : Qt::ForegroundRole).value<QColor>());
    const QFontMetrics metrics(font);
    const QString itemStr = metrics.elidedText(index.data().toString(), Qt::ElideRight, albumRect.width());
    painter->drawText(albumRect, itemStr, QTextOption(Qt::AlignVCenter));
}

/**
 * Draw the "is new" icon.
 * @return Row indent
 */
int MusicTreeView::drawNewIcon(QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index,
    int branchIndent) const
{
    if (!index.data(MusicRoles::IsNew).toBool()) {
        return 0;
    }
    const int itemIndent = 20;
    const bool isSelected = selectionModel()->isSelected(index);
    QRect iconRect(option.rect.x() + branchIndent, option.rect.y(), itemIndent - 6, option.rect.height());
    const int drawSize = qRound(iconRect.width() * 1.0);
    painter->setPen(isSelected ? QColor(255, 255, 255) : QColor(58, 135, 173));
    painter->setFont(Manager::instance()->iconFont()->font(drawSize));
    painter->drawText(iconRect, QString(QChar(icon_star)), QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
    return itemIndent;
}
