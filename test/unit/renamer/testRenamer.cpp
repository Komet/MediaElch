#include "test/test_helpers.h"

#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
#include "data/tv_show/TvShow.h"
#include "renamer/ConcertRenamer.h"
#include "renamer/EpisodeRenamer.h"
#include "renamer/MovieRenamer.h"

/// Ensure that we can access all data/conditions that are provided as placeholders.
/// This should catch cases where we access nullptr, etc.
void loadDataForAllPlaceholders(mediaelch::RenamerPlaceholders& placeholderProvider, mediaelch::RenamerData& data)
{
    for (const auto& placeholder : placeholderProvider.placeholders()) {
        const bool val = data.passesCondition(placeholder.name);
        Q_UNUSED(val);
        CHECK(true);
    }
}

TEST_CASE("TvShow Renamer works", "[renamer][tv_show]")
{
    mediaelch::TvShowRenamerPlaceholders placeholders;

    SECTION("Handles empty tv shows")
    {
        TvShow show;
        mediaelch::TvShowRenamerData data{show};
        loadDataForAllPlaceholders(placeholders, data);
    }
}

TEST_CASE("Episode Renamer works", "[renamer][episode]")
{
    mediaelch::EpisodeRenamerPlaceholders placeholders;

    SECTION("Handles empty episodes")
    {
        TvShowEpisode episode;
        mediaelch::EpisodeRenamerData data{episode};
        loadDataForAllPlaceholders(placeholders, data);
    }
}

TEST_CASE("Movie Renamer works", "[renamer][movie]")
{
    mediaelch::MovieRenamerPlaceholders placeholders;

    SECTION("Handles empty movies")
    {
        Movie movie;
        mediaelch::MovieRenamerData data{movie};
        loadDataForAllPlaceholders(placeholders, data);
    }
}

TEST_CASE("Concert Renamer works", "[renamer][concert]")
{
    mediaelch::ConcertRenamerPlaceholders placeholders;

    SECTION("Handles empty concerts")
    {
        Concert concert;
        mediaelch::ConcertRenamerData data{concert};
        loadDataForAllPlaceholders(placeholders, data);
    }
}
