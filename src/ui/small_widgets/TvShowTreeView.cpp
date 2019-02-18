#include "TvShowTreeView.h"

#include "globals/Globals.h"
#include "globals/Manager.h"

#include <QDebug>
#include <QHeaderView>

TvShowTreeView::TvShowTreeView(QWidget* parent) :
    QTreeView(parent),
    m_newIcon{QPixmap(":/img/star_blue.png").scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation)},
    m_syncIcon{QPixmap(":/img/reload_orange.png").scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation)},
    m_missingIcon{QPixmap(":/img/missing.png").scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation)}
{
    setAllColumnsShowFocus(true);
}

void TvShowTreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    if (getTvShowType(index) == TvShowType::Episode) {
        return;
    }

    painter->save();

    QString text = isExpanded(index) ? QChar(icon_angle_down) : QChar(icon_angle_right);
    const int drawSize = qRound(rect.height() * 0.8);
    painter->setPen(QColor(180, 180, 180));
    painter->setFont(Manager::instance()->iconFont()->font(drawSize));
    painter->drawText(rect, text, QTextOption(Qt::AlignCenter | Qt::AlignVCenter));

    painter->restore();
}

void TvShowTreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();

    if (getTvShowType(index) == TvShowType::TvShow) {
        drawTvShowRow(painter, option, index);

    } else {
        drawEpisodeRow(painter, option, index);
    }

    painter->restore();
}

