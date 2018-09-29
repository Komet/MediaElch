#include "MovieXmlReader.h"

#include "data/Movie.h"

#include <QDate>
#include <QDomDocument>
#include <QDomNodeList>
#include <QStringList>
#include <QUrl>

namespace Kodi {


MovieXmlReader::MovieXmlReader(Movie &movie) : m_movie{movie}
{
}

void MovieXmlReader::parseNfoDom(QDomDocument domDoc)
{
    if (!domDoc.elementsByTagName("title").isEmpty()) {
        m_movie.setName(domDoc.elementsByTagName("title").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("originaltitle").isEmpty()) {
        m_movie.setOriginalName(domDoc.elementsByTagName("originaltitle").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("rating").isEmpty()) {
        m_movie.setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().replace(",", ".").toDouble());
    }
    if (!domDoc.elementsByTagName("votes").isEmpty()) {
        m_movie.setVotes(
            domDoc.elementsByTagName("votes").at(0).toElement().text().replace(",", "").replace(".", "").toInt());
    }
    if (!domDoc.elementsByTagName("top250").isEmpty()) {
        m_movie.setTop250(domDoc.elementsByTagName("top250").at(0).toElement().text().toInt());
    }
    if (!domDoc.elementsByTagName("year").isEmpty()) {
        m_movie.setReleased(QDate::fromString(domDoc.elementsByTagName("year").at(0).toElement().text(), "yyyy"));
    }
    if (!domDoc.elementsByTagName("plot").isEmpty()) {
        m_movie.setOverview(domDoc.elementsByTagName("plot").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("outline").isEmpty()) {
        m_movie.setOutline(domDoc.elementsByTagName("outline").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("tagline").isEmpty()) {
        m_movie.setTagline(domDoc.elementsByTagName("tagline").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("runtime").isEmpty()) {
        m_movie.setRuntime(std::chrono::minutes(domDoc.elementsByTagName("runtime").at(0).toElement().text().toInt()));
    }
    if (!domDoc.elementsByTagName("mpaa").isEmpty()) {
        m_movie.setCertification(Certification(domDoc.elementsByTagName("mpaa").at(0).toElement().text()));
    }
    if (!domDoc.elementsByTagName("playcount").isEmpty()) {
        m_movie.setPlayCount(domDoc.elementsByTagName("playcount").at(0).toElement().text().toInt());
    }
    if (!domDoc.elementsByTagName("lastplayed").isEmpty()) {
        QDateTime lastPlayed = QDateTime::fromString(
            domDoc.elementsByTagName("lastplayed").at(0).toElement().text(), "yyyy-MM-dd HH:mm:ss");
        if (!lastPlayed.isValid()) {
            lastPlayed =
                QDateTime::fromString(domDoc.elementsByTagName("lastplayed").at(0).toElement().text(), "yyyy-MM-dd");
        }
        m_movie.setLastPlayed(lastPlayed);
    }
    if (!domDoc.elementsByTagName("dateadded").isEmpty()) {
        m_movie.setDateAdded(QDateTime::fromString(
            domDoc.elementsByTagName("dateadded").at(0).toElement().text(), "yyyy-MM-dd HH:mm:ss"));
    }
    if (!domDoc.elementsByTagName("id").isEmpty()) {
        m_movie.setId(ImdbId(domDoc.elementsByTagName("id").at(0).toElement().text()));
    }
    if (!domDoc.elementsByTagName("tmdbid").isEmpty()) {
        m_movie.setTmdbId(TmdbId(domDoc.elementsByTagName("tmdbid").at(0).toElement().text()));
    }
    const auto movieSetElements = domDoc.elementsByTagName("set");
    if (!movieSetElements.isEmpty()) {
        const QDomElement movieSetElement = movieSetElements.at(0).toElement();
        const QDomNodeList setNameElements = movieSetElement.elementsByTagName("name");

        // We need to support both the old and new XML syntax.
        //
        // New Kodi XML Syntax:
        // <set>
        //   <name>Movie Set Name</name>
        //   <overview></overview>
        // </set>
        //
        // Old Syntax:
        // <set>Movie Set Name</set>
        //
        if (!setNameElements.isEmpty()) {
            m_movie.setSet(setNameElements.at(0).toElement().text());
        } else {
            m_movie.setSet(movieSetElement.text());
        }
    }
    if (!domDoc.elementsByTagName("sorttitle").isEmpty()) {
        m_movie.setSortTitle(domDoc.elementsByTagName("sorttitle").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("trailer").isEmpty()) {
        m_movie.setTrailer(QUrl(domDoc.elementsByTagName("trailer").at(0).toElement().text()));
    }
    if (!domDoc.elementsByTagName("watched").isEmpty()) {
        m_movie.setWatched(domDoc.elementsByTagName("watched").at(0).toElement().text() == "true" ? true : false);
    } else {
        m_movie.setWatched(m_movie.playcount() > 0);
    }

    QStringList writers;
    for (int i = 0, n = domDoc.elementsByTagName("credits").size(); i < n; i++) {
        for (const QString &writer :
            domDoc.elementsByTagName("credits").at(i).toElement().text().split(",", QString::SkipEmptyParts)) {
            writers.append(writer.trimmed());
        }
    }
    m_movie.setWriter(writers.join(", "));

    QStringList directors;
    for (int i = 0, n = domDoc.elementsByTagName("director").size(); i < n; i++) {
        for (const QString &director :
            domDoc.elementsByTagName("director").at(i).toElement().text().split(",", QString::SkipEmptyParts)) {
            directors.append(director.trimmed());
        }
    }
    m_movie.setDirector(directors.join(", "));

    /**
     * For each element "tag", split the text by "/" and call the callbackFct with each item.
     */
    const auto forEachElement = [&domDoc](QString tag, auto callbackFct) {
        const QDomNodeList tags = domDoc.elementsByTagName(tag);
        const int tagCount = tags.size();
        for (int i = 0; i < tagCount; ++i) {
            for (const QString &item : tags.at(i).toElement().text().split("/", QString::SkipEmptyParts)) {
                callbackFct(item.trimmed());
            }
        }
    };

    forEachElement("studio", [&](QString studio) { m_movie.addStudio(studio); });
    forEachElement("genre", [&](QString genre) { m_movie.addGenre(genre); });
    forEachElement("country", [&](QString country) { m_movie.addCountry(country); });

    for (int i = 0, n = domDoc.elementsByTagName("tag").size(); i < n; i++) {
        m_movie.addTag(domDoc.elementsByTagName("tag").at(i).toElement().text());
    }
    for (int i = 0, n = domDoc.elementsByTagName("actor").size(); i < n; i++) {
        Actor a;
        a.imageHasChanged = false;
        const auto actorElement = domDoc.elementsByTagName("actor").at(i).toElement();
        if (!actorElement.elementsByTagName("name").isEmpty()) {
            a.name = actorElement.elementsByTagName("name").at(0).toElement().text();
        }
        if (!actorElement.elementsByTagName("role").isEmpty()) {
            a.role = actorElement.elementsByTagName("role").at(0).toElement().text();
        }
        if (!actorElement.elementsByTagName("thumb").isEmpty()) {
            a.thumb = actorElement.elementsByTagName("thumb").at(0).toElement().text();
        }
        m_movie.addActor(a);
    }
    for (int i = 0, n = domDoc.elementsByTagName("thumb").size(); i < n; i++) {
        QString parentTag = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().tagName();
        if (parentTag == "movie") {
            Poster p;
            p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
            p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            m_movie.images().addPoster(p);
        } else if (parentTag == "fanart") {
            Poster p;
            p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
            p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            m_movie.images().addBackdrop(p);
        }
    }
}

} // namespace Kodi
