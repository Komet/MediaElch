#include "test/test_helpers.h"

#include "scrapers/tv_show/imdb/ImdbTvSeasonParser.h"
#include "tv_shows/SeasonNumber.h"

using namespace mediaelch::scraper;

TEST_CASE("ImdbTvSeasonParser extracts season numbers from episodes overview page", "[season][ImdbTv][parse_data]")
{
    SECTION("Extracts all specified seasons")
    {
        // Taken from https://www.imdb.com/title/tt0096697/episodes on 2020-11-24
        // Removed most seasons for shorter HTML code.
        QString episodesHtml = R"(
<div class="seasonAndYearNav">
    <div class="episode-list-select">
  <div>
    <label for="bySeason">Season:</label>
    <select id="bySeason" tconst="tt0096697" class="current">
      <!--
      This ensures that we don't wind up accidentally marking two options
      (Unknown and the blank one) as selected.
      -->
      <option  value="1">
        1
      </option>
      <!--
      This ensures that we don't wind up accidentally marking two options
      (Unknown and the blank one) as selected.
      -->
      <option  value="2">
        2
      </option>
      <!--
      This ensures that we don't wind up accidentally marking two options
      (Unknown and the blank one) as selected.
      -->
      <option selected="selected" value="32">
        32
      </option>
    </select>
  </div>
    )";

        QSet<SeasonNumber> expectedSeasons{SeasonNumber(1), SeasonNumber(2), SeasonNumber(32)};

        QSet<SeasonNumber> actualSeasons = ImdbTvSeasonParser::parseSeasonNumbersFromEpisodesPage(episodesHtml);
        CHECK(actualSeasons == expectedSeasons);
    }

    SECTION("Ignores invalid season numbers")
    {
        // Taken from https://www.imdb.com/title/tt0096697/episodes on 2020-11-24
        // Removed most seasons for shorter HTML code.
        QString episodesHtml = R"(
    <label for="bySeason">Season:</label>
    <select id="bySeason" tconst="tt0096697" class="current">
      <option  value="-1">
        1
      </option>
      <option  value="-2">
        -3
      </option>
      <option value="0">
        1000
      </option>
    </select>
      )";

        QSet<SeasonNumber> expectedSeasons{SeasonNumber(0)};

        QSet<SeasonNumber> actualSeasons = ImdbTvSeasonParser::parseSeasonNumbersFromEpisodesPage(episodesHtml);
        CHECK(actualSeasons == expectedSeasons);
    }
}
