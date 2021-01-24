#include "media_centers/kodi/ArtistXmlWriter.h"

#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "music/Artist.h"

#include <QDomDocument>

namespace mediaelch {
namespace kodi {

ArtistXmlWriterGeneric::ArtistXmlWriterGeneric(KodiVersion version, Artist& artist) :
    ArtistXmlWriter(std::move(version)), m_artist{artist}
{
}

QByteArray ArtistXmlWriterGeneric::getArtistXml(bool testMode)
{
    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);

    xml.writeStartElement("artist");

    writeArtistTags(xml);

    if (!testMode) {
        addMediaelchGeneratorTag(xml);
    }

    xml.writeEndElement();
    xml.writeEndDocument();
    return xmlContent;
}

void ArtistXmlWriterGeneric::writeArtistTags(QXmlStreamWriter& xml)
{
    if (m_artist.mbId().isValid()) {
        xml.writeTextElement("musicBrainzArtistID", m_artist.mbId().toString());
    }

    if (m_artist.allMusicId().isValid()) {
        xml.writeTextElement("allmusicid", m_artist.allMusicId().toString());
    }

    xml.writeTextElement("name", m_artist.name());
    KodiXml::writeStringsAsOneTagEach(xml, "genre", m_artist.genres());
    KodiXml::writeStringsAsOneTagEach(xml, "style", m_artist.styles());
    KodiXml::writeStringsAsOneTagEach(xml, "mood", m_artist.moods());
    xml.writeTextElement("type", "artist"); // Kodi MediaType
    xml.writeTextElement("yearsactive", m_artist.yearsActive());
    xml.writeTextElement("formed", m_artist.formed());
    xml.writeTextElement("biography", m_artist.biography());
    xml.writeTextElement("born", m_artist.born());
    xml.writeTextElement("died", m_artist.died());
    xml.writeTextElement("disbanded", m_artist.disbanded());

    if (writeThumbUrlsToNfo()) {
        const auto& posters = m_artist.images(ImageType::ArtistThumb);
        for (const Poster& poster : posters) {
            xml.writeStartElement("thumb");

            QString aspect = poster.aspect.isEmpty() ? "thumb" : poster.aspect;
            xml.writeAttribute("aspect", aspect);
            xml.writeAttribute("preview", poster.thumbUrl.toString());

            xml.writeCharacters(poster.originalUrl.toString());
            xml.writeEndElement();
        }

        if (!m_artist.images(ImageType::ArtistFanart).isEmpty()) {
            xml.writeStartElement("fanart");
            const auto& images = m_artist.images(ImageType::ArtistFanart);
            for (const Poster& poster : images) {
                xml.writeStartElement("thumb");
                xml.writeAttribute("preview", poster.thumbUrl.toString());
                xml.writeCharacters(poster.originalUrl.toString());
                xml.writeEndElement();
            }
            xml.writeEndElement();
        }
    }

    const auto& albums = m_artist.discographyAlbums();
    for (const DiscographyAlbum& album : albums) {
        xml.writeStartElement("album");
        xml.writeTextElement("title", album.title);
        xml.writeTextElement("year", album.year);
        xml.writeEndElement();
    }
}

} // namespace kodi
} // namespace mediaelch
