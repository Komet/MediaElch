#include "test/helpers/xml_diff.h"

#include "test/helpers/debug_output.h"
#include "test/helpers/resource_dir.h"

#include <QtGlobal>

QDomDocument parseXml(const QString& content)
{
    QString errorMsg;
    int errorLine = -1;
    int errorColumn = -1;

    QDomDocument doc;
    doc.setContent(content, false, &errorMsg, &errorLine, &errorColumn);

    CAPTURE(errorMsg);
    CAPTURE(errorLine);
    CAPTURE(errorColumn);

    REQUIRE(errorLine == -1);
    REQUIRE(errorColumn == -1);

    return doc;
}

void compareXmlOrUpdateRef(const QString& expected, const QString& actual, const QString& filename)
{
    QDomDocument expectedDoc = parseXml(expected);
    QDomDocument actualDoc = parseXml(actual);

    if (!filename.isEmpty() && expectedDoc.toString() != actualDoc.toString() && !qgetenv("MEDIAELCH_UPDATE_REF_FILES").isEmpty()) {
        writeResourceFile(filename, actual);

    } else {
        CHECK(expectedDoc.toString() == actualDoc.toString());
    }
}
