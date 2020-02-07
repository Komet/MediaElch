#pragma once

#include <QString>
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
    // Equality operator not yet implemented:
    // Should we only compare the rating or also vote count
    // and source? Implement when necessary.

    bool operator<(const Rating& rhs) const { return rating < rhs.rating; }
    bool operator<=(const Rating& rhs) const { return rating <= rhs.rating; }
    bool operator>(const Rating& rhs) const { return rating > rhs.rating; }
    bool operator>=(const Rating& rhs) const { return rating >= rhs.rating; }
};
