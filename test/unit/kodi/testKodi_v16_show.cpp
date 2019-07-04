#include "test/test_helpers.h"

#include "media_centers/kodi/v16/TvShowXmlWriterV16.h"
#include "tv_shows/TvShow.h"

#include <QDateTime>
#include <QDomDocument>
#include <chrono>

using namespace std::chrono_literals;

TEST_CASE("TV show XML writer for Kodi v16", "[data][tvshow][kodi][nfo]")
{
    SECTION("Empty tvshow")
    {
        TvShow tvShow;
        mediaelch::kodi::TvShowXmlWriterV16 writer(tvShow);

        const QByteArray expectedNfo = R"(<?xml version="1.0" encoding="UTF-8" standalone='yes'?>
<tvshow>
   <title></title>
   <showtitle></showtitle>
   <rating></rating>
   <votes></votes>
   <top250>0</top250>
   <episode>0</episode>
   <plot></plot>
   <outline></outline>
   <mpaa></mpaa>
   <premiered></premiered>
   <studio></studio>
   <tvdbid></tvdbid>
   <id></id>
   <imdbid></imdbid>
   <genre></genre>
</tvshow>)";

        checkSameXml(expectedNfo, writer.getTvShowXml().trimmed());
    }
}
