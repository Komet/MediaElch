#include "test/helpers/normalize.h"

#include "data/movie/Movie.h"
#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "data/tv_show/TvShow.h"
#include "data/tv_show/TvShowEpisode.h"

/// Rounds the given number magnitude aware. Examples:
///  3    -> 3
///  123  -> 120
///  1234 -> 1200
static int roundToMagnitude(int number)
{
    if (number <= 10) {
        return number;
    } else if (number <= 100) {
        return number - (number % 10);
    } else if (number <= 1000) {
        return number - (number % 100);
    } else if (number <= 10000) {
        return number - (number % 1000);
    } else if (number <= 100000) {
        return number - (number % 10000);
    } else {
        return number - (number % 100000);
    }
}

static double roundCommaFirstDigit(double number)
{
    // We only want one digit, and only even ones, e.g. 7.2, 7.8, but not 7.3.
    auto asInt = static_cast<long long int>(number * 10);
    asInt = asInt - (asInt % 2);
    return static_cast<double>(asInt) / 10.;
}

namespace test {

void normalizeForReferenceFile(Movie& movie)
{
    for (auto& rating : movie.ratings()) {
        rating.rating = roundCommaFirstDigit(rating.rating);
        rating.voteCount = roundToMagnitude(rating.voteCount);
    }
}

void normalizeForReferenceFile(Artist& artist)
{
    Q_UNUSED(artist);
    // no-op
}

void normalizeForReferenceFile(Album& album)
{
    Q_UNUSED(album);
    // no-op
}

void normalizeForReferenceFile(TvShow& movie)
{
    for (auto& rating : movie.ratings()) {
        rating.rating = roundCommaFirstDigit(rating.rating);
        rating.voteCount = roundToMagnitude(rating.voteCount);
    }
}

void normalizeForReferenceFile(TvShowEpisode& movie)
{
    for (auto& rating : movie.ratings()) {
        rating.rating = roundCommaFirstDigit(rating.rating);
        rating.voteCount = roundToMagnitude(rating.voteCount);
    }
}

} // namespace test
