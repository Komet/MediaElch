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
