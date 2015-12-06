#include "TvShowTreeView.h"

#include <QDebug>
#include <QHeaderView>
#include "../globals/Globals.h"
#include "../globals/Manager.h"

TvShowTreeView::TvShowTreeView(QWidget *parent) :
    QTreeView(parent)
{
    setAllColumnsShowFocus(true);
    m_newIcon = QPixmap(":/img/star_blue.png").scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_syncIcon = QPixmap(":/img/reload_orange.png").scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_missingIcon = QPixmap(":/img/missing.png").scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

TvShowTreeView::~TvShowTreeView()
{
}

void TvShowTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    if (index.model()->data(index, TvShowRoles::Type).toInt() == TypeEpisode)
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

void TvShowTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    int seasonIndent = 30;
    int episodeIndent = 50;
    int branchIndent = 30;
    bool isSelected = selectionModel()->isSelected(index);

    QStyleOptionViewItem opt = option;
    if (index.data(TvShowRoles::Type).toInt() == TypeSeason)
        opt.rect.setX(opt.rect.x() + seasonIndent - 4);
    else if (index.data(TvShowRoles::Type).toInt() == TypeEpisode)
        opt.rect.setX(opt.rect.x() + episodeIndent - 4);

    if (alternatingRowColors() && index.data(TvShowRoles::Type).toInt() != TypeTvShow) {
        if (index.row()%2 == 0)
            opt.features |= QStyleOptionViewItem::Alternate;
        else
            opt.features &= ~QStyleOptionViewItem::Alternate;
    }

#ifdef Q_OS_WIN
    style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, painter, this);
#endif
    if (isSelected) {
        opt.state |= QStyle::State_Selected;
#ifdef Q_OS_WIN
        QPen pen(QColor(27, 106, 165));
        pen.setWidth(0);
        painter->setPen(pen);
        painter->setBrush(QBrush(QColor(27, 106, 165)));
        painter->drawRect(opt.rect);
#endif
    }

#ifndef Q_OS_WIN
    style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, painter, this);
