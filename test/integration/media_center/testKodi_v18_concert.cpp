#include "test/test_helpers.h"

#include "data/concert/Concert.h"
#include "media_center/KodiXml.h"
#include "media_center/kodi/ConcertXmlReader.h"
#include "media_center/kodi/ConcertXmlWriter.h"
#include "test/helpers/resource_dir.h"

#include <QDomDocument>
#include <chrono>

using namespace std::chrono_literals;

static QString concertToXml(mediaelch::KodiVersion version, const Concert& concert)
{
    auto writer = std::make_unique<mediaelch::kodi::ConcertXmlWriterGeneric>(version, concert);
    writer->setWriteThumbUrlsToNfo(true);
    return writer->getConcertXml(true);
}

template<class Callback>
static void createAndCompareConcert(const QString& filename, Callback callback)
{
    CAPTURE(filename);

    Concert concert;

    mediaelch::kodi::ConcertXmlReader reader(concert);
    QXmlStreamReader stream(test::readResourceFile(filename));
    reader.parse(stream);

    const QString actual = concertToXml(mediaelch::KodiVersion::v18, concert);
    test::compareXmlAgainstResourceFile(actual, filename);

    callback(concert);
}

TEST_CASE("Concert XML writer for Kodi v18", "[data][concert][kodi][nfo]")
{
    SECTION("Empty concert")
    {
        const QString actual = concertToXml(mediaelch::KodiVersion::v18, Concert{});
        test::compareXmlAgainstResourceFile(actual, "concert/kodi_v18_concert_empty.nfo");
    }

    SECTION("read / write details: Rammstein in Amerika 2015")
    {
        createAndCompareConcert("concert/kodi_v18_Rammstein_in_Amerika_2015.nfo", [](Concert& concert) {
            // check some details
            CHECK(concert.title() == "Rammstein in Amerika");
            CHECK(concert.originalTitle() == "RAMMSTEIN in Amerika");
            REQUIRE(!concert.ratings().isEmpty());
            CHECK(concert.ratings().first().voteCount == 15);
            // TODO: Difference to posters()?
            // CHECK(concert.image(ImageType::ConcertPoster).size() == 176);  // TODO: currently every thumb is a
            // poster...
            CHECK(concert.posters().size() == 4);
            // TODO: Difference to backdrops()?
            // CHECK(concert.image(ImageType::ConcertBackdrop).size() == 57); // <fanart>
            CHECK(concert.backdrops().size() == 5);
            CHECK(concert.certification() == Certification("Rated R"));
            REQUIRE(concert.artists().size() == 2);
            CHECK(concert.artists().at(0) == "Rammstein");
            CHECK(concert.artists().at(1) == "Rammstein2");
        });
    }
}
