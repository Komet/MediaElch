#include "test/test_helpers.h"

#include "data/concert/Concert.h"
#include "data/tv_show/TvShow.h"
#include "database/ConcertPersistence.h"
#include "database/Database.h"
#include "database/MoviePersistence.h"
#include "database/TvShowPersistence.h"

#include <QSqlQuery>
#include <functional>

namespace {

const mediaelch::FileList oldFiles{mediaelch::FilePath(QStringLiteral("/media/old.mkv"))};
const mediaelch::FileList newFiles{mediaelch::FilePath(QStringLiteral("/media/new-a.mkv")),
    mediaelch::FilePath(QStringLiteral("/media/new-b.mkv"))};

void checkAtomicFileUpdate(Database& database,
    const QString& parentTable,
    const QString& fileTable,
    const std::function<void()>& update)
{
    QSqlQuery query(database.db());
    bool succeeded = false;

    SECTION("Roll back a failed content update")
    {
        REQUIRE(query.exec("CREATE TEMP TRIGGER fail_update BEFORE UPDATE ON " + parentTable
                           + " BEGIN SELECT RAISE(ABORT, 'test failure'); END"));
        update();
    }
    SECTION("Roll back a failed file deletion")
    {
        REQUIRE(query.exec("CREATE TEMP TRIGGER fail_delete BEFORE DELETE ON " + fileTable
                           + " BEGIN SELECT RAISE(ABORT, 'test failure'); END"));
        update();
    }
    SECTION("Roll back a failed second file insertion")
    {
        REQUIRE(
            query.exec("CREATE TEMP TRIGGER fail_insert BEFORE INSERT ON " + fileTable + " WHEN (SELECT COUNT(*) FROM "
                       + fileTable + ") > 0 BEGIN SELECT RAISE(ABORT, 'test failure'); END"));
        update();
    }
    SECTION("Commit a successful replacement")
    {
        update();
        succeeded = true;
    }
    SECTION("Preserve the caller's transaction")
    {
        REQUIRE(database.db().transaction());
        update();
        REQUIRE(query.exec("SELECT content FROM " + parentTable));
        REQUIRE(query.next());
        CHECK(query.value(0).toString() == QStringLiteral("after"));
        query.finish();
        REQUIRE(database.db().rollback());
    }
    SECTION("Preserve the caller's other writes when the update fails")
    {
        REQUIRE(database.db().transaction());
        REQUIRE(query.exec("INSERT INTO importCache(filename, type, path) VALUES('test', 'movie', '/media')"));
        REQUIRE(query.exec("CREATE TEMP TRIGGER fail_insert BEFORE INSERT ON " + fileTable
                           + " BEGIN SELECT RAISE(ABORT, 'test failure'); END"));
        update();
        REQUIRE(database.db().commit());
        REQUIRE(query.exec("SELECT COUNT(*) FROM importCache"));
        REQUIRE(query.next());
        CHECK(query.value(0).toInt() == 1);
        query.finish();
    }

    REQUIRE(query.exec("SELECT content FROM " + parentTable));
    REQUIRE(query.next());
    CHECK(query.value(0).toString() == (succeeded ? QStringLiteral("after") : QStringLiteral("before")));
    query.finish();
    REQUIRE(query.exec("SELECT file FROM " + fileTable + " ORDER BY file"));
    QStringList actualFiles;
    while (query.next()) {
        actualFiles << QString::fromUtf8(query.value(0).toByteArray());
    }
    QStringList expectedFiles;
    for (const auto& file : succeeded ? newFiles : oldFiles) {
        expectedFiles << file.toString();
    }
    CHECK(actualFiles == expectedFiles);
}

} // namespace

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

TEST_CASE("Cache episodes without TMDb IDs", "[database][persistence]")
{
    Database database{QStringLiteral(":memory:")};
    mediaelch::TvShowPersistence persistence{database};
    TvShow show;
    const auto id = persistence.showsSettingsId(&show);
    REQUIRE(id.isValid());

    TvShowEpisode episode;
    for (const int season : {0, 1, 2}) {
        episode.setSeason(SeasonNumber(season));
        for (const int number : {1, 2}) {
            episode.setEpisode(EpisodeNumber(number));
            persistence.addEpisodeToShowList(&episode, id, TmdbId::NoId);
        }
    }

    episode.setSeason(SeasonNumber(1));
    episode.setEpisode(EpisodeNumber(2));
    persistence.addEpisodeToShowList(&episode, id, TmdbId::NoId);

    QSqlQuery query(database.db());
    REQUIRE(query.exec("SELECT COUNT(*) FROM showsEpisodes"));
    REQUIRE(query.next());
    REQUIRE(query.value(0).toInt() == 6);
    query.finish();

    // Switching scrapers can add an ID, then omit it again, without duplicating the row or losing the ID.
    persistence.addEpisodeToShowList(&episode, id, TmdbId(123));
    persistence.addEpisodeToShowList(&episode, id, TmdbId::NoId);
    REQUIRE(query.exec("SELECT tmdbid FROM showsEpisodes WHERE seasonNumber=1 AND episodeNumber=2"));
    REQUIRE(query.next());
    CHECK(query.value(0).toString() == QStringLiteral("123"));
    CHECK_FALSE(query.next());
    query.finish();

    persistence.clearEpisodeList(id);
    persistence.addEpisodeToShowList(&episode, id, TmdbId::NoId);
    persistence.cleanUpEpisodeList(id);
    REQUIRE(query.exec("SELECT COUNT(*) FROM showsEpisodes"));
    REQUIRE(query.next());
    CHECK(query.value(0).toInt() == 1);
}

