#include "test/test_helpers.h"

#include "media_center/kodi/MovieXmlReader.h"
#include "src/scrapers/movie/MovieMerger.h"
#include "test/helpers/fake_data.h"
#include "test/helpers/resource_dir.h"

TEST_CASE("movies are correctly merged", "[movie][merger]")
{
    using namespace mediaelch::scraper;

    std::unique_ptr<Movie> original = test::movieWithAllDetails();

    SECTION("All details are copied")
    {
        Movie copy;
        copyDetailsToMovie(copy, *original, allMovieScraperInfos(), true, false);

        CHECK(copy.title() == original->title());
        CHECK(copy.originalTitle() == original->originalTitle());
        CHECK(copy.overview() == original->overview());
        CHECK(copy.outline() == original->outline());
    }

    SECTION("Uses plot for outline if original has not outline")
    {
        Movie copy;
        original->setOutline("");
        copyDetailsToMovie(copy, *original, allMovieScraperInfos(), true, true);

        CHECK(copy.title() == original->title());
        CHECK(copy.originalTitle() == original->originalTitle());
        CHECK(copy.overview() == original->overview());
        CHECK(copy.outline() == original->overview()); // !
    }

    SECTION("Does not use plot for outline if not requested")
    {
        Movie copy;
        original->setOutline("");
        copyDetailsToMovie(copy, *original, allMovieScraperInfos(), false, true);

        CHECK(copy.title() == original->title());
        CHECK(copy.originalTitle() == original->originalTitle());
        CHECK(copy.overview() == original->overview());
        CHECK(copy.outline().isEmpty()); // !
    }

    SECTION("Does not copy original title if it's the same")
    {
        Movie copy;
        original->setOriginalTitle(original->title());
        copyDetailsToMovie(copy, *original, allMovieScraperInfos(), true, true);

        CHECK(copy.title() == original->title());
        CHECK(copy.originalTitle().isEmpty());
        CHECK(copy.overview() == original->overview());
        CHECK(copy.outline() == original->outline());
    }
}
