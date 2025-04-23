#include "TvShowTreeView.h"

#include "globals/Globals.h"
#include "globals/Manager.h"

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
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(index)
    return; // no-op
}

void TvShowTreeView::drawBranches(QPainter* painter,
    const QStyleOptionViewItem& option,
    const QRect& rect,
    const QModelIndex& index) const
{
    if (isEpisodeRow(index)) {
        return;
    }

    const bool isSelected = option.state.testFlag(QStyle::State_Selected);
    const QPalette::ColorRole textColorRole = isSelected ? QPalette::HighlightedText : QPalette::Text;
    QColor textColor = option.palette.color(textColorRole);

    const int drawSize = qRound(rect.height() * 0.85);
    QString text = isExpanded(index) ? QChar(static_cast<uint>(icon_angle_down)): QChar(static_cast<uint>(icon_angle_right));

    painter->save();
    painter->setPen(textColor);
    painter->setFont(Manager::instance()->iconFont()->font(drawSize));
    painter->drawText(rect, text, QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
    painter->restore();
}

void TvShowTreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();

    QStyleOptionViewItem opt = option;
    if (alternatingRowColors()) {
        if (index.row() % 2 == 1) {
            opt.features |= QStyleOptionViewItem::Alternate;
        } else {
            opt.features &= ~QStyleOptionViewItem::Alternate;
        }
    }

    if (selectionModel()->isSelected(index)) {
        opt.state |= QStyle::State_Selected;
    }

    // Draw Background
    drawRowBackground(painter, opt, index);

    if (isShowRow(index)) {
        drawTvShowRow(painter, opt, index);

    } else {
        drawEpisodeRow(painter, opt, index);
    }

    painter->restore();
}

void TvShowTreeView::drawTvShowRow(QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;

    QColor red(255, 0, 0);
    const bool isSelected = opt.state.testFlag(QStyle::State_Selected);
    const bool hasChanged = index.data(TvShowRoles::HasChanged).toBool();

    const QPalette::ColorRole textColorRole = isSelected ? QPalette::HighlightedText : QPalette::Text;
    QColor textColor = (hasChanged && !isSelected) ? red : opt.palette.color(textColorRole);
    QFont textFont = painter->font();
    textFont.setItalic(hasChanged);
    textFont.setBold(true);

    drawTvShowIcons(painter, option, index);

    QRect branches(option.rect.x() + 5, option.rect.y() + 5, 20, option.rect.height() - 10);
    drawBranches(painter, opt, branches, index);

    const int rowPadding = 4;
    const int itemIndent = m_branchIndent + drawNewIcon(painter, option, index, isSelected, m_branchIndent);

    const int textRowHeight = (option.rect.height() - 2 * rowPadding) / 2;
    const int textRowWidth = option.rect.width() - itemIndent;
    const int posX = option.rect.x() + itemIndent;
    const int posY = option.rect.y() + rowPadding;

    QRect showRect(posX, posY + 1, textRowWidth, textRowHeight);
    QRect episodesRect(posX, //
        posY + textRowHeight,
        header()->sectionSize(0) - m_branchIndent - itemIndent,
        textRowHeight);

    painter->setFont(textFont);
    painter->setPen(textColor);

    QFontMetrics metrics(textFont);
    const QString itemStr = metrics.elidedText(index.data().toString(), Qt::ElideRight, showRect.width());

    style()->drawItemText(painter, showRect, (Qt::AlignLeft | Qt::AlignVCenter), opt.palette, true, itemStr);

#ifdef Q_OS_MAC
    textFont.setPointSize(textFont.pointSize() - 2);
#else
    textFont.setPointSize(textFont.pointSize() - 1);
#endif
    textFont.setBold(false);

    metrics = QFontMetrics(textFont);
    const QString episodeStr = metrics.elidedText(
        tr("%n episodes", "", index.data(TvShowRoles::EpisodeCount).toInt()), Qt::ElideRight, episodesRect.width());
    painter->setFont(textFont);
    painter->setPen(textColor);
    style()->drawItemText(painter, episodesRect, (Qt::AlignLeft | Qt::AlignVCenter), opt.palette, true, episodeStr);

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
    QColor red(255, 0, 0);
    const bool isSelected = option.state.testFlag(QStyle::State_Selected);
    const bool hasChanged = index.data(TvShowRoles::HasChanged).toBool();
    const bool isSeason = isSeasonRow(index);
    int itemIndent = isSeason ? m_seasonIndent : m_episodeIndent;
    const QPalette::ColorRole textColorRole = isSelected ? QPalette::HighlightedText : QPalette::Text;

    QColor textColor = (hasChanged && !isSelected) ? red : option.palette.color(textColorRole);
    QFont textFont = painter->font();
    painter->setPen(textColor);
    textFont.setBold(isSeason);
    textFont.setItalic(hasChanged);

    if (isSeason) {
        QRect branches(option.rect.x() + 25, option.rect.y() + 5, 20, option.rect.height() - 10);
        drawBranches(painter, option, branches, index);
        itemIndent += 20;
    }

    itemIndent += drawNewIcon(painter, option, index, isSelected, itemIndent);

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
            iconRect, QString(QChar(static_cast<uint>(icon_refresh_cloud))), QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
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
        painter->drawText(iconRect, QString(QChar(static_cast<uint>(icon_attention))), QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
#endif
        itemIndent += 20;
    }

    const QRect itemRect(
        option.rect.x() + itemIndent, option.rect.y(), option.rect.width() - itemIndent, option.rect.height() - 1);
    const QFontMetrics metrics(textFont);
    const QString itemStr = metrics.elidedText(index.data().toString(), Qt::ElideRight, itemRect.width());

    painter->setFont(textFont);
    painter->setPen(textColor);
    style()->drawItemText(painter, itemRect, (Qt::AlignLeft | Qt::AlignVCenter), option.palette, true, itemStr);
}

