#include "media_centers/kodi/v17/AlbumXmlWriterV17.h"

#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "music/Album.h"
#include "settings/Settings.h"

#include <QDomDocument>

namespace mediaelch {
namespace kodi {

AlbumXmlWriterV17::AlbumXmlWriterV17(Album& album) : m_album{album}
{
}

QByteArray AlbumXmlWriterV17::getAlbumXml()
{
    QDomDocument doc;
    doc.setContent(m_album.nfoContent());
    if (m_album.nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8" standalone="yes" )");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("album"));
    }

    QDomElement albumElem = doc.elementsByTagName("album").at(0).toElement();

    // remove old v16 tags if they exist
    KodiXml::removeChildNodes(doc, "musicBrainzReleaseGroupID");
    KodiXml::removeChildNodes(doc, "musicBrainzAlbumID");

    if (!m_album.mbReleaseGroupId().isEmpty()) {
        KodiXml::setTextValue(doc, "musicbrainzreleasegroupid", m_album.mbReleaseGroupId());
    } else {
        KodiXml::removeChildNodes(doc, "musicbrainzreleasegroupid");
    }
    if (!m_album.mbAlbumId().isEmpty()) {
        KodiXml::setTextValue(doc, "musicbrainzalbumid", m_album.mbAlbumId());
    } else {
        KodiXml::removeChildNodes(doc, "musicbrainzalbumid");
    }
    if (!m_album.allMusicId().isEmpty()) {
        KodiXml::setTextValue(doc, "allmusicid", m_album.allMusicId());
    } else {
        KodiXml::removeChildNodes(doc, "allmusicid");
    }
    KodiXml::setTextValue(doc, "title", m_album.title());
    KodiXml::setTextValue(doc, "artistdesc", m_album.artist());

    const bool hasMbId = (m_album.artistObj() != nullptr && !m_album.artistObj()->mbId().isEmpty());
    KodiXml::setTextValue(doc, "scrapedmbid", hasMbId ? "true" : "false");

    KodiXml::setListValue(doc, "genre", m_album.genres());
    KodiXml::setListValue(doc, "style", m_album.styles());
    KodiXml::setListValue(doc, "mood", m_album.moods());
    KodiXml::setTextValue(doc, "review", m_album.review());
    KodiXml::setTextValue(doc, "type", "album"); // Kodi MediaType
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

    writeArtistCredits(doc);

    return doc.toByteArray(4);
}

void AlbumXmlWriterV17::writeArtistCredits(QDomDocument& doc)
{
    // <albumArtistCredits>
    //   <artist>AC/DC</artist>
    //   <musicBrainzArtistID>66c662b6-6e2f-4930-8610-912e24c63ed1</musicBrainzArtistID>
    // </albumArtistCredits>
    KodiXml::removeChildNodes(doc, "albumArtistCredits");

    QDomElement creditsElement = doc.createElement("albumArtistCredits");
    QDomElement artistElement = doc.createElement("artist");

    artistElement.appendChild(doc.createTextNode(m_album.artist()));
    creditsElement.appendChild(artistElement);

    if (m_album.artistObj() != nullptr) {
        QDomElement mbIdElement = doc.createElement("musicBrainzArtistID");
        mbIdElement.appendChild(doc.createTextNode(m_album.artistObj()->mbId()));
        creditsElement.appendChild(mbIdElement);
    }
    KodiXml::appendXmlNode(doc, creditsElement);
}

} // namespace kodi
} // namespace mediaelch
