#ifndef TVSHOWDELEGATE_H
#define TVSHOWDELEGATE_H

#include <QStyledItemDelegate>

class TvShowDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TvShowDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // TVSHOWDELEGATE_H
