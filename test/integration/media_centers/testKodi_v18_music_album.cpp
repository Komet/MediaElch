#include "test/test_helpers.h"

#include "media_centers/kodi/AlbumXmlReader.h"
#include "media_centers/kodi/AlbumXmlWriter.h"
#include "music/Album.h"
#include "test/integration/resource_dir.h"

#include <QDateTime>
#include <QDomDocument>
#include <chrono>

using namespace std::chrono_literals;

/// Reads a file, parses it, executes callback (you can add further checks), then
/// writes the file to a temporary file and compares the created file with the
/// reference file.
template<class Callback>
static void createAndCompareAlbum(const QString& filename, Callback callback)
{
    CAPTURE(filename);

    Album album;
    QString albumContent = getFileContent(filename);

    mediaelch::kodi::AlbumXmlReader reader(album);
    QDomDocument doc;
    doc.setContent(albumContent);
    reader.parseNfoDom(doc);

    callback(album);

    mediaelch::kodi::AlbumXmlWriterGeneric writer(mediaelch::KodiVersion(18), album);
    QString actual = writer.getAlbumXml(true).trimmed();
    writeTempFile(filename, actual);
    checkSameXml(albumContent, actual);
}

TEST_CASE("Music Album XML writer for Kodi v18", "[data][music][album][kodi][nfo]")
{
    SECTION("Empty album")
    {
        Album album;
        QString filename = "music/album/kodi_v18_music_album_empty.nfo";
        CAPTURE(filename);

        mediaelch::kodi::AlbumXmlWriterGeneric writer(mediaelch::KodiVersion(18), album);
        QString actual = writer.getAlbumXml(true).trimmed();
        writeTempFile(filename, actual);
        checkSameXml(getFileContent(filename), actual);
    }

    SECTION("read / write details: High Voltage")
    {
        createAndCompareAlbum("music/album/kodi_v18_music_album_High_Voltage.nfo", [](Album& album) {
            // check some details
            CHECK(album.title() == "High Voltage");
            {
                CAPTURE(album.genres());
                CHECK(album.genres().size() == 2);
                CHECK(album.genres().contains("Hard Rock"));
                CHECK(album.genres().contains("Rock"));
            }
            {
                CAPTURE(album.moods());
                CHECK(album.moods().size() == 2);
                CHECK(album.moods().contains("Energetic"));
                CHECK(album.moods().contains("Really Energetic"));
            }
            {
                CAPTURE(album.styles());
                CHECK(album.styles().size() == 2);
                CHECK(album.styles().contains("Rock/Pop"));
                CHECK(album.styles().contains("Pop/Rock"));
            }
        });
    }

    SECTION("read / write details: High Voltage")
    {
        createAndCompareAlbum("music/album/kodi_v18_music_album_Highway_to_Hell.nfo", [](Album& album) {
            // check some details
            CHECK(album.title() == "Highway to Hell");
        });
    }
}
