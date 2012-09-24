#ifndef CONCERTDELEGATE_H
#define CONCERTDELEGATE_H

#include <QStyledItemDelegate>

/**
 * @brief The ConcertDelegate class
 * This class is used to style the items in the concert list.
 */
class ConcertDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ConcertDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // CONCERTDELEGATE_H
