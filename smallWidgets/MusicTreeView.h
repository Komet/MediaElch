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
    ~MusicTreeView();

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // MUSICTREEVIEW_H
