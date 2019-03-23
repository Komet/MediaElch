#include "ConcertXmlReader.h"

#include "concerts/Concert.h"

#include <QDate>
#include <QDomDocument>
#include <QDomNodeList>
#include <QStringList>
#include <QUrl>

namespace Kodi {


ConcertXmlReader::ConcertXmlReader(Concert& concert) : m_concert{concert}
{
}

void ConcertXmlReader::parseNfoDom(QDomDocument domDoc)
{
    if (!domDoc.elementsByTagName("id").isEmpty()) {
        m_concert.setImdbId(ImdbId(domDoc.elementsByTagName("id").at(0).toElement().text()));
    }
    if (!domDoc.elementsByTagName("tmdbid").isEmpty()) {
        m_concert.setTmdbId(TmdbId(domDoc.elementsByTagName("tmdbid").at(0).toElement().text()));
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
    if (!domDoc.elementsByTagName("rating").isEmpty()) {
        m_concert.setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().replace(",", ".").toDouble());
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
    if (!domDoc.elementsByTagName("watched").isEmpty()) {
        m_concert.setWatched(domDoc.elementsByTagName("watched").at(0).toElement().text() == "true" ? true : false);
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
    for (int i = 0, n = domDoc.elementsByTagName("thumb").size(); i < n; i++) {
        QString parentTag = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().tagName();
        if (parentTag == "musicvideo") {
            Poster p;
            p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
            p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            m_concert.addPoster(p);
        } else if (parentTag == "fanart") {
            Poster p;
            p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
            p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            m_concert.addBackdrop(p);
        }
    }
}

} // namespace Kodi
