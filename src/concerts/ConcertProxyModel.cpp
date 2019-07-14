#include "ConcertProxyModel.h"

#include <QDebug>

#include "globals/Filter.h"
#include "globals/Globals.h"
#include "globals/Manager.h"

/**
 * @brief ConcertProxyModel::ConcertProxyModel
 */
ConcertProxyModel::ConcertProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
}

/**
 * @brief Checks if a row accepts the filter. Checks the first two "columns" of our model (Concert name and folder name)
 * @return Filter is accepted or not
 */
bool ConcertProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    Q_UNUSED(sourceParent);
    QVector<Concert*> concerts = Manager::instance()->concertModel()->concerts();
    if (sourceRow < 0 || sourceRow >= concerts.count()) {
        return true;
    }

    Concert* concert = concerts.at(sourceRow);
    for (Filter* filter : m_filters) {
        if (!filter->accepts(concert)) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Sort function for the concert model. Sorts concerts by name and new files to top.
 */
bool ConcertProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    if (sourceModel()->data(left, Qt::UserRole + 1).toBool()
        && !sourceModel()->data(right, Qt::UserRole + 1).toBool()) {
        return true;
    }
    if (!sourceModel()->data(left, Qt::UserRole + 1).toBool()
        && sourceModel()->data(right, Qt::UserRole + 1).toBool()) {
        return false;
    }
    int cmp = QString::localeAwareCompare(sourceModel()->data(left).toString(), sourceModel()->data(right).toString());
    return !(cmp < 0);
}

/**
 * @brief Sets active filters
 */
void ConcertProxyModel::setFilter(QVector<Filter*> filters, QString text)
{
    m_filters = filters;
    m_filterText = text;
}
