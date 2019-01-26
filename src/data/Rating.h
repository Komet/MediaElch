#pragma once

class Rating
{
public:
    Rating();

    int voteCount{0};
    double rating{0};
    double maxRating{0.0};
    double minRating{0.0};
    int imdbTop250{0};
};
