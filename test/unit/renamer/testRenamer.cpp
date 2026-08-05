#include "test/test_helpers.h"

#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
#include "data/tv_show/TvShow.h"
#include "data/tv_show/TvShowEpisode.h"
#include "renamer/ConcertRenamer.h"
#include "renamer/EpisodeRenamer.h"
#include "renamer/MovieRenamer.h"

#include <algorithm>

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

    SECTION("englishTitle falls back to title when empty")
    {
        TvShow show;
        show.setTitle("Amélie");
        show.setEnglishTitle("");
        mediaelch::TvShowRenamerData data{show};
        CHECK(data.value("englishTitle") == QStringLiteral("Amélie"));
        CHECK(data.value("title") == QStringLiteral("Amélie"));
    }

    SECTION("englishTitle uses its own value when set")
    {
        TvShow show;
        show.setTitle("Амели");
        show.setEnglishTitle("Amélie");
        mediaelch::TvShowRenamerData data{show};
        CHECK(data.value("englishTitle") == QStringLiteral("Amélie"));
        CHECK(data.value("title") == QStringLiteral("Амели"));
    }

    SECTION("englishTitle placeholder is listed and replaceable")
    {
        TvShow show;
        show.setTitle("Fallback");
        show.setEnglishTitle("English Name");
        show.setFirstAired(QDate(2001, 4, 25));
        mediaelch::TvShowRenamerData data{show};

        const auto all = placeholders.placeholders();
        const bool hasEnglishTitle = std::any_of(all.cbegin(), all.cend(), [](const mediaelch::Placeholder& p) {
            return p.name == QLatin1String("englishTitle") && p.isValue;
        });
        REQUIRE(hasEnglishTitle);

        CHECK(placeholders.replace("<englishTitle> (<year>)", data) == QStringLiteral("English Name (2001)"));
        show.setEnglishTitle("");
        CHECK(placeholders.replace("<englishTitle> (<year>)", data) == QStringLiteral("Fallback (2001)"));
    }

    SECTION("sortTitle has no title fallback")
    {
        TvShow show;
        show.setTitle("Amélie");
        show.setSortTitle("");
        mediaelch::TvShowRenamerData data{show};
        CHECK(data.value("sortTitle").isEmpty());

        show.setSortTitle("Amelie");
        CHECK(data.value("sortTitle") == QStringLiteral("Amelie"));
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

    SECTION("englishTitle uses parent show english title with title fallback")
    {
        TvShow show;
        show.setTitle("Show RU");
        show.setEnglishTitle("Show EN");

        TvShowEpisode episode({}, &show);
        episode.setShowTitle("Stale Show Title");
        mediaelch::EpisodeRenamerData data{episode};
        CHECK(data.value("englishTitle") == QStringLiteral("Show EN"));

        show.setEnglishTitle("");
        // Falls back to show title (not episode showTitle), same as TvShowRenamerData.
        CHECK(data.value("englishTitle") == QStringLiteral("Show RU"));
    }

    SECTION("originalTitle and sortTitle use parent show values")
    {
        TvShow show;
        show.setTitle("Show RU");
        show.setOriginalTitle("Show Original");
        show.setSortTitle("Show Sort");

        TvShowEpisode episode({}, &show);
        episode.setShowTitle("Show RU");
        mediaelch::EpisodeRenamerData data{episode};
        CHECK(data.value("originalTitle") == QStringLiteral("Show Original"));
        CHECK(data.value("sortTitle") == QStringLiteral("Show Sort"));

        show.setOriginalTitle("");
        show.setSortTitle("");
        CHECK(data.value("originalTitle") == QStringLiteral("Show RU"));
        CHECK(data.value("sortTitle").isEmpty());
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

    SECTION("englishTitle falls back to title when empty")
    {
        Movie movie;
        movie.setTitle("Amélie");
        movie.setEnglishTitle("");
        mediaelch::MovieRenamerData data{movie};
        CHECK(data.value("englishTitle") == QStringLiteral("Amélie"));
        CHECK(data.value("title") == QStringLiteral("Amélie"));
    }

    SECTION("englishTitle uses its own value when set")
    {
        Movie movie;
        movie.setTitle("Амели");
        movie.setEnglishTitle("Amélie");
        mediaelch::MovieRenamerData data{movie};
        CHECK(data.value("englishTitle") == QStringLiteral("Amélie"));
        CHECK(data.value("title") == QStringLiteral("Амели"));
    }

    SECTION("englishTitle placeholder is listed and replaceable")
    {
        Movie movie;
        movie.setTitle("Fallback");
        movie.setEnglishTitle("English Name");
        mediaelch::MovieRenamerData data{movie};
        data.setExtension("mkv");

        const auto all = placeholders.placeholders();
        const bool hasEnglishTitle = std::any_of(all.cbegin(), all.cend(), [](const mediaelch::Placeholder& p) {
            return p.name == QLatin1String("englishTitle") && p.isValue;
        });
        REQUIRE(hasEnglishTitle);

        CHECK(placeholders.replace("<englishTitle>.<extension>", data) == QStringLiteral("English Name.mkv"));
        movie.setEnglishTitle("");
        CHECK(placeholders.replace("<englishTitle>.<extension>", data) == QStringLiteral("Fallback.mkv"));
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
