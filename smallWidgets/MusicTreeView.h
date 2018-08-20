#ifndef MUSICTREEVIEW_H
#define MUSICTREEVIEW_H

#include <QPainter>
#include <QTreeView>
#include <QWidget>

class MusicTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit MusicTreeView(QWidget *parent = nullptr);
    ~MusicTreeView() override = default;

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int m_albumIndent{40};
    int m_branchIndent{30};

    void drawArtistRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawAlbumRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    int drawNewIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, int branchIndent) const;
};

#endif // MUSICTREEVIEW_H
