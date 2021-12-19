#pragma once

#include <QDate>
#include <QString>

class Movie;

namespace mediaelch {
namespace scraper {

class ImdbReferencePage
{
public:
    /// Extract the release date from the given reference page.
    /// If no release date can be extracted, an invalid QDate is returned.
    static QDate extractReleaseDate(const QString& html);

    static QString extractTitle(const QString& html);
    static QString extractOriginalTitle(const QString& html);

    static void extractStudios(Movie* movie, const QString& html);
    static void extractDirectors(Movie* movie, const QString& html);
    static void extractWriters(Movie* movie, const QString& html);
    static void extractCertification(Movie* movie, const QString& html);
    static void extractGenres(Movie* movie, const QString& html);
    static void extractRating(Movie* movie, const QString& html);
    static void extractOverview(Movie* movie, const QString& html);
    static void extractTaglines(Movie* movie, const QString& html);
    static void extractTags(Movie* movie, const QString& html);
    static void extractCountries(Movie* movie, const QString& html);
};

} // namespace scraper
} // namespace mediaelch
