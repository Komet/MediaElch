#include "test/test_helpers.h"

#include "data/tv_show/TvShowEpisode.h"
#include "media_center/kodi/EpisodeXmlReader.h"
#include "media_center/kodi/EpisodeXmlWriter.h"
#include "settings/Settings.h"
#include "test/helpers/resource_dir.h"

#include <QDateTime>
#include <QDomDocument>
#include <chrono>
#include <memory>
#include <vector>

using namespace std::chrono_literals;

/// Reads a file, parses it, executes callback (you can add further checks), then
/// writes the contents to a temporary file and compares the created file with the
/// reference file.
template<class Callback>
static void createAndCompareSingleEpisode(const QString& filename, Callback callback)
{
    CAPTURE(filename);

    TvShowEpisode episode;
    QString episodeContent = test::readResourceFile(filename);

    mediaelch::kodi::EpisodeXmlReader reader(episode);
    QDomDocument doc;
    doc.setContent(episodeContent);
    reader.parseNfoDom(doc.elementsByTagName("episodedetails").at(0).toElement());

    mediaelch::kodi::EpisodeXmlWriterGeneric writer(mediaelch::KodiVersion(18), {&episode});
    const QString actual = writer.getEpisodeXmlWithSingleRoot(true).trimmed();
    test::compareXmlOrUpdateRef(episodeContent, actual, filename);

    callback(episode);
}

template<class Callback>
static void createAndCompareMultiEpisode(const QString& filename, Callback callback)
{
    CAPTURE(filename);

    std::vector<std::unique_ptr<TvShowEpisode>> episodes;
    QVector<TvShowEpisode*> episodesPointer;
    const QString episodeContent = test::readResourceFile(filename);

    QDomDocument doc;
    doc.setContent(episodeContent);
    QDomNodeList detailTags = doc.elementsByTagName("episodedetails");

    for (int i = 0; i < detailTags.size(); ++i) {
        episodes.push_back(std::make_unique<TvShowEpisode>());
        episodesPointer.push_back(episodes.back().get());

        mediaelch::kodi::EpisodeXmlReader reader(*episodesPointer.last());
        reader.parseNfoDom(detailTags.at(i).toElement());
    }

    callback(episodesPointer);

    mediaelch::kodi::EpisodeXmlWriterGeneric writer(mediaelch::KodiVersion(18), episodesPointer);
    const QString actual = writer.getEpisodeXmlWithSingleRoot(true).trimmed();
    test::compareXmlOrUpdateRef(episodeContent, actual, filename);
}


TEST_CASE("Episode XML writer for Kodi v18", "[data][tvshow][kodi][nfo]")
{
    // required for consistent test runs
    Settings::instance()->setUsePlotForOutline(false);

    SECTION("empty episode")
    {
        TvShowEpisode episode;
        QString filename = "show/kodi_v18_episode_empty.nfo";
        CAPTURE(filename);

        mediaelch::kodi::EpisodeXmlWriterGeneric writer(mediaelch::KodiVersion(18), {&episode});
        QString actual = writer.getEpisodeXmlWithSingleRoot(true).trimmed();
        test::compareXmlOrUpdateRef(test::readResourceFile(filename), actual, filename);
    }

    SECTION("empty multi episode")
    {
        TvShowEpisode episode1;
        TvShowEpisode episode2;
        QString filename = "show/kodi_v18_episode_multi_empty.nfo";
        CAPTURE(filename);

        mediaelch::kodi::EpisodeXmlWriterGeneric writer(mediaelch::KodiVersion(18), {&episode1, &episode2});
        QString actual = writer.getEpisodeXmlWithSingleRoot(true).trimmed();
        test::compareXmlOrUpdateRef(test::readResourceFile(filename), actual, filename);
    }

    SECTION("read / write details: empty episode")
    {
        using mediaelch::kodi::EpisodeXmlReader;

        TvShowEpisode episode;
        const QString filename = "show/kodi_v18_episode_empty.nfo";
        const QString episodeContent = test::readResourceFile(filename);
        CAPTURE(filename);

        EpisodeXmlReader reader(episode);

        QDomDocument doc;
        doc.setContent(episodeContent);
        reader.parseNfoDom(doc.elementsByTagName("episodedetails").at(0).toElement());

        mediaelch::kodi::EpisodeXmlWriterGeneric writer(mediaelch::KodiVersion(18), {&episode});
        QString actual = writer.getEpisodeXmlWithSingleRoot(true).trimmed();
        test::compareXmlOrUpdateRef(episodeContent, actual, filename);
    }

    SECTION("read / write details: American Dad - single episode")
    {
        const QString filename = "show/kodi_v18_episode_American_Dad_S02E01.nfo";
        createAndCompareSingleEpisode(filename, [](TvShowEpisode& episode) {
            // check some details
            CHECK(episode.title() == "Camp Refoogee");
            CHECK(episode.certification() == Certification("TV-14"));
            CHECK(episode.actors().size() == 6);
            CHECK(episode.seasonNumber() == SeasonNumber(2));
            CHECK(episode.episodeNumber() == EpisodeNumber(1));
        });
    }

    SECTION("read / write details: American Dad - multi-episode")
    {
        QString filename = "show/kodi_v18_episode_American_Dad_S02E03-S02E04.nfo";
        createAndCompareMultiEpisode(filename, [](const QVector<TvShowEpisode*>& episodes) {
            // check some details
            REQUIRE(episodes.size() == 2);
            {
                TvShowEpisode* e = episodes.at(0);
                CHECK(e->title() == "Con Heir");
                CHECK(e->actors().size() == 11);
            }
            {
                TvShowEpisode* e = episodes.at(1);
                CHECK(e->title() == "All About Steve");
                CHECK(e->actors().size() == 16);
            }
        });
    }
}
