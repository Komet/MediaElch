#ifndef TVSHOWDELEGATE_H
#define TVSHOWDELEGATE_H

#include <QStyledItemDelegate>
#include <QPixmap>

/**
 * @brief The TvShowDelegate class
 * This class is used to style the items in the tv show list.
 */
class TvShowDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TvShowDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    int m_showItemHeight;
};

#endif // TVSHOWDELEGATE_H
