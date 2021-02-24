#include "test/test_helpers.h"

#include "media_centers/kodi/TvShowXmlReader.h"
#include "media_centers/kodi/TvShowXmlWriter.h"
#include "test/integration/resource_dir.h"
#include "tv_shows/TvShow.h"

#include <QDateTime>
#include <QDomDocument>
#include <chrono>

using namespace std::chrono_literals;

/// Reads a file, parses it, executes callback (you can add further checks), then
/// writes the file to a temporary file and compares the created file with the
/// reference file.
template<class Callback>
static void createAndCompareTvShow(const QString& filename, Callback callback)
{
    CAPTURE(filename);

    TvShow show;
    QString showContent = getFileContent(filename);

    mediaelch::kodi::TvShowXmlReader reader(show);
    QDomDocument doc;
    doc.setContent(showContent);
    reader.parseNfoDom(doc);

    callback(show);

    mediaelch::kodi::TvShowXmlWriterGeneric writer(mediaelch::KodiVersion(18), show);
    QString actual = writer.getTvShowXml(true).trimmed();
    writeTempFile(filename, actual);
    checkSameXml(showContent, actual);
}

TEST_CASE("TV show XML writer for Kodi v18", "[data][tvshow][kodi][nfo]")
{
    SECTION("Empty tvshow")
    {
        TvShow tvShow;
        QString filename = "show/kodi_v18_show_empty.nfo";
        CAPTURE(filename);

        mediaelch::kodi::TvShowXmlWriterGeneric writer(mediaelch::KodiVersion(18), tvShow);
        QString actual = writer.getTvShowXml(true).trimmed();
        writeTempFile(filename, actual);
        checkSameXml(getFileContent(filename), actual);
    }

    SECTION("read / write details: Game of Thrones")
    {
        createAndCompareTvShow("show/kodi_v18_show_Game_of_Thrones.nfo", [](TvShow& show) {
            // check some details
            CHECK(show.title() == "Game of Thrones");
            CHECK(show.ratings().first().voteCount == 1783);
            CHECK(show.certification() == Certification("TV-MA"));
            CHECK(show.actors().size() == 80);
        });
        createAndCompareTvShow("show/kodi_v18_show_Game_of_Thrones_TvDb_episode_guide.nfo", [](TvShow& show) {
            CHECK(show.title() == "Game of Thrones");
            CHECK(show.tvdbId().isValid());
            CHECK_FALSE(show.tmdbId().isValid());
        });
    }

    SECTION("read / write details: Torchwood")
    {
        createAndCompareTvShow("show/kodi_v18_show_Torchwood.nfo", [](TvShow& show) {
            // check some details
            CHECK(show.title() == "Torchwood");
            CHECK(show.ratings().first().voteCount == 167);
            CHECK(show.certification() == Certification("TV-MA"));
        });
    }

    SECTION("Full show details")
    {
        // Taken from https://kodi.wiki/view/NFO_files/TV_shows
        TvShow show;
        show.setTitle("Angel");
        show.setShowTitle("Angel 1");
        show.setSortTitle("TC15");
        show.setOriginalTitle("Angel 2");
        show.setTvdbId(TvDbId(71035));
        show.setTmdbId(TmdbId(2426));
        show.setImdbId(ImdbId("tt0162065"));
        show.setTvMazeId(TvMazeId(428));
        {
            Rating rating;
            rating.rating = 8.6;
            rating.voteCount = 88;
            rating.maxRating = 10;
            show.ratings().push_back(rating);
        }
        show.setUserRating(9.56);
        show.setCertification(Certification("TV-PG"));
        show.setRuntime(45min);
        show.setTop250(4);
        show.setFirstAired(QDate::fromString("1999-10-05", "yyyy-MM-dd"));
        show.setOverview("Angel is an American television series, a spin-off from the television series Buffy the "
                         "Vampire Slayer. Angel (David Boreanaz), a 240-year old vampire cursed with a conscience, "
                         "haunts the dark streets of Los Angeles alone");
        show.addGenre("Action");
        show.addGenre("Comedy");
        show.addGenre("Drama");
        show.addTag("BestTag");
        show.setStatus("Ended");
        show.setNetwork("The WB");
        {
            Poster banner;
            banner.thumbUrl = "http://thetvdb.com/banners/graphical/71035-g7.jpg";
            banner.originalUrl = "http://thetvdb.com/banners/graphical/71035-g7.jpg";
            banner.originalSize = QSize(758, 140);
            show.addBanner(banner);
        }
        {
            Poster poster;
            poster.season = SeasonNumber(5);
            poster.originalUrl = "http://thetvdb.com/banners/seasons/71035-5-2.jpg";
            poster.originalSize = QSize(400, 578);
            poster.thumbUrl = "http://thetvdb.com/banners/seasons/71035-5-2.jpg";
            show.addSeasonPoster(SeasonNumber(5), poster);
        }
        {
            Actor actor;
            actor.name = "David Boreanaz";
            actor.role = "Angel";
            actor.thumb = "http://thetvdb.com/banners/actors/6309.jpg";
            show.addActor(actor);
        }
        {
            Actor actor;
            actor.name = "Stephanie Romanov";
            actor.role = "Lilah Morgan";
            show.addActor(actor);
        }

        mediaelch::kodi::TvShowXmlWriterGeneric writer(mediaelch::KodiVersion(18), show);

        QString actual = writer.getTvShowXml(true).trimmed();
        QString filename = "show/kodi_v18_show_all.nfo";
        CAPTURE(filename);
        writeTempFile(filename, actual);
        checkSameXml(getFileContent("show/kodi_v18_show_all.nfo"), actual);
    }
}
