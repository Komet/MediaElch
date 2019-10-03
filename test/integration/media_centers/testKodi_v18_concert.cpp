#include "test/test_helpers.h"

#include "concerts/Concert.h"
#include "media_centers/KodiXml.h"
#include "media_centers/kodi/ConcertXmlReader.h"
#include "media_centers/kodi/v18/ConcertXmlWriterV18.h"
#include "test/integration/resource_dir.h"

#include <QDomDocument>
#include <chrono>

using namespace std::chrono_literals;

/// Reads a file, parses it, executes callback (you can add further checks), then
/// writes the file to a temporary file and compares the created file with the
/// reference file.
template<class Callback>
static void createAndCompareConcert(const QString& filename, Callback callback)
{
    CAPTURE(filename);

    Concert concert;
    QString concertContent = getFileContent(filename);

    mediaelch::kodi::ConcertXmlReader reader(concert);
    QDomDocument doc;
    doc.setContent(concertContent);
    reader.parseNfoDom(doc);

    callback(concert);

    mediaelch::kodi::ConcertXmlWriterV18 writer(concert);
    QString actual = writer.getConcertXml().trimmed();
    writeTempFile(filename, actual);
    checkSameXml(concertContent, actual);
}

TEST_CASE("Concert XML writer for Kodi v18", "[data][concert][kodi][nfo]")
{
    SECTION("Empty concert")
    {
        Concert concert;
        QString filename = "concert/kodi_v18_concert_empty.nfo";
        CAPTURE(filename);

        mediaelch::kodi::ConcertXmlWriterV18 writer(concert);
        QString actual = writer.getConcertXml().trimmed();
        writeTempFile(filename, actual);
        checkSameXml(getFileContent(filename), actual);
    }

    // TODO: Deactivated due to too many changes
    // SECTION("read / write details: Rammstein in Amerika 2015")
    // {
    //     createAndCompareConcert("concert/kodi_v18_Rammstein_in_Amerika_2015.nfo", [](Concert& concert) {
    //         // check some details
    //         CHECK(concert.name() == "Rammstein in Amerika");
    //         REQUIRE(!concert.ratings().isEmpty());
    //         CHECK(concert.ratings().first().voteCount == 7653);
    //         CHECK(concert.image(ImageType::ConcertPoster).size() == 176);  // TODO: currently every thumb is a
    //         poster... CHECK(concert.image(ImageType::ConcertBackdrop).size() == 57); // <fanart>
    //         CHECK(concert.certification() == Certification("Rated R"));
    //         CHECK(concert.artist() == "Rammstein");
    //     });
    // }
}
