#pragma once

#include "data/Rating.h"

#include <QAbstractTableModel>

class Movie;

class RatingModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum RatingRoles
    {
        RatingRole = Qt::UserRole + 1
    };

    enum Columns
    {
        SourceColumn = 0,
        RatingColumn = 1,
        VoteCountColumn = 2,
        MinRatingColumn = 3,
        MaxRatingColumn = 4
    };

public:
    RatingModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {}
    ~RatingModel() override = default;

    void setRatings(Ratings* ratings);
    void addRating(Rating rating);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;

    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool removeRows(int row, int count, const QModelIndex& parent = {}) override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    static QString sourceToName(const QString& source);

private:
    Ratings* m_ratings = nullptr;
};
