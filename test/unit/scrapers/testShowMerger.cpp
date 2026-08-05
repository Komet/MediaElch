#include "test/test_helpers.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/tv_show/ShowMerger.h"

TEST_CASE("TV shows are correctly merged", "[show][merger]")
{
    using namespace mediaelch;
    using namespace mediaelch::scraper;

    TvShow original;
    original.setTitle("Scrubs");
    original.setOriginalTitle("Scrubs");
    original.setEnglishTitle("Scrubs");
    original.setOverview("Medical comedy.");

    SECTION("All details are copied")
    {
        TvShow copy;
        copyDetailsToShow(copy, original, allShowScraperInfos());

        CHECK(copy.title() == original.title());
        CHECK(copy.originalTitle() == original.originalTitle());
        CHECK(copy.englishTitle() == original.englishTitle());
        CHECK(copy.overview() == original.overview());
    }

    SECTION("Overwrites english title when Title is requested and source has a value")
    {
        TvShow copy;
        copy.setEnglishTitle("Manual English Title");
        copyDetailsToShow(copy, original, {ShowScraperInfo::Title});

        CHECK(copy.title() == original.title());
        CHECK(copy.englishTitle() == original.englishTitle());
    }

    SECTION("Keeps english title when source english title is empty")
    {
        TvShow copy;
        copy.setEnglishTitle("Manual English Title");
        original.setEnglishTitle("");
        copyDetailsToShow(copy, original, {ShowScraperInfo::Title});

        CHECK(copy.englishTitle() == "Manual English Title");
    }

    SECTION("Does not copy english title if Title is not requested")
    {
        TvShow copy;
        copy.setEnglishTitle("Manual English Title");
        auto infos = allShowScraperInfos();
        infos.remove(ShowScraperInfo::Title);
        copyDetailsToShow(copy, original, infos);

        CHECK(copy.englishTitle() == "Manual English Title");
    }
}
