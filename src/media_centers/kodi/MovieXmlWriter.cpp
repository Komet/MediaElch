#include "MovieXmlWriter.h"

#include "data/Movie.h"
#include "globals/Helper.h"
#include "media_centers/XbmcXml.h"
#include "settings/Settings.h"

#include <QDomDocument>

namespace Kodi {

MovieXmlWriter::MovieXmlWriter(Movie& movie) : m_movie{movie}
{
}

QByteArray MovieXmlWriter::getMovieXml()
{
    using namespace std::chrono_literals;

    QDomDocument doc;
    doc.setContent(m_movie.nfoContent());
    if (m_movie.nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8" standalone="yes")");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("movie"));
    }

    QDomElement movieElem = doc.elementsByTagName("movie").at(0).toElement();

    XbmcXml::setTextValue(doc, "title", m_movie.name());
    XbmcXml::setTextValue(doc, "originaltitle", m_movie.originalName());
    XbmcXml::setTextValue(doc, "rating", QString("%1").arg(m_movie.rating()));
    XbmcXml::setTextValue(doc, "votes", QString::number(m_movie.votes()));
    XbmcXml::setTextValue(doc, "top250", QString::number(m_movie.top250()));
    XbmcXml::setTextValue(doc, "year", m_movie.released().toString("yyyy"));
    XbmcXml::setTextValue(doc, "plot", m_movie.overview());
    XbmcXml::setTextValue(doc, "outline", m_movie.outline());
    XbmcXml::setTextValue(doc, "tagline", m_movie.tagline());
    if (m_movie.runtime() > 0min) {
        XbmcXml::setTextValue(doc, "runtime", QString::number(m_movie.runtime().count()));
    } else {
        XbmcXml::removeChildNodes(doc, "runtime");
    }
    XbmcXml::setTextValue(doc, "mpaa", m_movie.certification().toString());
    XbmcXml::setTextValue(doc, "playcount", QString("%1").arg(m_movie.playcount()));
    if (!m_movie.lastPlayed().isNull()) {
        XbmcXml::setTextValue(doc, "lastplayed", m_movie.lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    } else {
        XbmcXml::removeChildNodes(doc, "lastplayed");
    }
    if (!m_movie.dateAdded().isNull()) {
        XbmcXml::setTextValue(doc, "dateadded", m_movie.dateAdded().toString("yyyy-MM-dd HH:mm:ss"));
    } else {
        XbmcXml::removeChildNodes(doc, "dateadded");
    }
    XbmcXml::setTextValue(doc, "id", m_movie.imdbId().toString());
    XbmcXml::setTextValue(doc, "tmdbid", m_movie.tmdbId().toString());

    // <set>
    //   <name>...</name>
    //   <overview></overview>
    // </set>
    XbmcXml::removeChildNodes(doc, "set");
    if (!m_movie.set().isEmpty()) {
        QDomElement setElement = doc.createElement("set");
        QDomElement setNameElement = doc.createElement("name");
        setNameElement.appendChild(doc.createTextNode(m_movie.set()));
        QDomElement setOverviewElement = doc.createElement("overview");
        setElement.appendChild(setNameElement);
        setElement.appendChild(setOverviewElement);
        XbmcXml::appendXmlNode(doc, setElement);
    }
    XbmcXml::setTextValue(doc, "sorttitle", m_movie.sortTitle());
    XbmcXml::setTextValue(doc, "trailer", Helper::instance()->formatTrailerUrl(m_movie.trailer().toString()));
    XbmcXml::setTextValue(doc, "watched", (m_movie.watched()) ? "true" : "false");

    QStringList writers;
    for (const QString& credit : m_movie.writer().split(",")) {
        writers << credit.trimmed();
    }
    XbmcXml::setListValue(doc, "credits", writers);

    QStringList directors;
    for (const QString& director : m_movie.director().split(",")) {
        directors << director.trimmed();
    }
    XbmcXml::setListValue(doc, "director", directors);

    XbmcXml::setListValue(doc,
        "studio",
        Settings::instance()->advanced()->useFirstStudioOnly() && !m_movie.studios().isEmpty()
            ? m_movie.studios().mid(0, 1)
            : m_movie.studios());
    XbmcXml::setListValue(doc, "genre", m_movie.genres());
    XbmcXml::setListValue(doc, "country", m_movie.countries());
    XbmcXml::setListValue(doc, "tag", m_movie.tags());

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        XbmcXml::removeChildNodes(doc, "thumb");
        XbmcXml::removeChildNodes(doc, "fanart");

        for (const Poster& poster : m_movie.images().posters()) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            XbmcXml::appendXmlNode(doc, elem);
        }

        if (!m_movie.images().backdrops().isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            for (const Poster& poster : m_movie.images().backdrops()) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());
                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            XbmcXml::appendXmlNode(doc, fanartElem);
        }
    }

    XbmcXml::removeChildNodes(doc, "actor");

    for (const Actor& actor : m_movie.actors()) {
        QDomElement elem = doc.createElement("actor");
        QDomElement elemName = doc.createElement("name");
        QDomElement elemRole = doc.createElement("role");
        elemName.appendChild(doc.createTextNode(actor.name));
        elemRole.appendChild(doc.createTextNode(actor.role));
        elem.appendChild(elemName);
        elem.appendChild(elemRole);
        if (!actor.thumb.isEmpty() && Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
            QDomElement elemThumb = doc.createElement("thumb");
            elemThumb.appendChild(doc.createTextNode(actor.thumb));
            elem.appendChild(elemThumb);
        }
        XbmcXml::appendXmlNode(doc, elem);
    }

    XbmcXml::writeStreamDetails(doc, m_movie.streamDetails(), m_movie.subtitles());

    return doc.toByteArray(4);
}

} // namespace Kodi
