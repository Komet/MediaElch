#include "test/helpers/diff.h"

#include "test/helpers/debug_output.h"
#include "test/helpers/resource_dir.h"

#include "data/concert/Concert.h"
#include "media_center/KodiVersion.h"
#include "media_center/kodi/ConcertXmlWriter.h"

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

static QString toXml(const Concert& concert)
{
    using namespace mediaelch;
    auto writer = std::make_unique<kodi::ConcertXmlWriterGeneric>(KodiVersion::v19, concert);
    writer->setWriteThumbUrlsToNfo(true);
    return writer->getConcertXml(true);
}

void compareAgainstReference(const Concert& concert, QString filename)
{
    if (!filename.endsWith(".xml")) {
        filename += ".xml";
    }
    compareXmlAgainstResourceFile(toXml(concert), filename);
}

} // namespace test
