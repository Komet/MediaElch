#include "media_centers/kodi/ConcertXmlWriter.h"

#include "concerts/Concert.h"
#include "globals/Helper.h"
#include "media_centers/KodiXml.h"

#include <QDomDocument>

namespace mediaelch {
namespace kodi {

ConcertXmlWriterGeneric::ConcertXmlWriterGeneric(KodiVersion version, Concert& concert) :
    ConcertXmlWriter(std::move(version)), m_concert{concert}
{
}

QByteArray ConcertXmlWriterGeneric::getConcertXml(bool testMode)
{
    using namespace std::chrono_literals;

    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);

    xml.writeStartElement("musicvideo");

    xml.writeTextElement("title", m_concert.title());
    xml.writeTextElement("originaltitle", m_concert.originalTitle());
    xml.writeTextElement("artist", m_concert.artist());
    xml.writeTextElement("album", m_concert.album());

    // v16 id
    xml.writeTextElement("id", m_concert.imdbId().toString());
    // unique id: IMDb and TMDb
    {
        xml.writeStartElement("uniqueid");
        xml.writeAttribute("type", "imdb");
        xml.writeAttribute("default", "true");
        xml.writeCharacters(m_concert.imdbId().toString());
        xml.writeEndElement();
    }
    if (m_concert.tmdbId().isValid()) {
        xml.writeStartElement("uniqueid");
        xml.writeAttribute("type", "tmdb");
        xml.writeCharacters(m_concert.tmdbId().toString());
        xml.writeEndElement();
    }

    // rating
    xml.writeStartElement("ratings");
    bool firstRating = true;
    for (const Rating& rating : m_concert.ratings()) {
        xml.writeStartElement("rating");
        xml.writeAttribute("name", rating.source);
        xml.writeAttribute("default", firstRating ? "true" : "false");
        if (rating.maxRating > 0) {
            xml.writeAttribute("max", QString::number(rating.maxRating));
        }
        xml.writeTextElement("value", QString::number(rating.rating));
        xml.writeTextElement("votes", QString::number(rating.voteCount));
        xml.writeEndElement();
        firstRating = false;
    }
    xml.writeEndElement();

    xml.writeTextElement("userrating", QString::number(m_concert.userRating()));

    xml.writeTextElement("year", m_concert.released().toString("yyyy"));
    xml.writeTextElement("plot", m_concert.overview());
    // xml.writeTextElement("outline", m_concert.overview());
    xml.writeTextElement("tagline", m_concert.tagline());
    if (m_concert.runtime() > 0min) {
        xml.writeTextElement("runtime", QString::number(m_concert.runtime().count()));
    }
    xml.writeTextElement("mpaa", m_concert.certification().toString());
    xml.writeTextElement("playcount", QString("%1").arg(m_concert.playcount()));
    xml.writeTextElement("lastplayed", m_concert.lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    xml.writeTextElement("trailer", helper::formatTrailerUrl(m_concert.trailer().toString()));
    KodiXml::writeStringsAsOneTagEach(xml, "genre", m_concert.genres());
    KodiXml::writeStringsAsOneTagEach(xml, "tag", m_concert.tags());

    if (writeThumbUrlsToNfo()) {
        const auto& posters = m_concert.posters();
        for (const Poster& poster : posters) {
            if (poster.originalUrl.isValid()) {
                xml.writeStartElement("thumb");
                const QString aspect = poster.aspect.isEmpty() ? "poster" : poster.aspect;
                xml.writeAttribute("aspect", aspect);
                if (poster.thumbUrl.isValid()) {
                    xml.writeAttribute("preview", poster.thumbUrl.toString());
                }
                xml.writeCharacters(poster.originalUrl.toString());
                xml.writeEndElement();
            }
        }

        if (!m_concert.backdrops().isEmpty()) {
            xml.writeStartElement("fanart");
            const auto& backdrops = m_concert.backdrops();
            for (const Poster& poster : backdrops) {
                if (poster.originalUrl.isValid()) {
                    xml.writeStartElement("thumb");
                    if (poster.thumbUrl.isValid()) {
                        xml.writeAttribute("preview", poster.thumbUrl.toString());
                    }
                    xml.writeCharacters(poster.originalUrl.toString());
                    xml.writeEndElement();
                }
            }
            xml.writeEndElement();
        }
    }

    KodiXml::writeStreamDetails(xml, m_concert.streamDetails(), {}, m_concert.streamDetailsLoaded());

    if (!testMode) {
        addMediaelchGeneratorTag(xml);
    }

    xml.writeEndElement();
    xml.writeEndDocument();
    return xmlContent;
}

} // namespace kodi
} // namespace mediaelch
