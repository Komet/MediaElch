#include "ArtistXmlWriter.h"

#include "globals/Helper.h"
#include "media_centers/XbmcXml.h"
#include "music/Artist.h"
#include "settings/Settings.h"

#include <QDomDocument>

namespace Kodi {

ArtistXmlWriter::ArtistXmlWriter(Artist& artist) : m_artist{artist}
{
}

QByteArray ArtistXmlWriter::getArtistXml()
{
    QDomDocument doc;
    doc.setContent(m_artist.nfoContent());
    if (m_artist.nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8" standalone="yes")");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("artist"));
    }

    QDomElement artistElem = doc.elementsByTagName("artist").at(0).toElement();

    if (!m_artist.mbId().isEmpty()) {
        XbmcXml::setTextValue(doc, "musicBrainzArtistID", m_artist.mbId());
    } else {
        XbmcXml::removeChildNodes(doc, "musicBrainzArtistID");
    }
    if (!m_artist.allMusicId().isEmpty()) {
        XbmcXml::setTextValue(doc, "allmusicid", m_artist.allMusicId());
    } else {
        XbmcXml::removeChildNodes(doc, "allmusicid");
    }
    XbmcXml::setTextValue(doc, "name", m_artist.name());
    XbmcXml::setTextValue(doc, "genre", m_artist.genres().join(" / "));
    XbmcXml::setListValue(doc, "style", m_artist.styles());
    XbmcXml::setListValue(doc, "mood", m_artist.moods());
    XbmcXml::setTextValue(doc, "yearsactive", m_artist.yearsActive());
    XbmcXml::setTextValue(doc, "formed", m_artist.formed());
    XbmcXml::setTextValue(doc, "biography", m_artist.biography());
    XbmcXml::setTextValue(doc, "born", m_artist.born());
    XbmcXml::setTextValue(doc, "died", m_artist.died());
    XbmcXml::setTextValue(doc, "disbanded", m_artist.disbanded());

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        XbmcXml::removeChildNodes(doc, "thumb");
        XbmcXml::removeChildNodes(doc, "fanart");

        for (const Poster& poster : m_artist.images(ImageType::ArtistThumb)) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            XbmcXml::appendXmlNode(doc, elem);
        }

        if (!m_artist.images(ImageType::ArtistFanart).isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            for (const Poster& poster : m_artist.images(ImageType::ArtistFanart)) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());
                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            XbmcXml::appendXmlNode(doc, fanartElem);
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
            XbmcXml::appendXmlNode(doc, elem);
        }
    }
    for (QDomNode node : albumNodes) {
        artistElem.removeChild(node);
    }

    return doc.toByteArray(4);
}

} // namespace Kodi
