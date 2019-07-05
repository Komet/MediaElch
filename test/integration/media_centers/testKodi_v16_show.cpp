#include "test/test_helpers.h"

#include "media_centers/kodi/v16/TvShowXmlWriterV16.h"
#include "test/integration/resource_dir.h"
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
        checkSameXml(getFileContent("show/kodi_v16_show_empty.nfo"), writer.getTvShowXml().trimmed());
    }
}
