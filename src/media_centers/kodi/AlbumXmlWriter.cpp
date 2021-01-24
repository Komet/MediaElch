#include "media_centers/kodi/AlbumXmlWriter.h"

#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "music/Album.h"

#include <QDomDocument>

namespace mediaelch {
namespace kodi {

AlbumXmlWriterGeneric::AlbumXmlWriterGeneric(KodiVersion version, Album& album) :
    AlbumXmlWriter(std::move(version)), m_album{album}
{
}

QByteArray AlbumXmlWriterGeneric::getAlbumXml(bool testMode)
{
    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);

    xml.writeStartElement("album");

    writeAlbumTags(xml);

    if (!testMode) {
        addMediaelchGeneratorTag(xml);
    }

    xml.writeEndElement();
    xml.writeEndDocument();
    return xmlContent;
}

void AlbumXmlWriterGeneric::writeAlbumTags(QXmlStreamWriter& xml)
{
    if (m_album.mbReleaseGroupId().isValid()) {
        xml.writeTextElement("musicbrainzreleasegroupid", m_album.mbReleaseGroupId().toString());
    }

    if (m_album.mbAlbumId().isValid()) {
        xml.writeTextElement("musicbrainzalbumid", m_album.mbAlbumId().toString());
    }

    if (m_album.allMusicId().isValid()) {
        xml.writeTextElement("allmusicid", m_album.allMusicId().toString());
    }

    xml.writeTextElement("title", m_album.title());
    xml.writeTextElement("artistdesc", m_album.artist());

    const bool hasMbId = (m_album.artistObj() != nullptr && m_album.artistObj()->mbId().isValid());
    xml.writeTextElement("scrapedmbid", hasMbId ? "true" : "false");

    KodiXml::writeStringsAsOneTagEach(xml, "genre", m_album.genres());
    KodiXml::writeStringsAsOneTagEach(xml, "style", m_album.styles());
    KodiXml::writeStringsAsOneTagEach(xml, "mood", m_album.moods());

    xml.writeTextElement("review", m_album.review());
    xml.writeTextElement("type", "album"); // Kodi MediaType
    xml.writeTextElement("label", m_album.label());
    xml.writeTextElement("releasedate", m_album.releaseDate());
    if (m_album.rating() > 0) {
        xml.writeTextElement("rating", QString("%1").arg(m_album.rating()));
    }
    if (m_album.year() > 0) {
        xml.writeTextElement("year", QString("%1").arg(m_album.year()));
    }

    if (writeThumbUrlsToNfo()) {
        const auto& images = m_album.images(ImageType::AlbumThumb);
        for (const Poster& poster : images) {
            xml.writeStartElement("thumb");
            xml.writeAttribute("preview", poster.thumbUrl.toString());
            xml.writeCharacters(poster.originalUrl.toString());
            xml.writeEndElement();
        }
    }

    writeArtistCredits(xml);
}

void AlbumXmlWriterGeneric::writeArtistCredits(QXmlStreamWriter& xml)
{
    // <albumArtistCredits>
    //   <artist>AC/DC</artist>
    //   <musicBrainzArtistID>66c662b6-6e2f-4930-8610-912e24c63ed1</musicBrainzArtistID>
    // </albumArtistCredits>

    xml.writeStartElement("albumArtistCredits");
    xml.writeTextElement("artist", m_album.artist());

    if (m_album.artistObj() != nullptr) {
        xml.writeTextElement("musicBrainzArtistID", m_album.artistObj()->mbId().toString());
    }

    xml.writeEndElement();
}

} // namespace kodi
} // namespace mediaelch
