#include "test/test_helpers.h"

#include "media_center/kodi/MovieSetArtworkFileNames.h"

using namespace mediaelch::kodi;

namespace {

/// \brief Renders a candidate list as "<fileName>|<legalised>" so an ordering mismatch shows up as
///        a readable diff instead of a bare "false".
QStringList describe(const QVector<MovieSetArtworkCandidate>& candidates)
{
    QStringList described;
    for (const MovieSetArtworkCandidate& candidate : candidates) {
        described << candidate.fileName + (candidate.legalise == LegalisePath::Yes ? "|legal" : "|verbatim");
    }
    return described;
}

} // namespace

TEST_CASE("kodi::legacyMovieSetFileNames", "[media_center][kodi][movie_set]")
{
    SECTION("the poster's legacy spelling is layout dependent")
    {
        CHECK(legacyMovieSetFileNames(DataFileType::MovieSetPoster, MovieSetArtworkType::SeparateArtworkFolder, {})
              == QStringList{"folder.jpg"});
        CHECK(legacyMovieSetFileNames(DataFileType::MovieSetPoster, MovieSetArtworkType::ArtworkNextToMovies, {})
              == QStringList{"<setName>-folder.jpg"});
    }

    // Term (c): fanart has only ever had one spelling in either layout.  Kills a fallback that
    // invents a legacy name for every type.
    SECTION("fanart has no legacy spelling in either layout")
    {
        CHECK(legacyMovieSetFileNames(DataFileType::MovieSetBackdrop, MovieSetArtworkType::SeparateArtworkFolder, {})
                .isEmpty());
        CHECK(legacyMovieSetFileNames(DataFileType::MovieSetBackdrop, MovieSetArtworkType::ArtworkNextToMovies, {})
                .isEmpty());
    }

    SECTION("types that are not movie set artwork have no legacy spelling")
    {
        CHECK(legacyMovieSetFileNames(DataFileType::MoviePoster, MovieSetArtworkType::SeparateArtworkFolder, {})
                .isEmpty());
        CHECK(legacyMovieSetFileNames(DataFileType::NoType, MovieSetArtworkType::ArtworkNextToMovies, {}).isEmpty());
    }

    // Term (d): "folder.jpg" has never been a shipped default, so it is a user's configured name
    // only if they picked a movie set artwork layout under a MediaElch that still suggested it.
    // That path must not be looked at twice.  Kills the removal of the skip condition.
    SECTION("a legacy name that is still configured is dropped")
    {
        CHECK(legacyMovieSetFileNames(
            DataFileType::MovieSetPoster, MovieSetArtworkType::SeparateArtworkFolder, {"folder.jpg"})
                .isEmpty());
        CHECK(legacyMovieSetFileNames(
            DataFileType::MovieSetPoster, MovieSetArtworkType::ArtworkNextToMovies, {"<setName>-folder.jpg"})
                .isEmpty());
        // ... but an unrelated configured name does not suppress it.
        CHECK(legacyMovieSetFileNames(
                  DataFileType::MovieSetPoster, MovieSetArtworkType::SeparateArtworkFolder, {"poster.jpg"})
              == QStringList{"folder.jpg"});
    }

    // Windows and macOS would resolve both spellings to the same file, so emitting the legacy name
    // beside a differently-cased configured one would only cost a redundant stat().  Kills dropping
    // the Qt::CaseInsensitive argument.
    SECTION("the comparison against configured names ignores case")
    {
        CHECK(legacyMovieSetFileNames(
            DataFileType::MovieSetPoster, MovieSetArtworkType::SeparateArtworkFolder, {"Folder.JPG"})
                .isEmpty());
        CHECK(legacyMovieSetFileNames(
            DataFileType::MovieSetPoster, MovieSetArtworkType::ArtworkNextToMovies, {"<setName>-FOLDER.jpg"})
                .isEmpty());
    }
}

TEST_CASE("kodi::movieSetArtworkCandidates", "[media_center][kodi][movie_set]")
{
    // Terms (a) and (b) at list level: the legacy spelling is looked for at all, and the
    // configured one is tried before it on both folder-name axes.  Kills a swap of the two
    // groups, and kills deleting the fallback.
    SECTION("configured names come before legacy names in the movie set information folder")
    {
        const QStringList candidates = describe(movieSetArtworkCandidates(
            DataFileType::MovieSetPoster, MovieSetArtworkType::SeparateArtworkFolder, {"poster.jpg"}));
        CHECK(candidates
              == QStringList{"poster.jpg|legal", "poster.jpg|verbatim", "folder.jpg|legal", "folder.jpg|verbatim"});
    }

    // Term (a) again for the other layout, which has no folder component to legalise.
    SECTION("next to the movies there is no verbatim folder axis")
    {
        const QStringList candidates = describe(movieSetArtworkCandidates(
            DataFileType::MovieSetPoster, MovieSetArtworkType::ArtworkNextToMovies, {"<setName>-poster.jpg"}));
        CHECK(candidates == QStringList{"<setName>-poster.jpg|legal", "<setName>-folder.jpg|legal"});
    }

    // Term (d) at list level: one attempt per folder axis, not two.
    SECTION("a configured name equal to the legacy name is not tried twice")
    {
        const QStringList candidates = describe(movieSetArtworkCandidates(
            DataFileType::MovieSetPoster, MovieSetArtworkType::SeparateArtworkFolder, {"folder.jpg"}));
        CHECK(candidates == QStringList{"folder.jpg|legal", "folder.jpg|verbatim"});
    }

    SECTION("fanart gets the configured names and nothing else")
    {
        CHECK(describe(movieSetArtworkCandidates(
                  DataFileType::MovieSetBackdrop, MovieSetArtworkType::SeparateArtworkFolder, {"fanart.jpg"}))
              == QStringList{"fanart.jpg|legal", "fanart.jpg|verbatim"});
        CHECK(describe(movieSetArtworkCandidates(
                  DataFileType::MovieSetBackdrop, MovieSetArtworkType::ArtworkNextToMovies, {"<setName>-fanart.jpg"}))
              == QStringList{"<setName>-fanart.jpg|legal"});
    }

    SECTION("every configured name keeps its order and is tried before any legacy name")
    {
        const QStringList candidates = describe(movieSetArtworkCandidates(
            DataFileType::MovieSetPoster, MovieSetArtworkType::SeparateArtworkFolder, {"poster.jpg", "cover.jpg"}));
        CHECK(candidates
              == QStringList{"poster.jpg|legal",
                  "cover.jpg|legal",
                  "poster.jpg|verbatim",
                  "cover.jpg|verbatim",
                  "folder.jpg|legal",
                  "folder.jpg|verbatim"});
    }

    SECTION("no configured names still yields the legacy ones")
    {
        CHECK(describe(movieSetArtworkCandidates(
                  DataFileType::MovieSetPoster, MovieSetArtworkType::SeparateArtworkFolder, {}))
              == QStringList{"folder.jpg|legal", "folder.jpg|verbatim"});
    }
}
