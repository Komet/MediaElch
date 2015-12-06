#ifndef TVSHOWTREEVIEW_H
#define TVSHOWTREEVIEW_H

#include <QPainter>
#include <QTreeView>
#include <QWidget>

class TvShowTreeView : public QTreeView
{
    Q_OBJECT
public:
    TvShowTreeView(QWidget *parent = 0);
    ~TvShowTreeView();

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QPixmap m_newIcon;
    QPixmap m_syncIcon;
    QPixmap m_missingIcon;

};

#endif // TVSHOWTREEVIEW_H
