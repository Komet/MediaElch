#pragma once

#include "globals/Globals.h"

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
    void drawTvShowRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void drawTvShowIcons(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void drawEpisodeRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void drawRowBackground(QPainter* painter, QStyleOptionViewItem option, const QModelIndex& index) const;
    TvShowType getTvShowType(const QModelIndex& index) const;

    const QPixmap m_newIcon;
    const QPixmap m_syncIcon;
    const QPixmap m_missingIcon;

    const int m_seasonIndent = 30;
    const int m_episodeIndent = 50;
    const int m_branchIndent = 30;
};
