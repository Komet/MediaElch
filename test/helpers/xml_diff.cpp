#include "test/helpers/xml_diff.h"

#include "test/helpers/debug_output.h"

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

void checkSameXml(const QString& expected, const QString& actual)
{
    QDomDocument expectedDoc = parseXml(expected);
    QDomDocument actualDoc = parseXml(actual);
    diffDom(expectedDoc, actualDoc);
}
