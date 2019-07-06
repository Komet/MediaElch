#include "media_centers/kodi/v17/TvShowXmlWriterV17.h"

#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "settings/Settings.h"
#include "tv_shows/TvShow.h"

#include <QDomDocument>

namespace mediaelch {
namespace kodi {

TvShowXmlWriterV17::TvShowXmlWriterV17(TvShow& tvShow) : m_show{tvShow}
{
}

QByteArray TvShowXmlWriterV17::getTvShowXml()
{
    using namespace std::chrono_literals;

    QDomDocument doc;
    doc.setContent(m_show.nfoContent());
    if (m_show.nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8" standalone="yes" )");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("tvshow"));
    }

    QDomElement showElem = doc.elementsByTagName("tvshow").at(0).toElement();

    KodiXml::setTextValue(doc, "title", m_show.name());
    KodiXml::setTextValue(doc, "showtitle", m_show.showTitle());
    if (!m_show.sortTitle().isEmpty()) {
        QDomElement elem = KodiXml::setTextValue(doc, "sorttitle", m_show.sortTitle());
        elem.setAttribute("clear", "true");
    } else {
        KodiXml::removeChildNodes(doc, "sorttitle");
    }
    // id
    KodiXml::setTextValue(doc, "id", m_show.id().toString());
    // unique id: IMDb and TMDb
    KodiXml::removeChildNodes(doc, "uniqueid");
    {
        QDomElement uniqueId = doc.createElement("uniqueid");
        uniqueId.setAttribute("type", "imdb");
        uniqueId.appendChild(doc.createTextNode(m_show.imdbId()));
        KodiXml::appendXmlNode(doc, uniqueId);
    }
    if (m_show.tvdbId().isValid()) {
        QDomElement uniqueId = doc.createElement("uniqueid");
        uniqueId.setAttribute("type", "tvdb");
        uniqueId.setAttribute("default", "true");
        uniqueId.appendChild(doc.createTextNode(m_show.tvdbId().toString()));
        KodiXml::appendXmlNode(doc, uniqueId);
    }
    // rating
    KodiXml::removeChildNodes(doc, "ratings");
    QDomElement ratings = doc.createElement("ratings");
    bool firstRating = true;
    for (const Rating& rating : m_show.ratings()) {
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

    KodiXml::setTextValue(doc, "top250", QString::number(m_show.top250()));
    KodiXml::setTextValue(doc, "episode", QString("%1").arg(m_show.episodes().count()));
    KodiXml::setTextValue(doc, "plot", m_show.overview());
    KodiXml::setTextValue(doc, "outline", m_show.overview());
    KodiXml::setTextValue(doc, "mpaa", m_show.certification().toString());
    KodiXml::setTextValue(doc, "premiered", m_show.firstAired().toString("yyyy-MM-dd"));
    KodiXml::setTextValue(doc, "studio", m_show.network());
    KodiXml::setTextValue(doc, "status", m_show.status());

    if (m_show.runtime() > 0min) {
        KodiXml::setTextValue(doc, "runtime", QString::number(m_show.runtime().count()));
    } else if (!showElem.elementsByTagName("runtime").isEmpty()) {
        showElem.removeChild(showElem.elementsByTagName("runtime").at(0));
    }

    if (!m_show.episodeGuideUrl().isEmpty()) {
        QDomNodeList childNodes = showElem.childNodes();
        for (int i = 0, n = childNodes.count(); i < n; ++i) {
            if (childNodes.at(i).nodeName() == "episodeguide") {
                showElem.removeChild(childNodes.at(i));
                break;
            }
        }
        QDomElement elem = doc.createElement("episodeguide");
        QDomElement elemUrl = doc.createElement("url");
        elemUrl.appendChild(doc.createTextNode(m_show.episodeGuideUrl()));
        elem.appendChild(elemUrl);
        KodiXml::appendXmlNode(doc, elem);
    } else {
        KodiXml::removeChildNodes(doc, "episodeguide");
    }
    // todo: one entry for each genre
    KodiXml::setTextValue(doc, "genre", m_show.genres().join(" / "));
    KodiXml::setListValue(doc, "tag", m_show.tags());

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        KodiXml::removeChildNodes(doc, "thumb");
        KodiXml::removeChildNodes(doc, "fanart");

        for (const Poster& poster : m_show.posters()) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            KodiXml::appendXmlNode(doc, elem);

            QDomElement elemSeason = doc.createElement("thumb");
            elemSeason.setAttribute("type", "season");
            elemSeason.setAttribute("season", "-1");
            elemSeason.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            KodiXml::appendXmlNode(doc, elemSeason);
        }

        if (!m_show.backdrops().isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            for (const Poster& poster : m_show.backdrops()) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());
                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            KodiXml::appendXmlNode(doc, fanartElem);
        }

        for (SeasonNumber season : m_show.seasons()) {
            for (const Poster& poster : m_show.seasonPosters(season)) {
                QDomElement elemSeason = doc.createElement("thumb");
                elemSeason.setAttribute("type", "season");
                elemSeason.setAttribute("season", season.toString());
                elemSeason.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                KodiXml::appendXmlNode(doc, elemSeason);
            }
        }
    }

    KodiXml::removeChildNodes(doc, "actor");

    int order = 0; // todo: save the order in the actor struct
    for (const Actor& actor : m_show.actors()) {
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

    return doc.toByteArray(4);
}

} // namespace kodi
} // namespace mediaelch
