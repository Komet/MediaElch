#include "media_centers/kodi/v17/ConcertXmlWriterV17.h"

#include "concerts/Concert.h"
#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "settings/Settings.h"

#include <QDomDocument>

namespace mediaelch {
namespace kodi {

ConcertXmlWriterV17::ConcertXmlWriterV17(Concert& concert) : m_concert{concert}
{
}

QByteArray ConcertXmlWriterV17::getConcertXml()
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

    // remove old v16 tags if they exist
    KodiXml::removeChildNodes(doc, "tmdbid");
    KodiXml::removeChildNodes(doc, "rating");

    KodiXml::setTextValue(doc, "title", m_concert.name());
    KodiXml::setTextValue(doc, "artist", m_concert.artist());
    KodiXml::setTextValue(doc, "album", m_concert.album());

    // id
    KodiXml::setTextValue(doc, "id", m_concert.imdbId().toString());
    // unique id: IMDb and TMDb
    KodiXml::removeChildNodes(doc, "uniqueid");
    {
        QDomElement uniqueId = doc.createElement("uniqueid");
        uniqueId.setAttribute("type", "imdb");
        uniqueId.setAttribute("default", "true");
        uniqueId.appendChild(doc.createTextNode(m_concert.imdbId().toString()));
        KodiXml::appendXmlNode(doc, uniqueId);
    }
    if (m_concert.tmdbId().isValid()) {
        QDomElement uniqueId = doc.createElement("uniqueid");
        uniqueId.setAttribute("type", "tmdb");
        uniqueId.appendChild(doc.createTextNode(m_concert.tmdbId().toString()));
        KodiXml::appendXmlNode(doc, uniqueId);
    }

    // rating
    KodiXml::removeChildNodes(doc, "ratings");
    QDomElement ratings = doc.createElement("ratings");
    bool firstRating = true;
    for (const Rating& rating : m_concert.ratings()) {
        QDomElement ratingValueElement = doc.createElement("value");
        ratingValueElement.appendChild(doc.createTextNode(QString::number(rating.rating)));
        QDomElement votesElement = doc.createElement("votes");
        votesElement.appendChild(doc.createTextNode(QString::number(rating.voteCount)));
        QDomElement ratingElement = doc.createElement("rating");
        ratingElement.setAttribute("name", rating.source);
        ratingElement.setAttribute("default", firstRating ? "true" : "false");
        if (rating.maxRating > 0) {
            ratingElement.setAttribute("max", rating.maxRating);
        }
        ratingElement.appendChild(ratingValueElement);
        ratingElement.appendChild(votesElement);
        ratings.appendChild(ratingElement);
        firstRating = false;
    }
    KodiXml::appendXmlNode(doc, ratings);
    KodiXml::setTextValue(doc, "userrating", QString::number(m_concert.userRating()));

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
    KodiXml::setListValue(doc, "genre", m_concert.genres());
    KodiXml::setListValue(doc, "tag", m_concert.tags());

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        KodiXml::removeChildNodes(doc, "thumb");
        KodiXml::removeChildNodes(doc, "fanart");

        for (const Poster& poster : m_concert.posters()) {
            QDomElement elem = doc.createElement("thumb");
            QString aspect = poster.aspect.isEmpty() ? "poster" : poster.aspect;

            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.setAttribute("aspect", aspect);
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
