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
    bool hasFeature(MovieScraperInterface& scraper, MovieScraperInfos feature);
    TableLayout createTableLayout();

    std::ostream& m_out;
    QMap<MovieScraperInfos, QString> m_featureMap = {{MovieScraperInfos::Title, "Title"},
        {MovieScraperInfos::Tagline, "Tagline"},
        {MovieScraperInfos::Rating, "Rating"},
        {MovieScraperInfos::Released, "Release Date"},
        {MovieScraperInfos::Runtime, "Runtime"},
        {MovieScraperInfos::Certification, "Certification"},
        {MovieScraperInfos::Trailer, "Trailer"},
        {MovieScraperInfos::Overview, "Overview"},
        {MovieScraperInfos::Poster, "Poster"},
        {MovieScraperInfos::Backdrop, "Backdrop"},
        {MovieScraperInfos::Actors, "Actors"},
        {MovieScraperInfos::Genres, "Genres"},
        {MovieScraperInfos::Studios, "Studios"},
        {MovieScraperInfos::Countries, "Countries"},
        {MovieScraperInfos::Writer, "Writer"},
        {MovieScraperInfos::Director, "Director"},
        {MovieScraperInfos::Tags, "Tags"},
        {MovieScraperInfos::ExtraFanarts, "Extra Fanarts"},
        {MovieScraperInfos::Set, "Set"},
        {MovieScraperInfos::Logo, "Logo"},
        {MovieScraperInfos::CdArt, "CdArt"},
        {MovieScraperInfos::ClearArt, "ClearArt"},
        {MovieScraperInfos::Banner, "Banner"},
        {MovieScraperInfos::Thumb, "Thumb"}};
};


} // namespace cli
} // namespace mediaelch
