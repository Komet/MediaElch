#pragma once

#include <QString>

class Artist;
class Album;
class Movie;
class TvShow;
class TvShowEpisode;

namespace test {

/// Rounds the given number magnitude aware and returns a human readable string.
/// Examples:
///  3    -> "3"
///  123  -> ">120"
///  1234 -> ">1200"
QString approxMagnitude(int number);

/// Normalize a movie for reference files.  Some fields change a lot
/// on sites like IMDb, for example, ratings change a lot (voteCount for
/// Godfather changes every few minutes).
/// This function takes the movie and modifies it in such a way, that
/// the _magnitude_ remains the same.  A rating's voteCount of 123 will
/// be rounded to 120, one of 1234 will be rounded to 1200, and so one.
void normalizeForReferenceFile(Movie& movie);
void normalizeForReferenceFile(Artist& movie);
void normalizeForReferenceFile(Album& movie);
void normalizeForReferenceFile(TvShow& movie);
void normalizeForReferenceFile(TvShowEpisode& movie);

} // namespace test
