#include "test/test_helpers.h"

#include "media_centers/kodi/TvShowXmlReader.h"
#include "media_centers/kodi/v16/TvShowXmlWriterV16.h"
#include "test/integration/resource_dir.h"
#include "tv_shows/TvShow.h"

#include <QDateTime>
#include <QDomDocument>
#include <chrono>

using namespace std::chrono_literals;

TEST_CASE("TV show XML writer for Kodi v16", "[data][tvshow][kodi][nfo]")
{
    SECTION("empty tvshow")
    {
        TvShow tvShow;
        QString filename = "show/kodi_v16_show_empty.nfo";
        CAPTURE(filename);

        mediaelch::kodi::TvShowXmlWriterV16 writer(tvShow);
        QString actual = writer.getTvShowXml().trimmed();
        writeTempFile(filename, actual);
        checkSameXml(getFileContent(filename), actual);
    }

    SECTION("read / write details: empty tvshow")
    {
        TvShow tvShow;
        QString filename = "show/kodi_v16_show_empty.nfo";
        QString showContent = getFileContent(filename);
        CAPTURE(filename);

        mediaelch::kodi::TvShowXmlReader reader(tvShow);

        QDomDocument doc;
        doc.setContent(showContent);
        reader.parseNfoDom(doc);

        mediaelch::kodi::TvShowXmlWriterV16 writer(tvShow);
        QString actual = writer.getTvShowXml().trimmed();
        writeTempFile(filename, actual);
        checkSameXml(showContent, actual);
    }
}
