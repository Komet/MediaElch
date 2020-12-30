#include "media_centers/kodi/KodiNfoMeta.h"

#include "Version.h"

#include <QDateTime>

namespace mediaelch {
namespace kodi {

void addMediaelchGeneratorTag(QDomDocument& doc, KodiVersion::Version kodiVersion)
{
    QDomNode rootNode = doc.firstChild();
    while ((rootNode.nodeName() == "xml" || rootNode.isComment()) && !rootNode.isNull()) {
        rootNode = rootNode.nextSibling();
    }

    // Remove existing "generator" tags
    QDomNodeList childNodes = rootNode.childNodes();
    QVector<QDomNode> nodesToRemove;
    for (int i = 0, n = childNodes.count(); i < n; ++i) {
        if (childNodes.at(i).nodeName() == "generator") {
            nodesToRemove.append(childNodes.at(i));
        }
    }
    for (const QDomNode& node : nodesToRemove) {
        rootNode.removeChild(node);
    }

    // Proposed here:
    // https://forum.kodi.tv/showthread.php?tid=347109
    //
    // <generator>
    //     <appname>MediaElch</appname>              <!-- can be any string -->
    //     <appversion>2.6.1-dev</appversion>        <!-- can be any string -->
    //     <appdata>meta data or similar</appdata>   <!-- can be any data, e.g. multiline string, more tags, etc. -->
    //     <kodiversion>18.4</kodiversion>           <!-- numerical representation of the target kodi version -->
    //     <datetime>2019-09-09 11:04:10</datetime>  <!-- generation time -->
    // </generator>

    QDomElement generator = doc.createElement("generator");

    QDomElement appName = doc.createElement("appname");
    appName.appendChild(doc.createTextNode(mediaelch::constants::AppName));

    QDomElement appVersion = doc.createElement("appversion");
    appVersion.appendChild(doc.createTextNode(mediaelch::constants::AppVersionFullStr));

    // TODO: Enable when the NFO writers are restructured. Currently there is a lot of
    // duplicated code and v18 is just an alias for the v17 writer.
    Q_UNUSED(kodiVersion)
    // QDomElement kodiTarget = doc.createElement("kodiversion");
    // kodiTarget.appendChild(doc.createTextNode(KodiVersion(kodiVersion).toString()));

    QDomElement dateTime = doc.createElement("datetime");
    dateTime.appendChild(doc.createTextNode(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)));

    generator.appendChild(appName);
    generator.appendChild(appVersion);
    // generator.appendChild(kodiTarget);
    generator.appendChild(dateTime);

    rootNode.appendChild(generator);
}

void addMediaelchGeneratorTag(QXmlStreamWriter& xml, KodiVersion::Version kodiVersion)
{
    xml.writeStartElement("generator");
    xml.writeTextElement("appname", mediaelch::constants::AppName);
    xml.writeTextElement("appversion", mediaelch::constants::AppVersionFullStr);
    // TODO: Enable when the NFO writers are restructured. Currently there is a lot of
    // duplicated code and v18 is just an alias for the v17 writer.
    Q_UNUSED(kodiVersion)
    // xml.writeTextElement("kodiversion", KodiVersion(kodiVersion).toString());
    xml.writeTextElement("datetime", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    xml.writeEndElement();
}

} // namespace kodi
} // namespace mediaelch
