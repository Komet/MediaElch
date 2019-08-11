#include "media_centers/kodi/v16/ConcertXmlWriterV16.h"

#include "concerts/Concert.h"
#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "settings/Settings.h"

#include <QDomDocument>

namespace mediaelch {
namespace kodi {

ConcertXmlWriterV16::ConcertXmlWriterV16(Concert& concert) : m_concert{concert}
{
}

QByteArray ConcertXmlWriterV16::getConcertXml()
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

    KodiXml::setTextValue(doc, "title", m_concert.name());
    KodiXml::setTextValue(doc, "artist", m_concert.artist());
    KodiXml::setTextValue(doc, "album", m_concert.album());
    KodiXml::setTextValue(doc, "id", m_concert.imdbId().toString());
    KodiXml::setTextValue(doc, "tmdbid", m_concert.tmdbId().toString());
    if (!m_concert.ratings().isEmpty()) {
        KodiXml::setTextValue(doc, "rating", QString("%1").arg(m_concert.ratings().first().rating));
    }
    KodiXml::setTextValue(doc, "year", m_concert.released().toString("yyyy"));
    KodiXml::setTextValue(doc, "plot", m_concert.overview());
    KodiXml::setTextValue(doc, "outline", m_concert.overview());
    KodiXml::setTextValue(doc, "tagline", m_concert.tagline());
    if (m_concert.runtime() > 0min) {
        KodiXml::setTextValue(doc, "runtime", QString::number(m_concert.runtime().count()));
    }
    KodiXml::setTextValue(doc, "mpaa", m_concert.certification().toString());
    KodiXml::setTextValue(doc, "playcount", QString("%1").arg(m_concert.playcount()));
    KodiXml::setTextValue(doc, "lastplayed", m_concert.lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    KodiXml::setTextValue(doc, "trailer", helper::formatTrailerUrl(m_concert.trailer().toString()));
    KodiXml::setTextValue(doc, "watched", (m_concert.watched()) ? "true" : "false");
    KodiXml::setTextValue(doc, "genre", m_concert.genres().join(" / "));
    KodiXml::setListValue(doc, "tag", m_concert.tags());

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        KodiXml::removeChildNodes(doc, "thumb");
        KodiXml::removeChildNodes(doc, "fanart");

        for (const Poster& poster : m_concert.posters()) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            KodiXml::appendXmlNode(doc, elem);
        }

        if (!m_concert.backdrops().isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            for (const Poster& poster : m_concert.backdrops()) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());
                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            KodiXml::appendXmlNode(doc, fanartElem);
        }
    }

    KodiXml::writeStreamDetails(doc, m_concert.streamDetails());

    return doc.toByteArray(4);
}

} // namespace kodi
} // namespace mediaelch