void TvShowTreeView::drawRowBackground(QPainter* painter, QStyleOptionViewItem opt, const QModelIndex& index) const
{
    if (!isShowRow(index)) {
        const int indent = (isSeasonRow(index)) ? m_seasonIndent : m_episodeIndent;
        opt.rect.setX(opt.rect.x() + indent - 4);
    }

    // TODO: Figure out why PE_PanelItemViewRow works on macOS (commit 3d1c37a35c52a593e78335da851f520b9151f81f)
    //       but doesn't on Linux/Windows.
    //       It could be https://stackoverflow.com/a/34646515/1603627 , but why?
#ifdef Q_OS_MAC
    // On macOS, PE_PanelItemViewItem works as well for selection background, but not for alternating colors.
    style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, painter, this);
#else
    // On Linux/Windows, PE_PanelItemViewRow does not draw selection backgrounds.
    style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, this);
#endif
}

int TvShowTreeView::drawNewIcon(QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index,
    bool isSelected,
    int branchIndent) const
{
    if (index.data(TvShowRoles::IsNew).toBool()) {
#ifdef Q_OS_WIN
        QRect iconRect(option.rect.x() + branchIndent, option.rect.y(), 18, option.rect.height());
        painter->drawPixmap(iconRect.x() + (iconRect.width() - m_newIcon.width()) / 2,
            iconRect.y() + (iconRect.height() - m_newIcon.height()) / 2,
            m_newIcon);
#else
        QRect iconRect(option.rect.x() + branchIndent, option.rect.y(), 14, option.rect.height());
        int drawSize = qRound(iconRect.width() * 1.0);
        painter->setPen(isSelected ? QColor(255, 255, 255) : QColor(58, 135, 173));
        painter->setFont(Manager::instance()->iconFont()->font(drawSize));
        painter->drawText(iconRect, QString(QChar(static_cast<uint>(icon_star))), QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
#endif
        return 20;

    } else {
        return 0;
    }
}

bool TvShowTreeView::isShowRow(const QModelIndex& index) const
{
    return TvShowType(index.model()->data(index, TvShowRoles::Type).toInt()) == TvShowType::TvShow;
}

bool TvShowTreeView::isSeasonRow(const QModelIndex& index) const
{
    return TvShowType(index.model()->data(index, TvShowRoles::Type).toInt()) == TvShowType::Season;
}

bool TvShowTreeView::isEpisodeRow(const QModelIndex& index) const
{
    return TvShowType(index.model()->data(index, TvShowRoles::Type).toInt()) == TvShowType::Episode;
}
