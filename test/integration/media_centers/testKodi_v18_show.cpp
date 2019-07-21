#include "test/test_helpers.h"

#include "media_centers/kodi/TvShowXmlReader.h"
#include "media_centers/kodi/v18/TvShowXmlWriterV18.h"
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
    QString showContent = getFileContent("show/" + filename);

    mediaelch::kodi::TvShowXmlReader reader(show);
    QDomDocument doc;
    doc.setContent(showContent);
    reader.parseNfoDom(doc);

    callback(show);

    mediaelch::kodi::TvShowXmlWriterV18 writer(show);
    QString actual = writer.getTvShowXml().trimmed();
    writeTempFile(filename, actual);
    checkSameXml(showContent, actual);
}

TEST_CASE("TV show XML writer for Kodi v18", "[data][tvshow][kodi][nfo]")
{
    SECTION("Empty tvshow")
    {
        TvShow tvShow;
        QString filename = "kodi_v18_show_empty.nfo";
        CAPTURE(filename);

        mediaelch::kodi::TvShowXmlWriterV18 writer(tvShow);
        QString actual = writer.getTvShowXml().trimmed();
        writeTempFile(filename, actual);
        checkSameXml(getFileContent("show/" + filename), actual);
    }

    SECTION("read / write details: Game of Thrones")
    {
        createAndCompareTvShow("kodi_v18_show_Game_of_Thrones.nfo", [](TvShow& show) {
            // check some details
            CHECK(show.name() == "Game of Thrones");
            CHECK(show.ratings().first().voteCount == 1783);
            CHECK(show.certification() == Certification("TV-MA"));
            CHECK(show.actors().size() == 80);
        });
    }

    SECTION("read / write details: Torchwood")
    {
        createAndCompareTvShow("kodi_v18_show_Torchwood.nfo", [](TvShow& show) {
            // check some details
            CHECK(show.name() == "Torchwood");
            CHECK(show.ratings().first().voteCount == 167);
            CHECK(show.certification() == Certification("TV-MA"));
        });
    }

    SECTION("Full movie details")
    {
        // Taken from https://kodi.wiki/view/NFO_files/TV_shows

        /*
         *
Currently not yet supported:

  <originaltitle>TtvshowC09</originaltitle>

  These are listed on the Kodi wiki but seem to be wrong:
  <season>5</season>
  <displayseason>-1</displayseason>
  <displayepisode>-1</displayepisode>
  <playcount>0</playcount>
  <tagline>A tagline</tagline>
  <aired></aired>
  <trailer></trailer>
  <path>E:\TV Shows-Test - Copy\Angel\</path>
  <filenameandpath></filenameandpath>
  <basepath>E:\TV Shows-Test - Copy\Angel\</basepath>
  <art>
    <banner>E:\TV Shows-Test - Copy\Angel\banner.jpg</banner>
    <fanart>E:\TV Shows-Test - Copy\Angel\fanart.jpg</fanart>
    <poster>E:\TV Shows-Test - Copy\Angel\poster.jpg</poster>
    <season num="-1">
      <poster>E:\TV Shows-Test - Copy\Angel\season-all-poster.jpg</poster>
    </season>
    <season num="0">
      <poster>http://thetvdb.com/banners/seasons/71035-0-2.jpg</poster>
    </season>
    <season num="1">
      <banner>http://thetvdb.com/banners/seasonswide/71035-1.jpg</banner>
      <poster>E:\TV Shows-Test - Copy\Angel\season01-poster.jpg</poster>
    </season>
    <season num="2">
      <banner>http://thetvdb.com/banners/seasonswide/71035-2.jpg</banner>
      <poster>E:\TV Shows-Test - Copy\Angel\season02-poster.jpg</poster>
    </season>
    <season num="3">
      <banner>http://thetvdb.com/banners/seasonswide/71035-3.jpg</banner>
      <poster>E:\TV Shows-Test - Copy\Angel\season03-poster.jpg</poster>
    </season>
    <season num="4">
      <banner>http://thetvdb.com/banners/seasonswide/71035-4.jpg</banner>
      <poster>E:\TV Shows-Test - Copy\Angel\season04-poster.jpg</poster>
    </season>
    <season num="5">
      <banner>http://thetvdb.com/banners/seasonswide/71035-5.jpg</banner>
      <poster>E:\TV Shows-Test - Copy\Angel\season05-poster.jpg</poster>
    </season>
  </art>
  <fanart url="http://thetvdb.com/banners/">
    <thumb dim="1920x1080" colors="" preview="_cache/fanart/original/71035-6.jpg">fanart/original/71035-6.jpg</thumb>
    <thumb dim="1920x1080" colors="" preview="_cache/fanart/original/71035-18.jpg">fanart/original/71035-18.jpg</thumb>
  </fanart>
  <file></file>
  <episodeguide>
    <url cache="71035-en.xml">http://thetvdb.com/api/439DFEBA9D3059C6/series/71035/all/en.zip</url>
  </episodeguide>

  Todo:
  - distinguish between plot and overview

         */

        TvShow show;
        show.setName("Angels");
        show.setShowTitle("Angels");
        show.setSortTitle("TtvshowC15");
        show.setId(TvDbId(71035));
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

        mediaelch::kodi::TvShowXmlWriterV18 writer(show);

        QString actual = writer.getTvShowXml().trimmed();
        QString filename = "kodi_v18_show_all.nfo";
        CAPTURE(filename);
        writeTempFile(filename, actual);
        checkSameXml(getFileContent("show/kodi_v18_show_all.nfo"), actual);
    }
}
