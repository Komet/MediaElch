#pragma once

#include "export/TableWriter.h"
#include "globals/Manager.h"

#include <ostream>

namespace mediaelch {
namespace cli {

class MovieScraperFeatureTable
{
public:
    MovieScraperFeatureTable(std::ostream& out) : m_out{out} {}

    void print();

private:
    bool hasFeature(scraper::MovieScraper& scraper, MovieScraperInfo feature);
    TableLayout createTableLayout();

    std::ostream& m_out;
    QMap<MovieScraperInfo, QString> m_featureMap = {{MovieScraperInfo::Title, "Title"},
        {MovieScraperInfo::Tagline, "Tagline"},
        {MovieScraperInfo::Rating, "Rating"},
        {MovieScraperInfo::Released, "Release Date"},
        {MovieScraperInfo::Runtime, "Runtime"},
        {MovieScraperInfo::Certification, "Certification"},
        {MovieScraperInfo::Trailer, "Trailer"},
        {MovieScraperInfo::Overview, "Overview"},
        {MovieScraperInfo::Poster, "Poster"},
        {MovieScraperInfo::Backdrop, "Backdrop"},
        {MovieScraperInfo::Actors, "Actors"},
        {MovieScraperInfo::Genres, "Genres"},
        {MovieScraperInfo::Studios, "Studios"},
        {MovieScraperInfo::Countries, "Countries"},
        {MovieScraperInfo::Writer, "Writer"},
        {MovieScraperInfo::Director, "Director"},
        {MovieScraperInfo::Tags, "Tags"},
        {MovieScraperInfo::ExtraFanarts, "Extra Fanarts"},
        {MovieScraperInfo::Set, "Set"},
        {MovieScraperInfo::Logo, "Logo"},
        {MovieScraperInfo::CdArt, "CdArt"},
        {MovieScraperInfo::ClearArt, "ClearArt"},
        {MovieScraperInfo::Banner, "Banner"},
        {MovieScraperInfo::Thumb, "Thumb"}};
};


} // namespace cli
} // namespace mediaelch
