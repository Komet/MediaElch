#include "ConcertXmlReader.h"

#include "concerts/Concert.h"

#include <QDate>
#include <QDomDocument>
#include <QDomNodeList>
#include <QStringList>
#include <QUrl>

namespace mediaelch {
namespace kodi {

ConcertXmlReader::ConcertXmlReader(Concert& concert) : m_concert{concert}
{
}

void ConcertXmlReader::parseNfoDom(QDomDocument domDoc)
{
    // v16 imdbid
    if (!domDoc.elementsByTagName("id").isEmpty()) {
        m_concert.setImdbId(ImdbId(domDoc.elementsByTagName("id").at(0).toElement().text()));
    }
    // v16 tmdbid
    if (!domDoc.elementsByTagName("tmdbid").isEmpty()) {
        m_concert.setTmdbId(TmdbId(domDoc.elementsByTagName("tmdbid").at(0).toElement().text()));
    }
    // v17 ids
    auto uniqueIds = domDoc.elementsByTagName("uniqueid");
    for (int i = 0; i < uniqueIds.size(); ++i) {
        QDomElement element = uniqueIds.at(i).toElement();
        QString type = element.attribute("type");
        QString value = element.text().trimmed();
        if (type == "imdb") {
            m_concert.setImdbId(ImdbId(value));
        } else if (type == "tmdb") {
            m_concert.setTmdbId(TmdbId(value));
        }
    }

    if (!domDoc.elementsByTagName("title").isEmpty()) {
        m_concert.setName(domDoc.elementsByTagName("title").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("artist").isEmpty()) {
        m_concert.setArtist(domDoc.elementsByTagName("artist").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("album").isEmpty()) {
        m_concert.setAlbum(domDoc.elementsByTagName("album").at(0).toElement().text());
    }

    // check for new ratings syntax
    if (!domDoc.elementsByTagName("ratings").isEmpty()) {
        // <ratings>
        //   <rating name="default" default="true">
        //     <value>10</value>
        //     <votes>10</votes>
        //   </rating>
        // </ratings>
        auto ratings = domDoc.elementsByTagName("ratings").at(0).toElement().elementsByTagName("rating");
        m_concert.ratings().clear();

        for (int i = 0; i < ratings.length(); ++i) {
            Rating rating;
            auto ratingElement = ratings.at(i).toElement();
            rating.source = ratingElement.attribute("name", "default");
            bool ok = false;
            const int max = ratingElement.attribute("max", "0").toInt(&ok);
            if (ok && max > 0) {
                rating.maxRating = max;
            }
            rating.rating =
                ratingElement.elementsByTagName("value").at(0).toElement().text().replace(",", ".").toDouble();
            rating.voteCount = ratingElement.elementsByTagName("votes")
                                   .at(0)
                                   .toElement()
                                   .text()
                                   .replace(",", "")
                                   .replace(".", "")
                                   .toInt();
            m_concert.ratings().push_back(rating);
            m_concert.setChanged(true);
        }

    } else if (!domDoc.elementsByTagName("rating").isEmpty()) {
        // otherwise use "old" syntax:
        // <rating>10.0</rating>
        // <votes>10.0</votes>
        QString value = domDoc.elementsByTagName("rating").at(0).toElement().text();
        if (!value.isEmpty()) {
            Rating rating;
            rating.rating = value.replace(",", ".").toDouble();
            if (!domDoc.elementsByTagName("votes").isEmpty()) {
                rating.voteCount = domDoc.elementsByTagName("votes")
                                       .at(0)
                                       .toElement()
                                       .text()
                                       .replace(",", "")
                                       .replace(".", "")
                                       .toInt();
            }
            m_concert.ratings().clear();
            m_concert.ratings().push_back(rating);
            m_concert.setChanged(true);
        }
    }
    if (!domDoc.elementsByTagName("userrating").isEmpty()) {
        m_concert.setUserRating(domDoc.elementsByTagName("userrating").at(0).toElement().text().toDouble());
    }

    if (!domDoc.elementsByTagName("year").isEmpty()) {
        m_concert.setReleased(QDate::fromString(domDoc.elementsByTagName("year").at(0).toElement().text(), "yyyy"));
    }
    if (!domDoc.elementsByTagName("plot").isEmpty()) {
        m_concert.setOverview(domDoc.elementsByTagName("plot").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("tagline").isEmpty()) {
        m_concert.setTagline(domDoc.elementsByTagName("tagline").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("runtime").isEmpty()) {
        m_concert.setRuntime(
            std::chrono::minutes(domDoc.elementsByTagName("runtime").at(0).toElement().text().toInt()));
    }
    if (!domDoc.elementsByTagName("mpaa").isEmpty()) {
        m_concert.setCertification(Certification(domDoc.elementsByTagName("mpaa").at(0).toElement().text()));
    }
    if (!domDoc.elementsByTagName("playcount").isEmpty()) {
        m_concert.setPlayCount(domDoc.elementsByTagName("playcount").at(0).toElement().text().toInt());
    }
    if (!domDoc.elementsByTagName("lastplayed").isEmpty()) {
        m_concert.setLastPlayed(QDateTime::fromString(
            domDoc.elementsByTagName("lastplayed").at(0).toElement().text(), "yyyy-MM-dd HH:mm:ss"));
    }
    if (!domDoc.elementsByTagName("trailer").isEmpty()) {
        m_concert.setTrailer(QUrl(domDoc.elementsByTagName("trailer").at(0).toElement().text()));
    }

    for (int i = 0, n = domDoc.elementsByTagName("genre").size(); i < n; i++) {
        for (const QString& genre :
            domDoc.elementsByTagName("genre").at(i).toElement().text().split(" / ", QString::SkipEmptyParts)) {
            m_concert.addGenre(genre);
        }
    }
    for (int i = 0, n = domDoc.elementsByTagName("tag").size(); i < n; i++) {
        m_concert.addTag(domDoc.elementsByTagName("tag").at(i).toElement().text());
    }

    QDomNodeList thumbElements = domDoc.elementsByTagName("thumb");
    for (int i = 0, n = thumbElements.size(); i < n; i++) {
        QString parentTag = thumbElements.at(i).parentNode().toElement().tagName();
        QDomElement thumb = thumbElements.at(i).toElement();

        Poster p;
        p.originalUrl = thumb.text();
        p.thumbUrl = thumb.attribute("preview").trimmed().isEmpty() ? p.originalUrl : thumb.attribute("preview");
        p.aspect = thumb.attribute("aspect").trimmed();

        if (parentTag == "musicvideo") {
            m_concert.addPoster(p);

        } else if (parentTag == "fanart") {
            m_concert.addBackdrop(p);
        }
    }
}

} // namespace kodi
} // namespace mediaelch
