#include "test/test_helpers.h"

#include "data/Rating.h"

#include <string>

using namespace mediaelch;

TEST_CASE("Rating class", "[data]")
{
    SECTION("comparing ratings")
    {
        Rating r1;
        r1.voteCount = 100;
        r1.rating = 5.5;
        r1.source = "imdb";

        Rating r2;
        r2.voteCount = 90;
        r2.rating = 9.5;
        r2.source = "tvdb";

        CHECK(r2 > r1);
        CHECK(r2 >= r1);
        CHECK(r1 < r2);
        CHECK(r1 <= r2);
    }

    SECTION("Rating::sourceToName")
    {
        SECTION("known sources map to display-names")
        {
            CHECK(Rating::sourceToName("themoviedb") == "TMDB");
            CHECK(Rating::sourceToName("tmdb") == "TMDB");
            CHECK(Rating::sourceToName("imdb") == "IMDb");
            CHECK(Rating::sourceToName("thetvdb") == "TheTvDb");
            CHECK(Rating::sourceToName("tvdb") == "TheTvDb");
            CHECK(Rating::sourceToName("metacritic") == "Metacritic");
            CHECK(Rating::sourceToName("default") == "Default");
        }

        SECTION("unknown source is returned as-is")
        {
            CHECK(Rating::sourceToName("unknown_source") == "unknown_source");
            CHECK(Rating::sourceToName("") == "");
        }

        SECTION("commonSources is non-empty and contains known entries")
        {
            const QStringList sources = Rating::commonSources();
            REQUIRE_FALSE(sources.isEmpty());
            // Only check a few stable entries
            CHECK(sources.contains("themoviedb"));
            CHECK(sources.contains("imdb"));
        }
    }
}

TEST_CASE("Ratings class", "[data]")
{
    SECTION("setOrAddRating")
    {
        SECTION("setOrAddRating adds a new source")
        {
            Ratings ratings;
            Rating r;
            r.source = "imdb";
            r.rating = 7.5;
            ratings.setOrAddRating(r);
            REQUIRE(ratings.size() == 1);
            CHECK(ratings.first().rating == 7.5);
        }

        SECTION("setOrAddRating replaces rating with same source")
        {
            Ratings ratings;
            Rating r1;
            r1.source = "imdb";
            r1.rating = 7.5;
            ratings.setOrAddRating(r1);

            Rating r2;
            r2.source = "imdb";
            r2.rating = 8.0;
            ratings.setOrAddRating(r2);

            REQUIRE(ratings.size() == 1);
            CHECK(ratings.first().rating == 8.0);
        }

        SECTION("setOrAddRating with different sources")
        {
            Ratings ratings;
            Rating r1;
            r1.source = "imdb";
            r1.rating = 7.5;
            ratings.setOrAddRating(r1);

            Rating r2;
            r2.source = "themoviedb";
            r2.rating = 8.2;
            ratings.setOrAddRating(r2);

            CHECK(ratings.size() == 2);
            CHECK(ratings.hasSource("imdb"));
            CHECK(ratings.hasSource("themoviedb"));
        }
    }

    SECTION("empty Ratings -> hasSource returns false")
    {
        Ratings ratings;
        CHECK_FALSE(ratings.hasSource("imdb"));
    }

    SECTION("merge ratings from another Ratings object")
    {
        Ratings a;
        Rating r1;
        r1.source = "imdb";
        r1.rating = 7.5;
        a.setOrAddRating(r1);

        Ratings b;
        Rating r2;
        r2.source = "themoviedb";
        r2.rating = 8.2;
        b.setOrAddRating(r2);

        a.merge(b);

        REQUIRE(a.size() == 2);
        CHECK(a.hasSource("imdb"));
        CHECK(a.hasSource("themoviedb"));
    }
}
