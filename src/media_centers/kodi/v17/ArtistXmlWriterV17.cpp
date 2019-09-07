#include "media_centers/kodi/v17/ArtistXmlWriterV17.h"

#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "music/Artist.h"
#include "settings/Settings.h"

#include <QDomDocument>

namespace mediaelch {
namespace kodi {

ArtistXmlWriterV17::ArtistXmlWriterV17(Artist& artist) : m_artist{artist}
{
}

QByteArray ArtistXmlWriterV17::getArtistXml()
{
    QDomDocument doc;
    doc.setContent(m_artist.nfoContent());
    if (m_artist.nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8" standalone="yes" )");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("artist"));
    }

    QDomElement artistElem = doc.elementsByTagName("artist").at(0).toElement();

    if (!m_artist.mbId().isEmpty()) {
        KodiXml::setTextValue(doc, "musicBrainzArtistID", m_artist.mbId());
    } else {
        KodiXml::removeChildNodes(doc, "musicBrainzArtistID");
    }
    if (!m_artist.allMusicId().isEmpty()) {
        KodiXml::setTextValue(doc, "allmusicid", m_artist.allMusicId());
    } else {
        KodiXml::removeChildNodes(doc, "allmusicid");
    }
    KodiXml::setTextValue(doc, "name", m_artist.name());
    KodiXml::setListValue(doc, "genre", m_artist.genres());
    KodiXml::setListValue(doc, "style", m_artist.styles());
    KodiXml::setListValue(doc, "mood", m_artist.moods());
    KodiXml::setTextValue(doc, "type", "artist"); // Kodi MediaType
    KodiXml::setTextValue(doc, "yearsactive", m_artist.yearsActive());
    KodiXml::setTextValue(doc, "formed", m_artist.formed());
    KodiXml::setTextValue(doc, "biography", m_artist.biography());
    KodiXml::setTextValue(doc, "born", m_artist.born());
    KodiXml::setTextValue(doc, "died", m_artist.died());
    KodiXml::setTextValue(doc, "disbanded", m_artist.disbanded());

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        KodiXml::removeChildNodes(doc, "thumb");
        KodiXml::removeChildNodes(doc, "fanart");

        for (const Poster& poster : m_artist.images(ImageType::ArtistThumb)) {
            QDomElement elem = doc.createElement("thumb");
            QString aspect = poster.aspect.isEmpty() ? "thumb" : poster.aspect;

            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.setAttribute("aspect", aspect);
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            KodiXml::appendXmlNode(doc, elem);
        }

        if (!m_artist.images(ImageType::ArtistFanart).isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            for (const Poster& poster : m_artist.images(ImageType::ArtistFanart)) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());
                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            KodiXml::appendXmlNode(doc, fanartElem);
        }
    }

    QVector<QDomNode> albumNodes;
    QDomNodeList childNodes = artistElem.childNodes();
    for (int i = 0, n = childNodes.count(); i < n; ++i) {
        if (childNodes.at(i).nodeName() == "album") {
            albumNodes.append(childNodes.at(i));
        }
    }

    for (const DiscographyAlbum& album : m_artist.discographyAlbums()) {
        bool nodeFound = false;
        for (QDomNode node : albumNodes) {
            if (!node.toElement().elementsByTagName("title").isEmpty()
                && node.toElement().elementsByTagName("title").at(0).toElement().text() == album.title) {
                albumNodes.removeOne(node);
                if (!node.toElement().elementsByTagName("year").isEmpty()) {
                    if (!node.toElement().elementsByTagName("year").at(0).firstChild().isText()) {
                        QDomText t = doc.createTextNode(album.year);
                        node.toElement().elementsByTagName("year").at(0).appendChild(t);
                    } else {
                        node.toElement().elementsByTagName("year").at(0).firstChild().setNodeValue(album.year);
                    }
                } else {
                    QDomElement elem = doc.createElement("year");
                    elem.appendChild(doc.createTextNode(album.year));
                    node.appendChild(elem);
                }
                nodeFound = true;
                break;
            }
        }
        if (!nodeFound) {
            QDomElement elem = doc.createElement("album");
            QDomElement elemTitle = doc.createElement("title");
            QDomElement elemYear = doc.createElement("year");
            elemTitle.appendChild(doc.createTextNode(album.title));
            elemYear.appendChild(doc.createTextNode(album.year));
            elem.appendChild(elemTitle);
            elem.appendChild(elemYear);
            KodiXml::appendXmlNode(doc, elem);
        }
    }
    for (const QDomNode& node : albumNodes) {
        artistElem.removeChild(node);
    }

    return doc.toByteArray(4);
}

} // namespace kodi
} // namespace mediaelch
