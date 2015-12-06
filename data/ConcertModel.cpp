#include "ConcertModel.h"

#include <QPainter>
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"

/**
 * @brief ConcertModel::ConcertModel
 * @param parent
 */
ConcertModel::ConcertModel(QObject *parent) :
    QAbstractItemModel(parent)
{
#ifdef Q_OS_WIN
    m_newIcon = QIcon(":/img/star_blue.png");
    m_syncIcon = QIcon(":/img/reload_orange.png");
#else
    MyIconFont *font = new MyIconFont(this);
    font->initFontAwesome();
    m_syncIcon = font->icon("refresh_cloud", QColor(248, 148, 6), QColor(255, 255, 255), "", 0, 1.0);
    m_newIcon = font->icon("star", QColor(58, 135, 173), QColor(255, 255, 255), "", 0, 1.0);
#endif

}

/**
 * @brief Adds a concert to the model
 * @param concert Concert to add
 */
void ConcertModel::addConcert(Concert *concert)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_concerts.append(concert);
    endInsertRows();
    connect(concert, SIGNAL(sigChanged(Concert*)), this, SLOT(onConcertChanged(Concert*)), Qt::UniqueConnection);
}

/**
 * @brief Called when a concerts data has changed
 * Emits dataChanged
 * @param concert Concert which has changed
 */
void ConcertModel::onConcertChanged(Concert *concert)
{
    QModelIndex index = createIndex(m_concerts.indexOf(concert), 0);
    emit dataChanged(index, index);
}

void ConcertModel::update()
{
    QModelIndex index = createIndex(0, 0);
    emit dataChanged(index, index);
}

/**
 * @brief Get a specific concert
 * @param row Row of the concert
 * @return Concert object
 */
Concert *ConcertModel::concert(int row)
{
    if (row < 0 || row >= m_concerts.count())
        return 0;
    return m_concerts.at(row);
}

/**
 * @brief Returns the rowcount in our model. (=number of movies)
 * @param parent
 * @return Number of rows (=number of movies)
 */
int ConcertModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_concerts.size();
}

/**
 * @brief Get the column count of our model
 * @param parent
 * @return 1
 */
int ConcertModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    // return roleNames().size();
    return 1;
}

/**
 * @brief Accesses items data
 * @param index Index of item
 * @param role Role
 * @return data
 */
QVariant ConcertModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > m_concerts.count())
        return QVariant();

    if (role == Qt::UserRole)
        return index.row();

    Concert *concert = m_concerts[index.row()];
    if (index.column() == 0 && role == Qt::DisplayRole) {
        return Helper::instance()->appendArticle(concert->name());
    } else if (index.column() == 0 && (role == Qt::ToolTipRole || role == Qt::UserRole+4)) {
        if (concert->files().size() == 0)
            return QVariant();
        return concert->files().at(0);
    } else if (index.column() == 1 && role == Qt::DisplayRole) {
        return concert->folderName();
    } else if (role == Qt::UserRole+1) {
        return concert->controller()->infoLoaded();
    } else if (role == Qt::UserRole+2) {
        return concert->hasChanged();
    } else if (role == Qt::UserRole+3) {
        return concert->syncNeeded();
    /*
    } else if (role == Qt::ForegroundRole) {
        if (concert->hasChanged())
            return QColor(255, 0, 0);
    */
    } else if (role == Qt::FontRole) {
        if (concert->hasChanged()) {
            QFont font;
            font.setItalic(true);
            return font;
        }
    } else if (role == Qt::DecorationRole) {
        if (!concert->controller()->infoLoaded())
            return m_newIcon;
        else if (concert->syncNeeded())
            return m_syncIcon;
    }
    return QVariant();
}

/**
 * @brief Returns an empty modelindex because no item has a parent
 * @param child Childindex
 * @return Modelindex of the parent item
 */
QModelIndex ConcertModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

/**
 * @brief Returns a modelindex for the given row and column
 * Parent is not used because our concert model uses only one column.
 * @param row Row of the item
 * @param column Column of the item
 * @param parent Parent modelindex
 * @return Index of the item
 */
QModelIndex ConcertModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column);
}

/**
 * @brief Clears the current contents
 */
void ConcertModel::clear()
{
    if (m_concerts.size() == 0)
        return;
    beginRemoveRows(QModelIndex(), 0, m_concerts.size()-1);
    foreach (Concert *concert, m_concerts)
        delete concert;
    m_concerts.clear();
    endRemoveRows();
}

/**
 * @brief Returns a list of all concerts
 * @return List of concerts
 */
QList<Concert*> ConcertModel::concerts()
{
    return m_concerts;
}

/**
 * @brief Checks if there are new concerts (concerts where infoLoaded is false)
 * @return True if there are new concerts
 */
int ConcertModel::hasNewConcerts()
{
    int newConcerts = 0;
    foreach (Concert *concert, m_concerts) {
        if (!concert->controller()->infoLoaded())
            newConcerts++;
    }
    return newConcerts;
}
