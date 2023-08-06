#pragma once

#include "utils/Meta.h"

#include <QPainter>
#include <QPixmap>
#include <QTreeView>
#include <QWidget>

class TvShowTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit TvShowTreeView(QWidget* parent = nullptr);
    ~TvShowTreeView() override = default;

protected:
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    void drawBranches(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QRect& rect,
        const QModelIndex& index) const;

    void drawTvShowRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void drawTvShowIcons(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void drawEpisodeRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void drawRowBackground(QPainter* painter, QStyleOptionViewItem opt, const QModelIndex& index) const;

    /// \brief   Draw the "is new" icon.
    /// \details On Windows image files are drawn whereas on mac and linux FontAwesome
    ///          icons are used.
    /// \return  Item indent, based on whether there is a "is new" icon or not.
    ELCH_NODISCARD int drawNewIcon(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index,
        bool isSelected,
        int branchIndent) const;

    ELCH_NODISCARD bool isShowRow(const QModelIndex& index) const;
    ELCH_NODISCARD bool isSeasonRow(const QModelIndex& index) const;
    ELCH_NODISCARD bool isEpisodeRow(const QModelIndex& index) const;

private:
    const QPixmap m_newIcon;
    const QPixmap m_syncIcon;
    const QPixmap m_missingIcon;

    const int m_seasonIndent{30};
    const int m_episodeIndent{50};
    const int m_branchIndent{30};
};