TEST_CASE("Replace cached movie files atomically", "[database][persistence]")
{
    Database database{QStringLiteral(":memory:")};
    mediaelch::MoviePersistence persistence{database};
    Movie movie{oldFiles.toStringList()};
    movie.setNfoContent(QStringLiteral("before"));
    persistence.addMovie(&movie, mediaelch::DirectoryPath(QStringLiteral("/media")));
    REQUIRE(movie.databaseId().isValid());
    movie.setNfoContent(QStringLiteral("after"));
    movie.setFiles(newFiles);
    checkAtomicFileUpdate(database, "movies", "movieFiles", [&]() { persistence.update(&movie); });
}

TEST_CASE("Replace cached concert files atomically", "[database][persistence]")
{
    Database database{QStringLiteral(":memory:")};
    mediaelch::ConcertPersistence persistence{database};
    Concert concert{oldFiles};
    concert.setNfoContent(QStringLiteral("before"));
    persistence.add(&concert, mediaelch::DirectoryPath(QStringLiteral("/media")));
    REQUIRE(concert.databaseId().isValid());
    concert.setNfoContent(QStringLiteral("after"));
    concert.setFiles(newFiles);
    checkAtomicFileUpdate(database, "concerts", "concertFiles", [&]() { persistence.update(&concert); });
}

TEST_CASE("Replace cached episode files atomically", "[database][persistence]")
{
    Database database{QStringLiteral(":memory:")};
    mediaelch::TvShowPersistence persistence{database};
    TvShowEpisode episode{oldFiles};
    episode.setNfoContent(QStringLiteral("before"));
    persistence.add(&episode, mediaelch::DirectoryPath(QStringLiteral("/media")), mediaelch::DatabaseId(1));
    REQUIRE(episode.databaseId().isValid());
    episode.setNfoContent(QStringLiteral("after"));
    episode.setFiles(newFiles);
    checkAtomicFileUpdate(database, "episodes", "episodeFiles", [&]() { persistence.update(&episode); });
}

TEST_CASE("Roll back movie files and content when replacing subtitles fails", "[database][persistence]")
{
    Database database{QStringLiteral(":memory:")};
    mediaelch::MoviePersistence persistence{database};
    Movie movie{oldFiles.toStringList()};
    movie.setNfoContent(QStringLiteral("before"));
    auto* subtitle = new Subtitle(&movie);
    subtitle->setFiles({QStringLiteral("old.srt")});
    subtitle->setLanguage(QStringLiteral("en"));
    subtitle->setForced(false);
    movie.addSubtitle(subtitle);
    persistence.addMovie(&movie, mediaelch::DirectoryPath(QStringLiteral("/media")));

    QSqlQuery query(database.db());
    REQUIRE(query.exec("CREATE TEMP TRIGGER fail_subtitle BEFORE INSERT ON movieSubtitles "
                       "BEGIN SELECT RAISE(ABORT, 'test failure'); END"));
    movie.setNfoContent(QStringLiteral("after"));
    movie.setFiles(newFiles);
    subtitle->setFiles({QStringLiteral("new.srt")});
    persistence.update(&movie);

    REQUIRE(query.exec("SELECT content FROM movies"));
    REQUIRE(query.next());
    CHECK(query.value(0).toString() == QStringLiteral("before"));
    query.finish();
    REQUIRE(query.exec("SELECT file FROM movieFiles"));
    REQUIRE(query.next());
    CHECK(QString::fromUtf8(query.value(0).toByteArray()) == oldFiles.first().toString());
    CHECK_FALSE(query.next());
    REQUIRE(query.exec("SELECT files FROM movieSubtitles"));
    REQUIRE(query.next());
    CHECK(query.value(0).toString() == QStringLiteral("old.srt"));
    CHECK_FALSE(query.next());
}
