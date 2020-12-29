#include "media_centers/kodi/MovieXmlReader.h"

#include "movies/Movie.h"

#include <QDate>
#include <QDomDocument>
#include <QDomNodeList>
#include <QStringList>
#include <QTextDocument>
#include <QUrl>

namespace mediaelch {
namespace kodi {

static QString htmlUnescape(const QString& htmlEscaped)
{
    QTextDocument doc;
    doc.setHtml(htmlEscaped);
    return doc.toPlainText();
}

MovieXmlReader::MovieXmlReader(Movie& movie) : m_movie{movie}
{
}

void MovieXmlReader::parseNfoDom(QDomDocument domDoc)
{
    if (domDoc.elementsByTagName("movie").isEmpty()) {
        qWarning() << "[MovieXmlReader] No <movie> tag in the document";
        return;
    }
    QDomElement movieElement = domDoc.elementsByTagName("movie").at(0).toElement();
    QMap<QString, void (MovieXmlReader::*)(const QDomElement&)> tagParsers;
    // clang-format off
    tagParsers.insert("title",         &MovieXmlReader::simpleString<&Movie::setName>);
    tagParsers.insert("originaltitle", &MovieXmlReader::simpleString<&Movie::setOriginalName>);
    tagParsers.insert("sorttitle",     &MovieXmlReader::simpleString<&Movie::setSortTitle>);
    tagParsers.insert("plot",          &MovieXmlReader::simpleString<&Movie::setOverview>);
    tagParsers.insert("outline",       &MovieXmlReader::simpleString<&Movie::setOutline>);
    tagParsers.insert("tagline",       &MovieXmlReader::simpleString<&Movie::setTagline>);
    tagParsers.insert("set",           &MovieXmlReader::movieSet);
    tagParsers.insert("actor",         &MovieXmlReader::movieActor);
    tagParsers.insert("thumb",         &MovieXmlReader::movieThumbnail);
    tagParsers.insert("fanart",        &MovieXmlReader::movieFanart);
    tagParsers.insert("playcount",     &MovieXmlReader::simpleInt<&Movie::setPlayCount>);
    tagParsers.insert("top250",        &MovieXmlReader::simpleInt<&Movie::setTop250>);
    tagParsers.insert("tag",           &MovieXmlReader::simpleString<&Movie::addTag>);
    tagParsers.insert("studio",        &MovieXmlReader::stringList<&Movie::addStudio, '/'>);
    tagParsers.insert("genre",         &MovieXmlReader::stringList<&Movie::addGenre, '/'>);
    tagParsers.insert("country",       &MovieXmlReader::stringList<&Movie::addCountry, '/'>);
    tagParsers.insert("ratings",       &MovieXmlReader::movieRatingV17);
    tagParsers.insert("rating",        &MovieXmlReader::movieRatingV16);
    tagParsers.insert("userrating",    &MovieXmlReader::simpleDouble<&Movie::setUserRating>);
    tagParsers.insert("votes",         &MovieXmlReader::movieVoteCountV16);
    tagParsers.insert("dateadded",     &MovieXmlReader::simpleDateTime<&Movie::setDateAdded>);
    tagParsers.insert("resume",        &MovieXmlReader::movieResumeTime);
    // clang-format on

    QDomNodeList nodes = movieElement.childNodes();
    for (int i = 0; i < nodes.size(); ++i) {
        if (nodes.at(i).isElement()) {
            QDomElement element = nodes.at(i).toElement();
            if (tagParsers.contains(element.tagName())) {
                // call the stored method pointer
                (this->*tagParsers[element.tagName()])(element);
            }
        }
    }

    if (!domDoc.elementsByTagName("year").isEmpty()) {
        m_movie.setReleased(QDate::fromString(domDoc.elementsByTagName("year").at(0).toElement().text(), "yyyy"));
    }
    // will overwrite the release date set by <year>
    if (!domDoc.elementsByTagName("premiered").isEmpty()) {
        QString value = domDoc.elementsByTagName("premiered").at(0).toElement().text().trimmed();
        QDate released = QDate::fromString(value, "yyyy-MM-dd");
        if (released.isValid()) {
            m_movie.setReleased(released);
        }
    }

    if (!domDoc.elementsByTagName("runtime").isEmpty()) {
        m_movie.setRuntime(std::chrono::minutes(domDoc.elementsByTagName("runtime").at(0).toElement().text().toInt()));
    }
    if (!domDoc.elementsByTagName("mpaa").isEmpty()) {
        m_movie.setCertification(Certification(domDoc.elementsByTagName("mpaa").at(0).toElement().text()));
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

    // v16 imdbid
    if (!domDoc.elementsByTagName("id").isEmpty()) {
        m_movie.setImdbId(ImdbId(domDoc.elementsByTagName("id").at(0).toElement().text()));
    }
    // v16 tmdbid
    if (!domDoc.elementsByTagName("tmdbid").isEmpty()) {
        m_movie.setTmdbId(TmdbId(domDoc.elementsByTagName("tmdbid").at(0).toElement().text()));
    }
    // >v17 ids
    auto uniqueIds = domDoc.elementsByTagName("uniqueid");
    for (int i = 0; i < uniqueIds.size(); ++i) {
        QDomElement element = uniqueIds.at(i).toElement();
        QString type = element.attribute("type");
        QString value = element.text().trimmed();
        if (type == "imdb") {
            m_movie.setImdbId(ImdbId(value));
        } else if (type == "tmdb") {
            m_movie.setTmdbId(TmdbId(value));
        }
    }

    if (!domDoc.elementsByTagName("trailer").isEmpty()) {
        m_movie.setTrailer(QUrl(domDoc.elementsByTagName("trailer").at(0).toElement().text()));
    }

    QStringList writers;
    for (int i = 0, n = domDoc.elementsByTagName("credits").size(); i < n; i++) {
        const auto credits =
            domDoc.elementsByTagName("credits").at(i).toElement().text().split(",", ElchSplitBehavior::SkipEmptyParts);
        for (const QString& writer : credits) {
            writers.append(writer.trimmed());
        }
    }
    m_movie.setWriter(writers.join(", "));

    QStringList directors;
    for (int i = 0, n = domDoc.elementsByTagName("director").size(); i < n; i++) {
        const auto directorsFound =
            domDoc.elementsByTagName("director").at(i).toElement().text().split(",", ElchSplitBehavior::SkipEmptyParts);
        for (const QString& director : directorsFound) {
            directors.append(director.trimmed());
        }
    }
    m_movie.setDirector(directors.join(", "));
}

void MovieXmlReader::movieSet(const QDomElement& movieSetElement)
{
    const QDomNodeList setNameElements = movieSetElement.elementsByTagName("name");
    const QDomNodeList setOverviewElements = movieSetElement.elementsByTagName("overview");

    // We need to support both the old and new XML syntax.
    //
    // New Kodi v17 XML Syntax:
    //   <set>
    //     <name>Movie Set Name</name>
    //     <overview>movie collection overview</overview>
    //   </set>
    //
    // Old Syntax:
    //   <set>Movie Set Name</set>
    //
    MovieSet set;
    if (!setNameElements.isEmpty()) {
        set.name = setNameElements.at(0).toElement().text();
    } else {
        set.name = movieSetElement.text();
    }
    if (!setOverviewElements.isEmpty()) {
        set.overview = htmlUnescape(setOverviewElements.at(0).toElement().text());
    }
    m_movie.setSet(set);
}

void MovieXmlReader::movieActor(const QDomElement& actorElement)
{
    Actor a;
    a.imageHasChanged = false;
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

void MovieXmlReader::movieThumbnail(const QDomElement& thumbElement)
{
    QString aspect = thumbElement.attribute("aspect").trimmed();
    // if (aspect == "set.poster") {
    //     // TODO: special handling of set-posters, etc.
    // }

    Poster p;
    p.originalUrl = QUrl(thumbElement.text());
    p.thumbUrl = QUrl(thumbElement.attribute("preview"));
    p.aspect = aspect;
    m_movie.images().addPoster(p);
}

void MovieXmlReader::movieFanart(const QDomElement& fanartElement)
{
    QDomNodeList thumbs = fanartElement.elementsByTagName("thumb");
    for (int i = 0; i < thumbs.size(); ++i) {
        QDomElement thumbElement = thumbs.at(i).toElement();
        Poster p;
        p.originalUrl = QUrl(thumbElement.text());
        p.thumbUrl = QUrl(thumbElement.attribute("preview"));
        m_movie.images().addBackdrop(p);
    }
}

void MovieXmlReader::movieRatingV17(const QDomElement& element)
{
    // <ratings>
    //   <rating name="default" default="true">
    //     <value>10</value>
    //     <votes>10</votes>
    //   </rating>
    // </ratings>
    auto ratings = element.elementsByTagName("rating");

    // clear all ratings in case that there are <rating> tags to avoid
    // duplicated and/or old ratings
    if (ratings.length() > 0) {
        m_movie.ratings().clear();
    }

    for (int i = 0; i < ratings.length(); ++i) {
        Rating rating;
        auto ratingElement = ratings.at(i).toElement();
        rating.source = ratingElement.attribute("name", "default");
        bool ok = false;
        int max = ratingElement.attribute("max", "0").toInt(&ok);
        if (ok && max > 0) {
            rating.maxRating = max;
        }
        rating.rating = ratingElement.elementsByTagName("value").at(0).toElement().text().replace(",", ".").toDouble();
        rating.voteCount =
            ratingElement.elementsByTagName("votes").at(0).toElement().text().replace(",", "").replace(".", "").toInt();
        m_movie.ratings().push_back(rating);
        m_movie.setChanged(true);
    }
}

void MovieXmlReader::movieRatingV16(const QDomElement& element)
{
    // <rating>10.0</rating>
    QString value = element.text();
    if (!value.isEmpty()) {
        if (m_movie.ratings().isEmpty()) {
            m_movie.ratings().push_back(Rating{});
        }
        m_movie.ratings().first().rating = value.replace(",", ".").toDouble();
        m_movie.setChanged(true);
    }
}

void MovieXmlReader::movieVoteCountV16(const QDomElement& element)
{
    // <votes>100</votes>
    QString value = element.text();
    if (!value.isEmpty()) {
        if (m_movie.ratings().isEmpty()) {
            m_movie.ratings().push_back(Rating{});
        }
        m_movie.ratings().first().voteCount = value.replace(",", ".").replace(".", "").toInt();
        m_movie.setChanged(true);
    }
}

void MovieXmlReader::movieResumeTime(const QDomElement& element)
{
    auto positions = element.elementsByTagName("position");
    auto totals = element.elementsByTagName("total");

    mediaelch::ResumeTime time;

    if (!positions.isEmpty()) {
        bool ok = false;
        const double position = positions.at(0).toElement().text().replace(",", ".").toDouble(&ok);
        if (ok) {
            time.position = position;
        }
    }

    if (!totals.isEmpty()) {
        bool ok = false;
        const double total = totals.at(0).toElement().text().replace(",", ".").toDouble(&ok);
        if (ok) {
            time.total = total;
        }
    }

    m_movie.setResumeTime(time);
}

} // namespace kodi
} // namespace mediaelch
