#pragma once

#include "utils/Meta.h"

#include <QMetaType>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <tuple>

class Rating
{
public:
    Rating();

    int voteCount{0};
    double rating{0};
    double maxRating{10.0};
    double minRating{0.0};
    QString source = "default";

public:
    /// \brief Returns a more readable representation of the source string.
    ///
    /// \details Example
    ///   \code{cpp}
    ///   QString name = Rating::sourceToName("themoviedb");
    ///   // name == "TMDB"
    ///   \endcode
    static QString sourceToName(const QString& source);
    static QStringList commonSources();

public:
    // Equality operator not yet implemented:
    // Should we only compare the rating or also vote count
    // and source? Implement when necessary.

    bool operator<(const Rating& rhs) const { return rating < rhs.rating; }
    bool operator<=(const Rating& rhs) const { return rating <= rhs.rating; }
    bool operator>(const Rating& rhs) const { return rating > rhs.rating; }
    bool operator>=(const Rating& rhs) const { return rating >= rhs.rating; }
};

Q_DECLARE_METATYPE(Rating)

class Ratings : public QObject
{
    Q_OBJECT
public:
    Ratings(QObject* parent = nullptr) : QObject(parent) {}

    /// \brief Sets the rating for the given rating source or adds it if it does not exist.
    void setOrAddRating(const Rating& rating);
    void addRating(const Rating& rating);
    bool hasRating() const { return !m_ratings.isEmpty(); }
    bool hasSource(const QString& source) const;

    void merge(const Ratings& ratings);

    // For compatibility to QVector<Rating>

    /// \todo Remove once multiple ratings are supported everywhere
    const Rating& first() const { return m_ratings.first(); }
    Rating& first() { return m_ratings.first(); }

    void remove(int row, int count) { return m_ratings.remove(row, count); }
    void clear() { m_ratings.clear(); }

    const Rating& at(int index) { return m_ratings.at(index); }
    Rating& operator[](int index) { return m_ratings[index]; }

    bool isEmpty() const { return m_ratings.isEmpty(); }

    // For STL container compatibility

    elch_ssize_t size() const { return m_ratings.size(); }
    auto begin() { return m_ratings.begin(); }
    auto end() { return m_ratings.end(); }
    auto begin() const { return m_ratings.begin(); }
    auto end() const { return m_ratings.end(); }
    auto cbegin() const { return m_ratings.cbegin(); }
    auto cend() const { return m_ratings.cend(); }

private:
    QVector<Rating> m_ratings;
};
