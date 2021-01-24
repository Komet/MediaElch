#include "media_centers/kodi/KodiXmlWriter.h"

#include "Version.h"

#include <QDateTime>

namespace mediaelch {
namespace kodi {

void KodiXmlWriter::addMediaelchGeneratorTag(QXmlStreamWriter& xml)
{
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

    xml.writeStartElement("generator");
    xml.writeTextElement("appname", mediaelch::constants::AppName);
    xml.writeTextElement("appversion", mediaelch::constants::AppVersionFullStr);
    xml.writeTextElement("kodiversion", m_version.toString());
    xml.writeTextElement("datetime", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    xml.writeEndElement();
}

bool KodiXmlWriter::writeThumbUrlsToNfo() const
{
    return m_writeThumbUrlsToNfo;
}

void KodiXmlWriter::setWriteThumbUrlsToNfo(bool writeThumbUrlsToNfo)
{
    m_writeThumbUrlsToNfo = writeThumbUrlsToNfo;
}

} // namespace kodi
} // namespace mediaelch
