#include "ConcertXmlWriter.h"

#include "data/Concert.h"
#include "globals/Helper.h"
#include "mediaCenterPlugins/XbmcXml.h"
#include "settings/Settings.h"

#include <QDomDocument>

namespace Kodi {

ConcertXmlWriter::ConcertXmlWriter(Concert &concert) : m_concert{concert}
{
}

QByteArray ConcertXmlWriter::getConcertXml()
{
    using namespace std::chrono_literals;

    QDomDocument doc;
    doc.setContent(m_concert.nfoContent());
    if (m_concert.nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8" standalone="yes")");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("musicvideo"));
    }

    QDomElement concertElem = doc.elementsByTagName("musicvideo").at(0).toElement();

    XbmcXml::setTextValue(doc, "title", m_concert.name());
    XbmcXml::setTextValue(doc, "artist", m_concert.artist());
    XbmcXml::setTextValue(doc, "album", m_concert.album());
    XbmcXml::setTextValue(doc, "id", m_concert.imdbId().toString());
    XbmcXml::setTextValue(doc, "tmdbid", m_concert.tmdbId().toString());
    XbmcXml::setTextValue(doc, "rating", QString("%1").arg(m_concert.rating()));
    XbmcXml::setTextValue(doc, "year", m_concert.released().toString("yyyy"));
    XbmcXml::setTextValue(doc, "plot", m_concert.overview());
    XbmcXml::setTextValue(doc, "outline", m_concert.overview());
    XbmcXml::setTextValue(doc, "tagline", m_concert.tagline());
    if (m_concert.runtime() > 0min) {
        XbmcXml::setTextValue(doc, "runtime", QString::number(m_concert.runtime().count()));
    }
    XbmcXml::setTextValue(doc, "mpaa", m_concert.certification());
    XbmcXml::setTextValue(doc, "playcount", QString("%1").arg(m_concert.playcount()));
    XbmcXml::setTextValue(doc, "lastplayed", m_concert.lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    XbmcXml::setTextValue(doc, "trailer", Helper::instance()->formatTrailerUrl(m_concert.trailer().toString()));
    XbmcXml::setTextValue(doc, "watched", (m_concert.watched()) ? "true" : "false");
    XbmcXml::setTextValue(doc, "genre", m_concert.genres().join(" / "));
    XbmcXml::setListValue(doc, "tag", m_concert.tags());

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        XbmcXml::removeChildNodes(doc, "thumb");
        XbmcXml::removeChildNodes(doc, "fanart");

        foreach (const Poster &poster, m_concert.posters()) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            XbmcXml::appendXmlNode(doc, elem);
        }

        if (!m_concert.backdrops().isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            foreach (const Poster &poster, m_concert.backdrops()) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());
                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            XbmcXml::appendXmlNode(doc, fanartElem);
        }
    }

    XbmcXml::writeStreamDetails(doc, m_concert.streamDetails());

    return doc.toByteArray(4);
}

} // namespace Kodi
