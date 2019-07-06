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
        QDomNode node = doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8" standalone="yes")");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("movie"));
        QDomComment meta;
        meta.setData(getKodiNfoComment());
        doc.appendChild(meta);
    }

    QDomElement movieElem = doc.elementsByTagName("movie").at(0).toElement();

    KodiXml::setTextValue(doc, "title", m_movie.name());
    KodiXml::setTextValue(doc, "originaltitle", m_movie.originalName());

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
        ratingElement.setAttribute("default", firstRating ? "true" : "false");
        ratingElement.appendChild(ratingValueElement);
        ratingElement.appendChild(votesElement);
        ratings.appendChild(ratingElement);
        firstRating = false;
    }
    KodiXml::appendXmlNode(doc, ratings);

    KodiXml::setTextValue(doc, "top250", QString::number(m_movie.top250()));
    KodiXml::setTextValue(doc, "year", m_movie.released().toString("yyyy"));
    KodiXml::setTextValue(doc, "plot", m_movie.overview());
    KodiXml::setTextValue(doc, "outline", m_movie.outline());
    KodiXml::setTextValue(doc, "tagline", m_movie.tagline());
    if (m_movie.runtime() > 0min) {
        KodiXml::setTextValue(doc, "runtime", QString::number(m_movie.runtime().count()));
    } else {
        KodiXml::removeChildNodes(doc, "runtime");
    }
    KodiXml::setTextValue(doc, "mpaa", m_movie.certification().toString());
    KodiXml::setTextValue(doc, "playcount", QString("%1").arg(m_movie.playcount()));
    if (m_movie.lastPlayed().isValid()) {
        KodiXml::setTextValue(doc, "lastplayed", m_movie.lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    } else {
        KodiXml::removeChildNodes(doc, "lastplayed");
    }
    if (!m_movie.dateAdded().isNull()) {
        KodiXml::setTextValue(doc, "dateadded", m_movie.dateAdded().toString("yyyy-MM-dd HH:mm:ss"));
    } else {
        KodiXml::removeChildNodes(doc, "dateadded");
    }

    // id
    KodiXml::setTextValue(doc, "id", m_movie.imdbId().toString());
    // unique id: IMDb and TMDb
    KodiXml::removeChildNodes(doc, "uniqueid");
    {
        QDomElement uniqueId = doc.createElement("uniqueid");
        uniqueId.setAttribute("default", "true");
        uniqueId.setAttribute("type", "imdb");
        uniqueId.appendChild(doc.createTextNode(m_movie.imdbId().toString()));
        KodiXml::appendXmlNode(doc, uniqueId);
    }
    if (m_movie.tmdbId().isValid()) {
        QDomElement uniqueId = doc.createElement("uniqueid");
        uniqueId.setAttribute("type", "tmdb");
        uniqueId.appendChild(doc.createTextNode(m_movie.tmdbId().toString()));
        KodiXml::appendXmlNode(doc, uniqueId);
    }

    // <set>
    //   <name>...</name>
    //   <overview></overview>
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
    KodiXml::setTextValue(doc, "sorttitle", m_movie.sortTitle());
    KodiXml::setTextValue(doc, "trailer", Helper::formatTrailerUrl(m_movie.trailer().toString()));
    // TODO: is this required in v17?
    KodiXml::setTextValue(doc, "watched", (m_movie.watched()) ? "true" : "false");

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

    KodiXml::setListValue(doc,
        "studio",
        Settings::instance()->advanced()->useFirstStudioOnly() && !m_movie.studios().isEmpty()
            ? m_movie.studios().mid(0, 1)
            : m_movie.studios());
    KodiXml::setListValue(doc, "genre", m_movie.genres());
    KodiXml::setListValue(doc, "country", m_movie.countries());
    KodiXml::setListValue(doc, "tag", m_movie.tags());

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        KodiXml::removeChildNodes(doc, "thumb");
        KodiXml::removeChildNodes(doc, "fanart");

        for (const Poster& poster : m_movie.images().posters()) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("aspect", "poster");
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

    KodiXml::removeChildNodes(doc, "actor");

    int order = 0; // todo: save the order in the actor struct
    for (const Actor& actor : m_movie.actors()) {
        QDomElement elem = doc.createElement("actor");
        QDomElement elemName = doc.createElement("name");
        QDomElement elemRole = doc.createElement("role");
        QDomElement elemOrder = doc.createElement("order");
        elemName.appendChild(doc.createTextNode(actor.name));
        elemRole.appendChild(doc.createTextNode(actor.role));
        elemOrder.appendChild(doc.createTextNode(QString::number(order)));
        elem.appendChild(elemName);
        elem.appendChild(elemRole);
        elem.appendChild(elemOrder);
        if (!actor.thumb.isEmpty() && Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
            QDomElement elemThumb = doc.createElement("thumb");
            elemThumb.appendChild(doc.createTextNode(actor.thumb));
            elem.appendChild(elemThumb);
        }
        KodiXml::appendXmlNode(doc, elem);
        ++order;
    }

    KodiXml::writeStreamDetails(doc, m_movie.streamDetails(), m_movie.subtitles());

    return doc.toByteArray(4);
}

} // namespace kodi
} // namespace mediaelch
