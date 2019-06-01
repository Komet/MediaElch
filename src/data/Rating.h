#pragma once

#include <QString>

class Rating
{
public:
    Rating();

    int voteCount{0};
    double rating{0};
    double maxRating{0.0};
    double minRating{0.0};
    QString source = "default";
};
