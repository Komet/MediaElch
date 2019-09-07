#include "media_centers/kodi/v16/AlbumXmlWriterV16.h"

#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "music/Album.h"
#include "settings/Settings.h"

#include <QDomDocument>

namespace mediaelch {
namespace kodi {

AlbumXmlWriterV16::AlbumXmlWriterV16(Album& album) : m_album{album}
{
}

QByteArray AlbumXmlWriterV16::getAlbumXml()
{
    QDomDocument doc;
    doc.setContent(m_album.nfoContent());
    if (m_album.nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8" standalone="yes" )");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("album"));
    }

    QDomElement albumElem = doc.elementsByTagName("album").at(0).toElement();

    if (!m_album.mbReleaseGroupId().isEmpty()) {
        KodiXml::setTextValue(doc, "musicBrainzReleaseGroupID", m_album.mbReleaseGroupId());
    } else {
        KodiXml::removeChildNodes(doc, "musicBrainzReleaseGroupID");
    }
    if (!m_album.mbAlbumId().isEmpty()) {
        KodiXml::setTextValue(doc, "musicBrainzAlbumID", m_album.mbAlbumId());
    } else {
        KodiXml::removeChildNodes(doc, "musicBrainzAlbumID");
    }
    if (!m_album.allMusicId().isEmpty()) {
        KodiXml::setTextValue(doc, "allmusicid", m_album.allMusicId());
    } else {
        KodiXml::removeChildNodes(doc, "allmusicid");
    }
    KodiXml::setTextValue(doc, "title", m_album.title());
    KodiXml::setTextValue(doc, "artist", m_album.artist());
    KodiXml::setTextValue(doc, "genre", m_album.genres().join(" / "));
    KodiXml::setListValue(doc, "style", m_album.styles());
    KodiXml::setListValue(doc, "mood", m_album.moods());
    KodiXml::setTextValue(doc, "review", m_album.review());
    KodiXml::setTextValue(doc, "label", m_album.label());
    KodiXml::setTextValue(doc, "releasedate", m_album.releaseDate());
    if (m_album.rating() > 0) {
        KodiXml::setTextValue(doc, "rating", QString("%1").arg(m_album.rating()));
    } else {
        KodiXml::removeChildNodes(doc, "rating");
    }
    if (m_album.year() > 0) {
        KodiXml::setTextValue(doc, "year", QString("%1").arg(m_album.year()));
    } else {
        KodiXml::removeChildNodes(doc, "year");
    }

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        KodiXml::removeChildNodes(doc, "thumb");

        for (const Poster& poster : m_album.images(ImageType::AlbumThumb)) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            KodiXml::appendXmlNode(doc, elem);
        }
    }

    return doc.toByteArray(4);
}

} // namespace kodi
} // namespace mediaelch
