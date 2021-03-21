#include "data/RatingModel.h"

#include "data/Rating.h"
#include "movies/Movie.h"

#include <QIcon>

void RatingModel::setMovie(Movie* movie)
{
    beginResetModel();
    m_movie = movie;
    endResetModel();
}

void RatingModel::addRatingToMovie(Rating rating)
{
    if (m_movie == nullptr) {
        return;
    }
    QModelIndex root{};
    beginInsertRows(root, rowCount(), rowCount());
    m_movie->ratings().setOrAddRating(std::move(rating));
    m_movie->setChanged(true);
    endInsertRows();
}

int RatingModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || m_movie == nullptr) {
        // Root has an invalid model index.
        return 0;
    }
    return m_movie->ratings().size();
}

int RatingModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        // Root has an invalid model index.
        return 0;
    }
    return 5;
}

QVariant RatingModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || m_movie == nullptr) {
        return {};
    }

    if (index.row() < 0 || index.row() >= rowCount()) {
        return {};
    }

    Rating rating = m_movie->ratings().at(index.row());

    switch (role) {
    case RatingRoles::RatingRole: return QVariant::fromValue(rating);

    case Qt::DisplayRole:
    case Qt::EditRole: {
        switch (Columns(index.column())) {
        case Columns::SourceColumn: {
            if (role == Qt::DisplayRole) {
                return Rating::sourceToName(rating.source);
            }
            return rating.source;
        }
        case Columns::RatingColumn: return rating.rating;
        case Columns::VoteCountColumn: return rating.voteCount;
        case Columns::MinRatingColumn: return rating.minRating;
        case Columns::MaxRatingColumn: return rating.maxRating;
        }
        break;
    }
    }

    return {};
}

QVariant RatingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Vertical) {
        return {};
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return {};
    }

    switch (Columns(section)) {
    case Columns::SourceColumn: return tr("Source");
    case Columns::RatingColumn: return tr("Rating");
    case Columns::VoteCountColumn: return tr("Vote Count");
    case Columns::MinRatingColumn: return tr("Min. Rating");
    case Columns::MaxRatingColumn: return tr("Max. Rating");
    }

    return {};
}

bool RatingModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid() || m_movie == nullptr) {
        // non-root element not possible in table view
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    m_movie->ratings().remove(row, count);
    m_movie->setChanged(true);
    endRemoveRows();

    return true;
}

bool RatingModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || m_movie == nullptr) {
        // root element can't be edited
        return false;
    }
    if (role != Qt::EditRole) {
        return false;
    }

    Rating ratingCopy = m_movie->ratings().at(index.row());

    bool ok = false;
    switch (index.column()) {
    case Columns::SourceColumn: {
        ratingCopy.source = value.toString();
        ok = true;
        break;
    }
    case Columns::RatingColumn: ratingCopy.rating = value.toDouble(&ok); break;
    case Columns::VoteCountColumn: ratingCopy.voteCount = value.toInt(&ok); break;
    case Columns::MinRatingColumn: ratingCopy.minRating = value.toDouble(&ok); break;
    case Columns::MaxRatingColumn: ratingCopy.maxRating = value.toDouble(&ok); break;
    }

    if (!ok) {
        return false;
    }

    m_movie->ratings()[index.row()] = ratingCopy;
    m_movie->setChanged(true);

    emit dataChanged(index, index, {role});

    return true;
}

Qt::ItemFlags RatingModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.isValid())
        f |= Qt::ItemIsEditable;
    return f;
}
