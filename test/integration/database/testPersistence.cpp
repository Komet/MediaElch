#include "test/test_helpers.h"

#include "data/tv_show/TvShow.h"
#include "database/Database.h"
#include "database/TvShowPersistence.h"

#include <QSqlError>
#include <QSqlQuery>

TEST_CASE("Persist the hide-specials setting", "[database][persistence]")
{
    Database database{QStringLiteral(":memory:")};
    REQUIRE(database.db().isOpen());
    QSqlQuery query(database.db());
    TvShow show;
    query.prepare("INSERT INTO showsSettings(tmdbid, url, showMissingEpisodes, hideSpecialsInMissingEpisodes, dir) "
                  "VALUES('123', '', 1, 0, :dir)");
    query.bindValue(":dir", show.dir().toString().toUtf8());
    REQUIRE(query.exec());
    show.setTmdbId(TmdbId(123));
    mediaelch::TvShowPersistence persistence{database};
    for (const bool hide : {true, false}) {
        persistence.setHideSpecialsInMissingEpisodes(&show, hide);
        REQUIRE(query.exec("SELECT hideSpecialsInMissingEpisodes, showMissingEpisodes FROM showsSettings"));
        REQUIRE(query.next());
        CHECK(query.value(0).toBool() == hide);
        CHECK(query.value(1).toBool());
        query.finish();
    }
}

TEST_CASE("Create missing TV show settings", "[database][persistence]")
{
    Database database{QStringLiteral(":memory:")};
    mediaelch::TvShowPersistence persistence{database};
    TvShow show;
    bool showMissing = false;
    bool hideSpecials = false;

    SECTION("Create settings on demand without a TMDb ID")
    {
        const auto id = persistence.showsSettingsId(&show);
        REQUIRE(id.isValid());
        CHECK(persistence.showsSettingsId(&show).toInt() == id.toInt());
    }

    SECTION("Enable missing episodes before settings exist")
    {
        persistence.setShowMissingEpisodes(&show, true);
        showMissing = true;
    }

    SECTION("Hide specials before settings exist")
    {
        persistence.setHideSpecialsInMissingEpisodes(&show, true);
        hideSpecials = true;
    }

    SECTION("Add a show without a TMDb ID")
    {
        persistence.add(&show, mediaelch::DirectoryPath(QStringLiteral("/shows")));
        REQUIRE(show.databaseId().isValid());
    }

    QSqlQuery query(database.db());
    REQUIRE(query.exec("SELECT tmdbid, url, showMissingEpisodes, hideSpecialsInMissingEpisodes FROM showsSettings"));
    REQUIRE(query.next());
    CHECK_FALSE(query.value(0).isNull());
    CHECK_FALSE(query.value(1).isNull());
    CHECK_FALSE(query.value(2).isNull());
    CHECK_FALSE(query.value(3).isNull());
    CHECK(query.value(2).toBool() == showMissing);
    CHECK(query.value(3).toBool() == hideSpecials);
    CHECK_FALSE(query.next());
}

TEST_CASE("Keep cached episode lists separate for each show directory", "[database][persistence]")
{
    Database database{QStringLiteral(":memory:")};
    mediaelch::TvShowPersistence persistence{database};
    TvShow firstShow{mediaelch::DirectoryPath(QStringLiteral("/shows/first"))};
    TvShow secondShow{mediaelch::DirectoryPath(QStringLiteral("/shows/second"))};
    const auto firstId = persistence.showsSettingsId(&firstShow);
    const auto secondId = persistence.showsSettingsId(&secondShow);
    REQUIRE(firstId.isValid());
    REQUIRE(secondId.isValid());
    REQUIRE(firstId.toInt() != secondId.toInt());

    TvShowEpisode episode;
    episode.setSeason(SeasonNumber(1));
    episode.setEpisode(EpisodeNumber(2));
    persistence.addEpisodeToShowList(&episode, firstId, TmdbId(123));
    persistence.addEpisodeToShowList(&episode, secondId, TmdbId(123));

    // Refreshing the second directory must not reuse or remove the first directory's row.
    persistence.clearEpisodeList(secondId);
    episode.setEpisode(EpisodeNumber(3));
    persistence.addEpisodeToShowList(&episode, secondId, TmdbId(123));
    persistence.cleanUpEpisodeList(secondId);

    QSqlQuery query(database.db());
    query.prepare("SELECT episodeNumber FROM showsEpisodes WHERE idShow=:idShow");
    query.bindValue(":idShow", firstId.toInt());
    REQUIRE(query.exec());
    REQUIRE(query.next());
    CHECK(query.value(0).toInt() == 2);
    CHECK_FALSE(query.next());
    query.bindValue(":idShow", secondId.toInt());
    REQUIRE(query.exec());
    REQUIRE(query.next());
    CHECK(query.value(0).toInt() == 3);
    CHECK_FALSE(query.next());
}
