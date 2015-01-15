#include "MusicTreeView.h"

#include <QDebug>
#include "../globals/Globals.h"
#include "../globals/Manager.h"

MusicTreeView::MusicTreeView(QWidget *parent) :
    QTreeView(parent)
{
}

MusicTreeView::~MusicTreeView()
{
}

void MusicTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    if (index.model()->data(index, MusicRoles::Type).toInt() != TypeArtist)
        return;

    painter->save();

    QString text = (isExpanded(index)) ? QString(QChar(icon_angle_down)) : QString(QChar(icon_angle_right));
    int drawSize = qRound(rect.height() * 0.8);
    painter->setPen(QColor(70, 155, 198));
    painter->setPen(QColor(180, 180, 180));
    painter->setFont(Manager::instance()->iconFont()->font(drawSize));
    painter->drawText(rect, text, QTextOption(Qt::AlignCenter|Qt::AlignVCenter));

    painter->restore();
}

void MusicTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    int albumIndent = 40;
    int branchIndent = 30;
    bool isSelected = selectionModel()->isSelected(index);

    QStyleOptionViewItem opt = option;
    if (index.data(MusicRoles::Type).toInt() == TypeAlbum)
        opt.rect.setX(opt.rect.x() + albumIndent - 4);
    if (alternatingRowColors() && index.data(MusicRoles::Type).toInt() == TypeAlbum) {
        if (index.row()%2 == 0)
            opt.features |= QStyleOptionViewItem::Alternate;
        else
            opt.features &= ~QStyleOptionViewItem::Alternate;
    }
    if (isSelected)
        opt.state |= QStyle::State_Selected;
    style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, painter, this);


    if (index.data(MusicRoles::Type).toInt() == TypeArtist) {
        QRect branches(option.rect.x()+5, option.rect.y()+5, 20, option.rect.height()-10);
        drawBranches(painter, branches, index);

        int rowPadding = 4;
        int textRowHeight = (option.rect.height()-2*rowPadding)/2;
        QFont font = painter->font();

        int itemIndent = 0;
        if (index.data(MusicRoles::IsNew).toBool()) {
            itemIndent = 20;
            QRect iconRect(option.rect.x()+branchIndent, option.rect.y(), itemIndent-6, option.rect.height());
            int drawSize = qRound(iconRect.width() * 1.0);
            painter->setPen(isSelected ? QColor(255, 255, 255) : QColor(58, 135, 173));
            painter->setFont(Manager::instance()->iconFont()->font(drawSize));
            painter->drawText(iconRect, QString(QChar(icon_star)), QTextOption(Qt::AlignCenter|Qt::AlignVCenter));
        }

        QRect artistRect(option.rect.x()+branchIndent+itemIndent, option.rect.y()+rowPadding+1, option.rect.width()-branchIndent-itemIndent, textRowHeight);
        QRect albumsRect(option.rect.x()+branchIndent+itemIndent, option.rect.y()+textRowHeight+rowPadding, option.rect.width()-branchIndent-itemIndent, textRowHeight);

        painter->setPen(index.data(isSelected ? MusicRoles::SelectionForeground : Qt::ForegroundRole).value<QColor>());
        painter->setFont(index.data(Qt::FontRole).value<QFont>());
        painter->drawText(artistRect, index.data(Qt::DisplayRole).toString(), QTextOption(Qt::AlignVCenter));

        font = painter->font();
        font.setPointSize(font.pointSize()-2);
        font.setBold(false);
        painter->setFont(font);
        painter->drawText(albumsRect, tr("%n album(s)", "", index.data(MusicRoles::NumOfAlbums).toInt()), QTextOption(Qt::AlignVCenter));

        QPoint lineStart(option.rect.x(), option.rect.y());
        QPoint lineEnd(option.rect.x()+option.rect.width()-1, option.rect.y());
        painter->setPen(QColor(220, 220, 220));
        painter->drawLine(lineStart, lineEnd);

    } else {

        int itemIndent = 0;
        if (index.data(MusicRoles::IsNew).toBool()) {
            itemIndent = 20;
            QRect iconRect(option.rect.x()+albumIndent, option.rect.y(), itemIndent-6, option.rect.height());
            int drawSize = qRound(iconRect.width() * 1.0);
            painter->setPen(isSelected ? QColor(255, 255, 255) : QColor(58, 135, 173));
            painter->setFont(Manager::instance()->iconFont()->font(drawSize));
            painter->drawText(iconRect, QString(QChar(icon_star)), QTextOption(Qt::AlignCenter|Qt::AlignVCenter));
        }

        QRect albumRect(option.rect.x()+albumIndent+itemIndent, option.rect.y(), option.rect.width()-albumIndent-itemIndent, option.rect.height()-1);
        painter->setFont(index.data(Qt::FontRole).value<QFont>());
        painter->setPen(index.data(isSelected ? MusicRoles::SelectionForeground : Qt::ForegroundRole).value<QColor>());
        painter->drawText(albumRect, index.data().toString(), QTextOption(Qt::AlignVCenter));
    }

    painter->restore();
}
