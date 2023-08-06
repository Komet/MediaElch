#pragma once

#include "utils/Meta.h"

#include <QPainter>
#include <QPixmap>
#include <QTreeView>
#include <QWidget>

class MusicTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit MusicTreeView(QWidget* parent = nullptr);
    ~MusicTreeView() override = default;

protected:
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    void drawBranches(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QRect& rect,
        const QModelIndex& index) const;

    void drawArtistRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void drawAlbumRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
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

    ELCH_NODISCARD bool isAlbumRow(const QModelIndex& index) const;
    ELCH_NODISCARD bool isArtistRow(const QModelIndex& index) const;

private:
    const QPixmap m_newIcon;

    const int m_albumIndent{40};
    const int m_branchIndent{30};
};