void TvShowTreeView::drawTvShowRow(QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    const bool isSelected = selectionModel()->isSelected(index);

    drawRowBackground(painter, option, index);
    drawTvShowIcons(painter, option, index);

    QRect branches(option.rect.x() + 5, option.rect.y() + 5, 20, option.rect.height() - 10);
    drawBranches(painter, branches, index);

    const int rowPadding = 4;
    int textRowHeight = (option.rect.height() - 2 * rowPadding) / 2;
    QFont font = painter->font();

    int itemIndent = 0;
    if (index.data(TvShowRoles::IsNew).toBool()) {
        itemIndent = 20;
        // On Windows image files are drawn whereas on mac and linux FontAwesome icons are used.
#ifdef Q_OS_WIN
        QRect iconRect(option.rect.x() + m_branchIndent, option.rect.y(), itemIndent - 6, option.rect.height());
        painter->drawPixmap(iconRect.x() + (iconRect.width() - m_newIcon.width()) / 2,
            iconRect.y() + (iconRect.height() - m_newIcon.height()) / 2,
            m_newIcon);
#else
        QRect iconRect(option.rect.x() + m_branchIndent, option.rect.y(), itemIndent - 6, option.rect.height());
        int drawSize = qRound(iconRect.width() * 1.0);
        painter->setPen(isSelected ? QColor(255, 255, 255) : QColor(58, 135, 173));
        painter->setFont(Manager::instance()->iconFont()->font(drawSize));
        painter->drawText(iconRect, QString(QChar(icon_star)), QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
#endif
    }

    QRect showRect(option.rect.x() + m_branchIndent + itemIndent,
        option.rect.y() + rowPadding + 1,
        option.rect.width() - m_branchIndent - itemIndent,
        textRowHeight);
    QRect episodesRect(option.rect.x() + m_branchIndent + itemIndent,
        option.rect.y() + textRowHeight + rowPadding,
        header()->sectionSize(0) - m_branchIndent - itemIndent,
        textRowHeight);

    font = index.data(Qt::FontRole).value<QFont>();
    painter->setPen(index.data(isSelected ? TvShowRoles::SelectionForeground : Qt::ForegroundRole).value<QColor>());
    painter->setFont(font);

    QFontMetrics metrics(font);
    const QString itemStr = metrics.elidedText(index.data().toString(), Qt::ElideRight, showRect.width());
    painter->drawText(showRect, itemStr, QTextOption(Qt::AlignVCenter));

#ifdef Q_OS_MAC
    font.setPointSize(font.pointSize() - 2);
#else
    font.setPointSize(font.pointSize() - 1);
#endif
    font.setBold(false);

    metrics = QFontMetrics(font);
    const QString episodeStr = metrics.elidedText(
        tr("%n episodes", "", index.data(TvShowRoles::EpisodeCount).toInt()), Qt::ElideRight, episodesRect.width());
    painter->setFont(font);
    painter->drawText(episodesRect, episodeStr, QTextOption(Qt::AlignVCenter));

    QPoint lineStart(option.rect.x(), option.rect.y());
    QPoint lineEnd(option.rect.x() + option.rect.width() - 1, option.rect.y());
    painter->setPen(QColor(220, 220, 220));
    painter->drawLine(lineStart, lineEnd);
}

void TvShowTreeView::drawTvShowIcons(QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    const QHeaderView* horizontalHeader = header();
    const int columnCount = index.model()->columnCount();

    for (int col = 1; col < columnCount; ++col) {
        const QRect iconRect(option.rect.x() + columnViewportPosition(col),
            option.rect.y(),
            horizontalHeader->sectionSize(col),
            option.rect.height());
        const QPixmap icon =
            model()->index(index.row(), col).data(Qt::DecorationRole).value<QIcon>().pixmap(iconSize());
        painter->drawPixmap(iconRect.x() + (iconRect.width() - icon.width()) / 2,
            iconRect.y() + (iconRect.height() - icon.height()) - 6,
            icon);
    }
}

void TvShowTreeView::drawEpisodeRow(QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    const bool isSelected = selectionModel()->isSelected(index);
    const TvShowType type = getTvShowType(index);
    int itemIndent = (type == TvShowType::Season) ? m_seasonIndent : m_episodeIndent;
    drawRowBackground(painter, option, index);

    if (type == TvShowType::Season) {
        QRect branches(option.rect.x() + 25, option.rect.y() + 5, 20, option.rect.height() - 10);
        drawBranches(painter, branches, index);
        itemIndent += 20;
    }

    // On Windows image files are drawn whereas on mac and linux FontAwesome icons are used.

    if (index.data(TvShowRoles::IsNew).toBool()) {
#ifdef Q_OS_WIN
        QRect iconRect(option.rect.x() + itemIndent, option.rect.y(), 18, option.rect.height());
        painter->drawPixmap(iconRect.x() + (iconRect.width() - m_newIcon.width()) / 2,
            iconRect.y() + (iconRect.height() - m_newIcon.height()) / 2,
            m_newIcon);
#else
        QRect iconRect(option.rect.x() + itemIndent, option.rect.y(), 14, option.rect.height());
        int drawSize = qRound(iconRect.width() * 1.0);
        painter->setPen(isSelected ? QColor(255, 255, 255) : QColor(58, 135, 173));
        painter->setFont(Manager::instance()->iconFont()->font(drawSize));
        painter->drawText(iconRect, QString(QChar(icon_star)), QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
#endif
        itemIndent += 20;
    }

    if (index.data(TvShowRoles::SyncNeeded).toBool()) {
#ifdef Q_OS_WIN
        QRect iconRect(option.rect.x() + itemIndent, option.rect.y(), 18, option.rect.height());
        painter->drawPixmap(iconRect.x() + (iconRect.width() - m_syncIcon.width()) / 2,
            iconRect.y() + (iconRect.height() - m_syncIcon.height()) / 2,
            m_syncIcon);
#else
        QRect iconRect(option.rect.x() + itemIndent, option.rect.y(), 14, option.rect.height());
        int drawSize = qRound(iconRect.width() * 1.0);
        painter->setPen(isSelected ? QColor(255, 255, 255) : QColor(248, 148, 6));
        painter->setFont(Manager::instance()->iconFont()->font(drawSize));
        painter->drawText(
            iconRect, QString(QChar(icon_refresh_cloud)), QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
#endif
        itemIndent += 20;
    }

    if (TvShowType(index.data(TvShowRoles::Type).toInt()) == TvShowType::Season
        && index.data(TvShowRoles::HasDummyEpisodes).toBool()) {
#ifdef Q_OS_WIN
        QRect iconRect(option.rect.x() + itemIndent, option.rect.y(), 18, option.rect.height());
        painter->drawPixmap(iconRect.x() + (iconRect.width() - m_missingIcon.width()) / 2,
            iconRect.y() + (iconRect.height() - m_missingIcon.height()) / 2,
            m_missingIcon);
#else
        QRect iconRect(option.rect.x() + itemIndent, option.rect.y(), 14, option.rect.height());
        int drawSize = qRound(iconRect.width() * 1.0);
        painter->setPen(isSelected ? QColor(255, 255, 255) : QColor(241, 96, 106));
        painter->setFont(Manager::instance()->iconFont()->font(drawSize));
        painter->drawText(iconRect, QString(QChar(icon_attention)), QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
#endif
        itemIndent += 20;
    }

    const QRect itemRect(
        option.rect.x() + itemIndent, option.rect.y(), option.rect.width() - itemIndent, option.rect.height() - 1);
    const QFont font = index.data(Qt::FontRole).value<QFont>();
    painter->setFont(font);
    painter->setPen(index.data(isSelected ? TvShowRoles::SelectionForeground : Qt::ForegroundRole).value<QColor>());
    const QFontMetrics metrics(font);
    const QString itemStr = metrics.elidedText(index.data().toString(), Qt::ElideRight, itemRect.width());
    painter->drawText(itemRect, itemStr, QTextOption(Qt::AlignVCenter));
}

void TvShowTreeView::drawRowBackground(QPainter* painter, QStyleOptionViewItem option, const QModelIndex& index) const
{
    const TvShowType type = getTvShowType(index);

    if (type != TvShowType::TvShow) {
        const int indent = (type == TvShowType::Season) ? m_seasonIndent : m_episodeIndent;
        option.rect.setX(option.rect.x() + indent - 4);

        if (alternatingRowColors()) {
            if (index.row() % 2 == 0) {
                option.features |= QStyleOptionViewItem::Alternate;
#ifdef Q_OS_WIN
                option.features &= ~QStyleOptionViewItem::Alternate;
#endif
            } else {
                option.features &= ~QStyleOptionViewItem::Alternate;
            }
        }
    }

    if (selectionModel()->isSelected(index)) {
        option.state |= QStyle::State_Selected;
#ifdef Q_OS_WIN
        const QColor blue(27, 106, 165);
        QPen pen(blue);
        pen.setWidth(0);
        painter->setPen(pen);
        painter->setBrush(QBrush(blue));
        painter->drawRect(option.rect);
#endif
    }

    style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &option, painter, this);
}

TvShowType TvShowTreeView::getTvShowType(const QModelIndex& index) const
{
    return TvShowType(index.model()->data(index, TvShowRoles::Type).toInt());
}
