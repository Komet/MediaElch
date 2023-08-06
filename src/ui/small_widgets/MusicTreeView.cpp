#include "MusicTreeView.h"

#include "log/Log.h"
#include "model/music/MusicModelRoles.h"

#include "globals/Globals.h"
#include "globals/Manager.h"

MusicTreeView::MusicTreeView(QWidget* parent) :
    QTreeView(parent),
    m_newIcon{QPixmap(":/img/star_blue.png").scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation)}
{
    setAllColumnsShowFocus(true);
}

void MusicTreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(index)
    return; // no-op
}


void MusicTreeView::drawBranches(QPainter* painter,
    const QStyleOptionViewItem& option,
    const QRect& rect,
    const QModelIndex& index) const
{
    if (isAlbumRow(index)) {
        return;
    }

    const bool isSelected = option.state.testFlag(QStyle::State_Selected);
    const QPalette::ColorRole textColorRole = isSelected ? QPalette::HighlightedText : QPalette::Text;
    QColor textColor = option.palette.color(textColorRole);

    const int drawSize = qRound(rect.height() * 0.85);
    QString text = isExpanded(index) ? QChar(icon_angle_down) : QChar(icon_angle_right);

    painter->save();
    painter->setPen(textColor);
    painter->setFont(Manager::instance()->iconFont()->font(drawSize));
    painter->drawText(rect, text, QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
    painter->restore();
}

void MusicTreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
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

    if (isArtistRow(index)) {
        drawArtistRow(painter, opt, index);

    } else {
        drawAlbumRow(painter, opt, index);
    }

    painter->restore();
}

void MusicTreeView::drawArtistRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;

    QColor red(255, 0, 0);
    const bool isSelected = opt.state.testFlag(QStyle::State_Selected);
    const bool hasChanged = index.data(mediaelch::MusicRoles::HasChanged).toBool();

    const QPalette::ColorRole textColorRole = isSelected ? QPalette::HighlightedText : QPalette::Text;
    QColor textColor = (hasChanged && !isSelected) ? red : opt.palette.color(textColorRole);
    QFont textFont = painter->font();
    textFont.setItalic(hasChanged);
    textFont.setBold(true);


    QRect branches(option.rect.x() + 5, option.rect.y() + 5, 20, option.rect.height() - 10);
    drawBranches(painter, opt, branches, index);

    const int rowPadding = 4;
    const int itemIndent = m_branchIndent + drawNewIcon(painter, option, index, isSelected, m_branchIndent);

    const int textRowHeight = (option.rect.height() - 2 * rowPadding) / 2;
    const int textRowWidth = option.rect.width() - itemIndent;
    const int posX = option.rect.x() + itemIndent;
    const int posY = option.rect.y() + rowPadding;

    QRect artistRect(posX, posY + 1, textRowWidth, textRowHeight);
    QRect albumsRect(posX, posY + textRowHeight, textRowWidth, textRowHeight);

    painter->setFont(textFont);
    painter->setPen(textColor);

    QFontMetrics metrics(textFont);
    const QString itemStr = metrics.elidedText(index.data().toString(), Qt::ElideRight, artistRect.width());

    style()->drawItemText(painter, artistRect, (Qt::AlignLeft | Qt::AlignVCenter), opt.palette, true, itemStr);

#ifdef Q_OS_MAC
    textFont.setPointSize(textFont.pointSize() - 2);
#else
    textFont.setPointSize(textFont.pointSize() - 1);
#endif
    textFont.setBold(false);

    metrics = QFontMetrics(textFont);
    const QString albumStr =
        metrics.elidedText(tr("%n albums", "", index.data(mediaelch::MusicRoles::NumOfAlbums).toInt()),
            Qt::ElideRight,
            albumsRect.width());

    painter->setFont(textFont);
    painter->setPen(textColor);
    style()->drawItemText(painter, albumsRect, (Qt::AlignLeft | Qt::AlignVCenter), opt.palette, true, albumStr);

    QPoint lineStart(option.rect.x(), option.rect.y());
    QPoint lineEnd(option.rect.x() + option.rect.width() - 1, option.rect.y());
    painter->setPen(QColor(220, 220, 220));
    painter->drawLine(lineStart, lineEnd);
}


void MusicTreeView::drawAlbumRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QColor red(255, 0, 0);
    const bool isSelected = option.state.testFlag(QStyle::State_Selected);
    const bool hasChanged = index.data(TvShowRoles::HasChanged).toBool();
    const QPalette::ColorRole textColorRole = isSelected ? QPalette::HighlightedText : QPalette::Text;

    QColor textColor = (hasChanged && !isSelected) ? red : option.palette.color(textColorRole);
    QFont textFont = painter->font();
    painter->setPen(textColor);
    textFont.setItalic(hasChanged);

    const int itemIndent = m_albumIndent + drawNewIcon(painter, option, index, isSelected, m_albumIndent);

    QRect itemRect(
        option.rect.x() + itemIndent, option.rect.y(), option.rect.width() - itemIndent, option.rect.height() - 1);
    const QFontMetrics metrics(textFont);
    const QString itemStr = metrics.elidedText(index.data().toString(), Qt::ElideRight, itemRect.width());

    painter->setFont(textFont);
    painter->setPen(textColor);
    style()->drawItemText(painter, itemRect, (Qt::AlignLeft | Qt::AlignVCenter), option.palette, true, itemStr);
}

void MusicTreeView::drawRowBackground(QPainter* painter, QStyleOptionViewItem opt, const QModelIndex& index) const
{
    if (!isArtistRow(index)) {
        const int indent = m_albumIndent;
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

/// \brief Draw the "is new" icon.
/// \return Additional row indent
int MusicTreeView::drawNewIcon(QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index,
    bool isSelected,
    int branchIndent) const
{
    if (index.data(mediaelch::MusicRoles::IsNew).toBool()) {
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
        painter->drawText(iconRect, QString(QChar(icon_star)), QTextOption(Qt::AlignCenter | Qt::AlignVCenter));
#endif
        return 20;

    } else {
        return 0;
    }
}

bool MusicTreeView::isAlbumRow(const QModelIndex& index) const
{
    using namespace mediaelch;
    return MusicType(index.data(MusicRoles::Type).toInt()) == MusicType::Album;
}

bool MusicTreeView::isArtistRow(const QModelIndex& index) const
{
    using namespace mediaelch;
    return MusicType(index.data(MusicRoles::Type).toInt()) == MusicType::Artist;
}
