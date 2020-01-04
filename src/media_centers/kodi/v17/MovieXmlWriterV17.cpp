#include "media_centers/kodi/v17/MovieXmlWriterV17.h"

#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "media_centers/kodi/KodiNfoMeta.h"
#include "movies/Movie.h"
#include "settings/Settings.h"

#include <QDomComment>
#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace mediaelch {
namespace kodi {

MovieXmlWriterV17::MovieXmlWriterV17(Movie& movie) : m_movie{movie}
{
}

QByteArray MovieXmlWriterV17::getMovieXml()
{
    using namespace std::chrono_literals;

    QDomDocument doc;
    doc.setContent(m_movie.nfoContent());
    if (m_movie.nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8" standalone="yes" )");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("movie"));
        QDomComment meta;
        meta.setData(getKodiNfoComment());
        doc.appendChild(meta);
    }

    QDomElement movieElem = doc.elementsByTagName("movie").at(0).toElement();

    // remove old v16 tags if they exist
    KodiXml::removeChildNodes(doc, "tmdbid");
    KodiXml::removeChildNodes(doc, "rating");
    KodiXml::removeChildNodes(doc, "votes");

    KodiXml::setTextValue(doc, "title", m_movie.name());
    if (!m_movie.originalName().isEmpty() && m_movie.originalName() != m_movie.name()) {
        KodiXml::setTextValue(doc, "originaltitle", m_movie.originalName());
    }
    if (!m_movie.sortTitle().isEmpty()) {
        KodiXml::setTextValue(doc, "sorttitle", m_movie.sortTitle());
    }

    // rating
    KodiXml::removeChildNodes(doc, "ratings");
    QDomElement ratings = doc.createElement("ratings");
    bool firstRating = true;
    for (const Rating& rating : m_movie.ratings()) {
        QDomElement ratingValueElement = doc.createElement("value");
        ratingValueElement.appendChild(doc.createTextNode(QString::number(rating.rating)));
        QDomElement votesElement = doc.createElement("votes");
        votesElement.appendChild(doc.createTextNode(QString::number(rating.voteCount)));
        QDomElement ratingElement = doc.createElement("rating");
        ratingElement.setAttribute("name", rating.source);
        if (rating.maxRating > 0) {
            ratingElement.setAttribute("max", rating.maxRating);
        }
        ratingElement.setAttribute("default", firstRating ? "true" : "false");
        ratingElement.appendChild(ratingValueElement);
        ratingElement.appendChild(votesElement);
        ratings.appendChild(ratingElement);
        firstRating = false;
    }
    KodiXml::appendXmlNode(doc, ratings);

    KodiXml::setTextValue(doc, "userrating", QString::number(m_movie.userRating()));
    KodiXml::setTextValue(doc, "top250", QString::number(m_movie.top250()));
    KodiXml::setTextValue(doc, "outline", m_movie.outline());
    KodiXml::setTextValue(doc, "plot", m_movie.overview());
    KodiXml::setTextValue(doc, "tagline", m_movie.tagline());
    if (m_movie.runtime() > 0min) {
        KodiXml::setTextValue(doc, "runtime", QString::number(m_movie.runtime().count()));
    } else {
        KodiXml::removeChildNodes(doc, "runtime");
    }

    KodiXml::removeChildNodes(doc, "thumb");
    KodiXml::removeChildNodes(doc, "fanart");

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        for (const Poster& poster : m_movie.images().posters()) {
            QDomElement elem = doc.createElement("thumb");
            QString aspect = poster.aspect.isEmpty() ? "poster" : poster.aspect;
            elem.setAttribute("aspect", aspect);
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            KodiXml::appendXmlNode(doc, elem);
        }

        if (!m_movie.images().backdrops().isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            for (const Poster& poster : m_movie.images().backdrops()) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());
                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            KodiXml::appendXmlNode(doc, fanartElem);
        }
    }

    KodiXml::setTextValue(doc, "mpaa", m_movie.certification().toString());
    KodiXml::setTextValue(doc, "playcount", QString("%1").arg(m_movie.playcount()));
    KodiXml::setTextValue(doc, "lastplayed", m_movie.lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    // id
    KodiXml::setTextValue(doc, "id", m_movie.imdbId().toString());
    // unique id: IMDb and TMDb
    KodiXml::removeChildNodes(doc, "uniqueid");
    {
        QDomElement uniqueId = doc.createElement("uniqueid");
        uniqueId.setAttribute("type", "imdb");
        uniqueId.setAttribute("default", "true");
        uniqueId.appendChild(doc.createTextNode(m_movie.imdbId().toString()));
        KodiXml::appendXmlNode(doc, uniqueId);
    }
    if (m_movie.tmdbId().isValid()) {
        QDomElement uniqueId = doc.createElement("uniqueid");
        uniqueId.setAttribute("type", "tmdb");
        uniqueId.appendChild(doc.createTextNode(m_movie.tmdbId().toString()));
        KodiXml::appendXmlNode(doc, uniqueId);
    }
    KodiXml::setListValue(doc, "genre", m_movie.genres());
    KodiXml::setListValue(doc, "country", m_movie.countries());

    // <set>
    //   <name>...</name>
    //   <overview>...</overview>
    // </set>
    KodiXml::removeChildNodes(doc, "set");
    if (!m_movie.set().name.isEmpty()) {
        MovieSet set = m_movie.set();
        QDomElement setElement = doc.createElement("set");
        QDomElement setNameElement = doc.createElement("name");
        setNameElement.appendChild(doc.createTextNode(set.name));
        QDomElement setOverviewElement = doc.createElement("overview");
        setOverviewElement.appendChild(doc.createTextNode(set.overview));
        setElement.appendChild(setNameElement);
        setElement.appendChild(setOverviewElement);
        KodiXml::appendXmlNode(doc, setElement);
    }

    QStringList writers;
    for (const QString& credit : m_movie.writer().split(",")) {
        writers << credit.trimmed();
    }
    KodiXml::setListValue(doc, "credits", writers);
    QStringList directors;
    for (const QString& director : m_movie.director().split(",")) {
        directors << director.trimmed();
    }
    KodiXml::setListValue(doc, "director", directors);
    KodiXml::setTextValue(doc, "premiered", m_movie.released().toString("yyyy-MM-dd"));
    KodiXml::setTextValue(doc, "year", m_movie.released().toString("yyyy"));
    KodiXml::setListValue(doc,
        "studio",
        Settings::instance()->advanced()->useFirstStudioOnly() && !m_movie.studios().isEmpty()
            ? m_movie.studios().mid(0, 1)
            : m_movie.studios());
    KodiXml::setTextValue(doc, "trailer", helper::formatTrailerUrl(m_movie.trailer().toString()));
    KodiXml::writeStreamDetails(doc, m_movie.streamDetails(), m_movie.subtitles());

    KodiXml::removeChildNodes(doc, "actor");
    for (const Actor* actor : m_movie.actors()) {
        QDomElement elem = doc.createElement("actor");
        QDomElement elemName = doc.createElement("name");
        QDomElement elemRole = doc.createElement("role");
        QDomElement elemOrder = doc.createElement("order");
        elemName.appendChild(doc.createTextNode(actor->name));
        elemRole.appendChild(doc.createTextNode(actor->role));
        elemOrder.appendChild(doc.createTextNode(QString::number(actor->order)));
        elem.appendChild(elemName);
        elem.appendChild(elemRole);
        elem.appendChild(elemOrder);
        if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
            // create a thumb tag even if its value is empty
            // Kodi does the same
            QDomElement elemThumb = doc.createElement("thumb");
            elemThumb.appendChild(doc.createTextNode(actor->thumb));
            elem.appendChild(elemThumb);
        }
        KodiXml::appendXmlNode(doc, elem);
    }

    // <resume>
    //   <position>0.000000</position>
    //   <total>0.000000</total>
    // </resume>
    KodiXml::removeChildNodes(doc, "resume");
    ResumeTime time = m_movie.resumeTime();
    QDomElement resumeElement = doc.createElement("resume");
    QDomElement resumePositionElement = doc.createElement("position");
    resumePositionElement.appendChild(doc.createTextNode(QString::number(time.position)));
    QDomElement resumeTotalElement = doc.createElement("total");
    resumeTotalElement.appendChild(doc.createTextNode(QString::number(time.total)));
    resumeElement.appendChild(resumePositionElement);
    resumeElement.appendChild(resumeTotalElement);
    KodiXml::appendXmlNode(doc, resumeElement);

    if (m_movie.dateAdded().isValid()) {
        KodiXml::setTextValue(doc, "dateadded", m_movie.dateAdded().toString("yyyy-MM-dd HH:mm:ss"));
    } else {
        KodiXml::removeChildNodes(doc, "dateadded");
    }
    KodiXml::setListValue(doc, "tag", m_movie.tags());

    return doc.toByteArray(4);
}

} // namespace kodi
} // namespace mediaelch
