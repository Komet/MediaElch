#include "model/RatingModel.h"

#include "data/Rating.h"
#include "data/movie/Movie.h"

#include <QIcon>

void RatingModel::setRatings(Ratings* ratings)
{
    if (ratings != nullptr) {
        MediaElch_Expects(ratings->size() < std::numeric_limits<int>::max());
    }
    beginResetModel();
    m_ratings = ratings;
    endResetModel();
}

void RatingModel::addRating(Rating rating)
{
    if (m_ratings == nullptr) {
        return;
    }
    MediaElch_Expects(rowCount() < std::numeric_limits<int>::max() - 1);
    QModelIndex root{};
    beginInsertRows(root, rowCount(), rowCount());
    m_ratings->addRating(std::move(rating));
    endInsertRows();
}

int RatingModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || m_ratings == nullptr) {
        // Root has an invalid model index.
        return 0;
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return m_ratings->size();
#else
    // Static cast is safe here, as the number of elements is limited in "addRating" and "setRatings
    return static_cast<int>(m_ratings->size());
#endif
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
    if (!index.isValid() || m_ratings == nullptr) {
        return {};
    }

    if (index.row() < 0 || index.row() >= rowCount()) {
        return {};
    }

    Rating rating = m_ratings->at(index.row());

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
    if (parent.isValid() || m_ratings == nullptr) {
        // non-root element not possible in table view
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    m_ratings->remove(row, count);
    endRemoveRows();

    return true;
}

bool RatingModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || m_ratings == nullptr) {
        // root element can't be edited
        return false;
    }
    if (role != Qt::EditRole) {
        return false;
    }

    Rating ratingCopy = m_ratings->at(index.row());

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

    (*m_ratings)[index.row()] = ratingCopy;

    emit dataChanged(index, index, {role});

    return true;
}

Qt::ItemFlags RatingModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.isValid()) {
        f |= Qt::ItemIsEditable;
    }
    return f;
}