#endif

    if (index.data(TvShowRoles::Type).toInt() == TypeTvShow) {

        QHeaderView *horizontalHeader = header();
        for (int col=1, n=index.model()->columnCount() ; col<n ; ++col) {
            QRect iconRect(option.rect.x() + columnViewportPosition(col), option.rect.y(), horizontalHeader->sectionSize(col), option.rect.height());
            QPixmap icon = model()->index(index.row(), col).data(Qt::DecorationRole).value<QIcon>().pixmap(iconSize());
            painter->drawPixmap(iconRect.x() + (iconRect.width()-icon.width())/2, iconRect.y() + (iconRect.height()-icon.height()) - 6, icon);
        }

        QRect branches(option.rect.x()+5, option.rect.y()+5, 20, option.rect.height()-10);
        drawBranches(painter, branches, index);

        int rowPadding = 4;
        int textRowHeight = (option.rect.height()-2*rowPadding)/2;
        QFont font = painter->font();

        int itemIndent = 0;
        if (index.data(TvShowRoles::IsNew).toBool()) {
            itemIndent = 20;
#ifdef Q_OS_WIN
            QRect iconRect(option.rect.x()+branchIndent, option.rect.y(), itemIndent-6, option.rect.height());
            painter->drawPixmap(iconRect.x() + (iconRect.width()-m_newIcon.width())/2, iconRect.y() + (iconRect.height()-m_newIcon.height())/2, m_newIcon);
#else
            QRect iconRect(option.rect.x()+branchIndent, option.rect.y(), itemIndent-6, option.rect.height());
            int drawSize = qRound(iconRect.width() * 1.0);
            painter->setPen(isSelected ? QColor(255, 255, 255) : QColor(58, 135, 173));
            painter->setFont(Manager::instance()->iconFont()->font(drawSize));
            painter->drawText(iconRect, QString(QChar(icon_star)), QTextOption(Qt::AlignCenter|Qt::AlignVCenter));
#endif
        }

        QRect showRect(option.rect.x()+branchIndent+itemIndent, option.rect.y()+rowPadding+1, option.rect.width()-branchIndent-itemIndent, textRowHeight);
        QRect episodesRect(option.rect.x()+branchIndent+itemIndent, option.rect.y()+textRowHeight+rowPadding, option.rect.width()-branchIndent-itemIndent, textRowHeight);

        painter->setPen(index.data(isSelected ? TvShowRoles::SelectionForeground : Qt::ForegroundRole).value<QColor>());
        painter->setFont(index.data(Qt::FontRole).value<QFont>());
        painter->drawText(showRect, index.data(Qt::DisplayRole).toString(), QTextOption(Qt::AlignVCenter));

        font = painter->font();
#ifdef Q_OS_MAC
        font.setPointSize(font.pointSize()-2);
#else
        font.setPointSize(font.pointSize()-1);
#endif
        font.setBold(false);
        painter->setFont(font);
        painter->drawText(episodesRect, tr("%n episodes", "", index.data(TvShowRoles::EpisodeCount).toInt()), QTextOption(Qt::AlignVCenter));

        QPoint lineStart(option.rect.x(), option.rect.y());
        QPoint lineEnd(option.rect.x()+option.rect.width()-1, option.rect.y());
        painter->setPen(QColor(220, 220, 220));
        painter->drawLine(lineStart, lineEnd);

    } else {

        int itemIndent = (index.data(TvShowRoles::Type).toInt() == TypeSeason) ? seasonIndent : episodeIndent;

        if (index.data(TvShowRoles::Type).toInt() == TypeSeason) {
            QRect branches(option.rect.x()+25, option.rect.y()+5, 20, option.rect.height()-10);
            drawBranches(painter, branches, index);
            itemIndent += 20;
        }

        if (index.data(TvShowRoles::IsNew).toBool()) {
#ifdef Q_OS_WIN
            QRect iconRect(option.rect.x()+itemIndent, option.rect.y(), 18, option.rect.height());
            painter->drawPixmap(iconRect.x() + (iconRect.width()-m_newIcon.width())/2, iconRect.y() + (iconRect.height()-m_newIcon.height())/2, m_newIcon);
#else
            QRect iconRect(option.rect.x()+itemIndent, option.rect.y(), 14, option.rect.height());
            int drawSize = qRound(iconRect.width() * 1.0);
            painter->setPen(isSelected ? QColor(255, 255, 255) : QColor(58, 135, 173));
            painter->setFont(Manager::instance()->iconFont()->font(drawSize));
            painter->drawText(iconRect, QString(QChar(icon_star)), QTextOption(Qt::AlignCenter|Qt::AlignVCenter));
#endif
            itemIndent += 20;
        }

        if (index.data(TvShowRoles::SyncNeeded).toBool()) {
#ifdef Q_OS_WIN
            QRect iconRect(option.rect.x()+itemIndent, option.rect.y(), 18, option.rect.height());
            painter->drawPixmap(iconRect.x() + (iconRect.width()-m_syncIcon.width())/2, iconRect.y() + (iconRect.height()-m_syncIcon.height())/2, m_syncIcon);
#else
            QRect iconRect(option.rect.x()+itemIndent, option.rect.y(), 14, option.rect.height());
            int drawSize = qRound(iconRect.width() * 1.0);
            painter->setPen(isSelected ? QColor(255, 255, 255) : QColor(248, 148, 6));
            painter->setFont(Manager::instance()->iconFont()->font(drawSize));
            painter->drawText(iconRect, QString(QChar(icon_refresh_cloud)), QTextOption(Qt::AlignCenter|Qt::AlignVCenter));
#endif
            itemIndent += 20;
        }

        if (index.data(TvShowRoles::Type).toInt() == TypeSeason && index.data(TvShowRoles::HasDummyEpisodes).toBool()) {
#ifdef Q_OS_WIN
            QRect iconRect(option.rect.x()+itemIndent, option.rect.y(), 18, option.rect.height());
            painter->drawPixmap(iconRect.x() + (iconRect.width()-m_missingIcon.width())/2, iconRect.y() + (iconRect.height()-m_missingIcon.height())/2, m_missingIcon);
#else
            QRect iconRect(option.rect.x()+itemIndent, option.rect.y(), 14, option.rect.height());
            int drawSize = qRound(iconRect.width() * 1.0);
            painter->setPen(isSelected ? QColor(255, 255, 255) : QColor(241, 96, 106));
            painter->setFont(Manager::instance()->iconFont()->font(drawSize));
            painter->drawText(iconRect, QString(QChar(icon_attention)), QTextOption(Qt::AlignCenter|Qt::AlignVCenter));
#endif
            itemIndent += 20;
        }

        QRect itemRect(option.rect.x()+itemIndent, option.rect.y(), option.rect.width()-itemIndent, option.rect.height()-1);
        QFont font = index.data(Qt::FontRole).value<QFont>();
        painter->setFont(font);
        painter->setPen(index.data(isSelected ? TvShowRoles::SelectionForeground : Qt::ForegroundRole).value<QColor>());
        painter->drawText(itemRect, index.data().toString(), QTextOption(Qt::AlignVCenter));

    }

    painter->restore();
}
