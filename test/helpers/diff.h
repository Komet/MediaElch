#pragma once

#include "data/tv_show/EpisodeMap.h"

#include <QDomDocument>
#include <QString>

class Artist;
class Album;
class Concert;
class Movie;
class TvShow;
class TvShowEpisode;

namespace test {

/// Parses a given XML string and fails using Catch2's REQUIRE macro if
/// the string can't be parsed.
QDomDocument parseXmlOrFail(const QString& content);

/// Checks whether both strings are the same. Fails otherwise.
/// If the environment variable MEDIAELCH_UPDATE_REF_FILES is set
/// the original file will be overwritten if there are differences.
///
/// \note In contrast to compareXmlAgainstResourceFile(), does not compare
///       XML sensitive.
void compareStringAgainstResourceFile(const QString& actual, const QString& filename);

/// Checks whether both XML strings are the same. Fails otherwise.
/// If the environment variable MEDIAELCH_UPDATE_REF_FILES is set
/// the original file will be overwritten if there are differences.
///
/// \note In contrast to compareXmlAgainstResourceFile(), compares
///       XML sensitive.
void compareXmlAgainstResourceFile(const QString& actual, const QString& filename);

namespace scraper {

/// Checks whether the Concert matches the reference file. Fails otherwise.
/// If the environment variable MEDIAELCH_UPDATE_REF_FILES is set
/// the reference file will be overwritten if there are differences.
///
/// Uses XML as storage format, but that is an implementation detail.
///
/// Example
/// \code{cpp} compareAgainstReference(myConcert, "scrapers/concert/my_tmdb_concert_12345"); \endcode
void compareAgainstReference(Concert& concert, QString filename);
void compareAgainstReference(Movie& movie, QString filename);
void compareAgainstReference(Album& album, QString filename);
void compareAgainstReference(Artist& artist, QString filename);
void compareAgainstReference(TvShow& show, QString filename);
void compareAgainstReference(TvShowEpisode& episode, QString filename);
void compareAgainstReference(const mediaelch::EpisodeMap& episodes, QString filename);

} // namespace scraper

} // namespace test
