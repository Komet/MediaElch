#include "MovieXmlReader.h"

#include "movies/Movie.h"

#include <QDate>
#include <QDomDocument>
#include <QDomNodeList>
#include <QStringList>
#include <QUrl>

namespace mediaelch {
namespace kodi {

MovieXmlReader::MovieXmlReader(Movie& movie) : m_movie{movie}
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
    // check for new ratings syntax
    if (!domDoc.elementsByTagName("ratings").isEmpty()) {
        // <ratings>
        //   <rating name="default" default="true">
        //     <value>10</value>
        //     <votes>10</votes>
        //   </rating>
        // </ratings>
        auto ratings = domDoc.elementsByTagName("ratings").at(0).toElement().elementsByTagName("rating");
        for (int i = 0; i < ratings.length(); ++i) {
            Rating rating;
            auto ratingElement = ratings.at(i).toElement();
            rating.source = ratingElement.attribute("name", "default");
            rating.rating =
                ratingElement.elementsByTagName("value").at(0).toElement().text().replace(",", ".").toDouble();
            rating.voteCount = ratingElement.elementsByTagName("votes")
                                   .at(0)
                                   .toElement()
                                   .text()
                                   .replace(",", "")
                                   .replace(".", "")
                                   .toInt();
            m_movie.ratings().push_back(rating);
            m_movie.setChanged(true);
        }

    } else if (!domDoc.elementsByTagName("rating").isEmpty()) {
        // otherwise use "old" syntax:
        // <rating>10.0</rating>
        // <votes>10.0</votes>
        Rating rating;
        rating.rating = domDoc.elementsByTagName("rating").at(0).toElement().text().replace(",", ".").toDouble();
        if (!domDoc.elementsByTagName("votes").isEmpty()) {
            rating.voteCount =
                domDoc.elementsByTagName("votes").at(0).toElement().text().replace(",", "").replace(".", "").toInt();
        }
        m_movie.ratings().clear();
        m_movie.ratings().push_back(rating);
        m_movie.setChanged(true);
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
        const QDomNodeList setOverviewElements = movieSetElement.elementsByTagName("overview");

        // We need to support both the old and new XML syntax.
        //
        // New Kodi v17 XML Syntax:
        // <set>
        //   <name>Movie Set Name</name>
        //   <overview></overview>
        // </set>
        //
        // Old Syntax:
        // <set>Movie Set Name</set>
        //
        MovieSet set;
        if (!setNameElements.isEmpty()) {
            set.name = setNameElements.at(0).toElement().text();
        } else {
            set.name = movieSetElement.text();
        }
        if (!setOverviewElements.isEmpty()) {
            set.overview = setOverviewElements.at(0).toElement().text();
        }
        m_movie.setSet(set);
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
        for (const QString& writer :
            domDoc.elementsByTagName("credits").at(i).toElement().text().split(",", QString::SkipEmptyParts)) {
            writers.append(writer.trimmed());
        }
    }
    m_movie.setWriter(writers.join(", "));

    QStringList directors;
    for (int i = 0, n = domDoc.elementsByTagName("director").size(); i < n; i++) {
        for (const QString& director :
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
            for (const QString& item : tags.at(i).toElement().text().split("/", QString::SkipEmptyParts)) {
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

} // namespace kodi
} // namespace mediaelch
