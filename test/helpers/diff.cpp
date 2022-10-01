#include "test/helpers/diff.h"

#include "test/helpers/reference_file.h"
#include "test/helpers/resource_dir.h"

#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "media_center/KodiVersion.h"

#include "third_party/catch2/catch.hpp"

#include <QtGlobal>

namespace test {

QDomDocument parseXml(const QString& content)
{
    QString errorMsg;
    int errorLine = -1;
    int errorColumn = -1;

    QDomDocument doc;
    doc.setContent(content, false, &errorMsg, &errorLine, &errorColumn);

    // CAPTURE(content);
    CAPTURE(errorMsg);
    CAPTURE(errorLine);
    CAPTURE(errorColumn);

    REQUIRE(errorLine == -1);
    REQUIRE(errorColumn == -1);

    return doc;
}

void compareStringAgainstResourceFile(const QString& actual, const QString& filename)
{
    CAPTURE(filename);

    const QString expected = test::readResourceFile(filename);

    if (expected != actual && shouldUpdateResourceFiles()) {
        test::writeResourceFile(filename, actual);

    } else {
        CHECK(expected == actual);
    }
}

void compareXmlAgainstResourceFile(const QString& actual, const QString& filename)
{
    CAPTURE(filename);

    const QString expected = test::readResourceFile(filename);

    // TODO: Use XML aware comparison
    QString expectedDoc = expected.isEmpty() ? "" : parseXml(expected).toString();
    QString actualDoc = parseXml(actual).toString();

    if (expectedDoc != actualDoc && shouldUpdateResourceFiles()) {
        writeResourceFile(filename, actual);

    } else {
        CHECK(expectedDoc == actualDoc);
    }
}

namespace scraper {

template<class Data>
static void compareDataAgainstReference(Data& data, QString filename)
{
    if (!filename.endsWith(".ref.txt")) {
        filename += ".ref.txt";
    }
    compareStringAgainstResourceFile(serializeForReference(data), filename);
}

void compareAgainstReference(Concert& concert, QString filename)
{
    compareDataAgainstReference(concert, std::move(filename));
}

void compareAgainstReference(Movie& movie, QString filename)
{
    compareDataAgainstReference(movie, std::move(filename));
}

void compareAgainstReference(Album& album, QString filename)
{
    compareDataAgainstReference(album, std::move(filename));
}

void compareAgainstReference(Artist& artist, QString filename)
{
    compareDataAgainstReference(artist, std::move(filename));
}

void compareAgainstReference(TvShow& show, QString filename)
{
    compareDataAgainstReference(show, std::move(filename));
}

void compareAgainstReference(TvShowEpisode& episode, QString filename)
{
    compareDataAgainstReference(episode, std::move(filename));
}

} // namespace scraper
} // namespace test
