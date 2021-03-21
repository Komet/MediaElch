#pragma once

#include <QMetaType>
#include <QString>
#include <QStringList>
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
    /// \example sourceToName("themoviedb") => "TMDb"
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

Q_DECLARE_METATYPE(Rating);
