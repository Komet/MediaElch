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

void diffDom(const QDomDocument& expected, const QDomDocument& actual)
{
    CHECK(expected.toString() == actual.toString());
}

void compareXmlOrUpdateRef(const QString& expected, const QString& actual, const QString& filename)
{
    if (!filename.isEmpty() && !qgetenv("MEDIAELCH_UPDATE_REF_FILES").isEmpty()) {
        writeResourceFile(filename, actual);

    } else {
        QDomDocument expectedDoc = parseXml(expected);
        QDomDocument actualDoc = parseXml(actual);
        diffDom(expectedDoc, actualDoc);
    }
}
