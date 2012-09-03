#ifndef MOVIEDELEGATE_H
#define MOVIEDELEGATE_H

#include <QStyledItemDelegate>

/**
 * @brief The MovieDelegate class
 * This class is used to style the items in the movie list.
 */
class MovieDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit MovieDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // MOVIEDELEGATE_H
