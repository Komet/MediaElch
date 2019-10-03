#include "test/test_helpers.h"

#include "media_centers/kodi/EpisodeXmlReader.h"
#include "media_centers/kodi/v16/EpisodeXmlWriterV16.h"
#include "test/integration/resource_dir.h"
#include "tv_shows/TvShowEpisode.h"

#include <QDateTime>
#include <QDomDocument>
#include <chrono>

using namespace std::chrono_literals;

TEST_CASE("Episode XML writer for Kodi v16", "[data][tvshow][kodi][nfo]")
{
    SECTION("empty episode")
    {
        TvShowEpisode episode;
        QString filename = "show/kodi_v16_episode_empty.nfo";
        CAPTURE(filename);

        mediaelch::kodi::EpisodeXmlWriterV16 writer({&episode});
        QString actual = writer.getEpisodeXmlWithSingleRoot().trimmed();
        writeTempFile(filename, actual);
        checkSameXml(getFileContent(filename), actual);
    }

    SECTION("empty multi episode")
    {
        TvShowEpisode episode1;
        TvShowEpisode episode2;
        QString filename = "show/kodi_v16_episode_multi_empty.nfo";
        CAPTURE(filename);

        mediaelch::kodi::EpisodeXmlWriterV16 writer({&episode1, &episode2});
        QString actual = writer.getEpisodeXmlWithSingleRoot().trimmed();
        writeTempFile(filename, actual);
        checkSameXml(getFileContent(filename), actual);
    }

    SECTION("read / write details: empty episode")
    {
        using mediaelch::kodi::EpisodeXmlReader;

        TvShowEpisode episode;
        QString filename = "show/kodi_v16_episode_empty.nfo";
        QString episodeContent = getFileContent(filename);
        CAPTURE(filename);

        EpisodeXmlReader reader(episode);

        QDomDocument doc;
        doc.setContent(episodeContent);
        reader.parseNfoDom(doc.elementsByTagName("episodedetails").at(0).toElement());

        mediaelch::kodi::EpisodeXmlWriterV16 writer({&episode});
        QString actual = writer.getEpisodeXmlWithSingleRoot().trimmed();
        writeTempFile(filename, actual);
        checkSameXml(episodeContent, actual);
    }
}
