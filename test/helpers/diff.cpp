#include "test/helpers/diff.h"

#include "test/helpers/reference_file.h"
#include "test/helpers/resource_dir.h"

#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
#include "media_center/KodiVersion.h"
#include "media_center/kodi/MovieXmlWriter.h"

#include "third_party/catch2/catch.hpp"

#include <QtGlobal>

static QString toXml(Movie& movie)
{
    using namespace mediaelch;
    auto writer = std::make_unique<kodi::MovieXmlWriterGeneric>(KodiVersion::v19, movie);
    writer->setWriteThumbUrlsToNfo(true);
    return writer->getMovieXml(true);
}

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

void compareAgainstReference(const Concert& concert, QString filename)
{
    if (!filename.endsWith(".ref.txt")) {
        filename += ".ref.txt";
    }
    compareStringAgainstResourceFile(serializeForReference(concert), filename);
}

void compareAgainstReference(Movie& movie, QString filename)
{
    if (!filename.endsWith(".xml")) {
        filename += ".xml";
    }
    compareXmlAgainstResourceFile(toXml(movie), filename);
}

} // namespace test
