#ifndef DATA_RATING_H
#define DATA_RATING_H


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

#endif // DATA_RATING_H
