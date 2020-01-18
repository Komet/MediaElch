#include "media_centers/kodi/v16/MovieXmlWriterV16.h"

#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "media_centers/kodi/KodiNfoMeta.h"
#include "movies/Movie.h"
#include "settings/Settings.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace mediaelch {
namespace kodi {

MovieXmlWriterV16::MovieXmlWriterV16(Movie& movie) : m_movie{movie}
{
}

QByteArray MovieXmlWriterV16::getMovieXml()
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

    KodiXml::setTextValue(doc, "title", m_movie.name());
    KodiXml::setTextValue(doc, "originaltitle", m_movie.originalName());

    // rating
    if (!m_movie.ratings().empty()) {
        // v16 only supports one rating/vote node
        const auto& rating = m_movie.ratings().front();
        KodiXml::setTextValue(doc, "rating", QString::number(rating.rating));
        KodiXml::setTextValue(doc, "votes", QString::number(rating.voteCount));
    } else {
        KodiXml::setTextValue(doc, "rating", "");
        KodiXml::setTextValue(doc, "votes", "");
    }

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
    KodiXml::setTextValue(doc, "id", m_movie.imdbId().toString());
    if (m_movie.tmdbId().isValid()) {
        KodiXml::setTextValue(doc, "tmdbid", m_movie.tmdbId().toString());
    }
    if (!m_movie.set().name.isEmpty()) {
        // v16 syntax is different from v17
        // <set>Movie set</set>
        KodiXml::setTextValue(doc, "set", m_movie.set().name);
    }
    KodiXml::setTextValue(doc, "sorttitle", m_movie.sortTitle());
    KodiXml::setTextValue(doc, "trailer", helper::formatTrailerUrl(m_movie.trailer().toString()));
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

    KodiXml::removeChildNodes(doc, "thumb");
    KodiXml::removeChildNodes(doc, "fanart");

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
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

    for (const Actor* actor : m_movie.actors()) {
        QDomElement elem = doc.createElement("actor");
        QDomElement elemName = doc.createElement("name");
        QDomElement elemRole = doc.createElement("role");
        elemName.appendChild(doc.createTextNode(actor->name));
        elemRole.appendChild(doc.createTextNode(actor->role));
        elem.appendChild(elemName);
        elem.appendChild(elemRole);
        if (!actor->thumb.isEmpty() && Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
            QDomElement elemThumb = doc.createElement("thumb");
            elemThumb.appendChild(doc.createTextNode(actor->thumb));
            elem.appendChild(elemThumb);
        }
        KodiXml::appendXmlNode(doc, elem);
    }

    KodiXml::writeStreamDetails(doc, m_movie.streamDetails(), m_movie.subtitles());

    return doc.toByteArray(4);
}

} // namespace kodi
} // namespace mediaelch
