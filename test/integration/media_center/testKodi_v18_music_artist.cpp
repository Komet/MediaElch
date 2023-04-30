#include "test/test_helpers.h"

#include "data/music/Artist.h"
#include "media_center/kodi/ArtistXmlReader.h"
#include "media_center/kodi/ArtistXmlWriter.h"
#include "test/helpers/resource_dir.h"

#include <QDateTime>
#include <QDomDocument>
#include <chrono>

using namespace std::chrono_literals;

/// Reads a file, parses it, executes callback (you can add further checks), then
/// writes the file to a temporary file and compares the created file with the
/// reference file.
template<class Callback>
static void createAndCompareArtist(const QString& filename, Callback callback)
{
    CAPTURE(filename);

    Artist artist;

    mediaelch::kodi::ArtistXmlReader reader(artist);
    QDomDocument doc;
    doc.setContent(test::readResourceFile(filename));
    CHECK(reader.parseNfoDom(doc));

    mediaelch::kodi::ArtistXmlWriterGeneric writer(mediaelch::KodiVersion(18), artist);
    QString actual = writer.getArtistXml(true).trimmed();
    test::compareXmlAgainstResourceFile(actual, filename);

    callback(artist);
}

TEST_CASE("Music Artist XML writer for Kodi v18", "[data][music][artist][kodi]")
{
    SECTION("Empty artist")
    {
        Artist artist;
        QString filename = "music/artist/kodi_v18_music_artist_empty.nfo";
        CAPTURE(filename);

        mediaelch::kodi::ArtistXmlWriterGeneric writer(mediaelch::KodiVersion(18), artist);
        QString actual = writer.getArtistXml(true).trimmed();
        test::compareXmlAgainstResourceFile(actual, filename);
    }

    SECTION("read / write details: AC/DC")
    {
        createAndCompareArtist("music/artist/kodi_v18_music_artist_AC_DC.nfo", [](Artist& artist) {
            // check some details
            CHECK(artist.name() == "AC/DC");
            {
                CAPTURE(artist.genres());
                CHECK(artist.genres().size() == 2);
                CHECK(artist.genres().contains("Hard Rock"));
                CHECK(artist.genres().contains("Rock"));
            }
            {
                CAPTURE(artist.moods());
                CHECK(artist.moods().size() == 2);
                CHECK(artist.moods().contains("Energetic"));
                CHECK(artist.moods().contains("Really Energetic"));
            }
            {
                CAPTURE(artist.styles());
                CHECK(artist.styles().size() == 2);
                CHECK(artist.styles().contains("Rock/Pop"));
                CHECK(artist.styles().contains("Pop/Rock"));
            }
            // @todo Allow multiple yearsActive tags
            // {
            //     CAPTURE(artist.yearsActive());
            //     CHECK(artist.yearsActive().size() == 2);
            //     CHECK(artist.yearsActive().contains("TODO"));
            //     CHECK(artist.yearsActive().contains("TODO"));
            // }
        });
    }
}
