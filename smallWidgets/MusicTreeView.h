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
    ~MusicTreeView() override;

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // MUSICTREEVIEW_H
