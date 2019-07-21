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
        uniqueId.appendChild(doc.createTextNode(m_show.imdbId().toString()));
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

    KodiXml::setTextValue(doc, "userrating", QString::number(m_show.userRating()));
    KodiXml::setTextValue(doc, "top250", QString::number(m_show.top250()));
    KodiXml::setTextValue(doc, "episode", QString::number(m_show.episodes().count()));
    KodiXml::setTextValue(doc, "season", QString::number(m_show.seasons().count()));
    KodiXml::setTextValue(doc, "plot", m_show.overview());
    KodiXml::setTextValue(doc, "mpaa", m_show.certification().toString());
    KodiXml::setTextValue(doc, "premiered", m_show.firstAired().toString("yyyy-MM-dd"));
    KodiXml::setTextValue(doc, "year", m_show.firstAired().toString("yyyy"));
    KodiXml::setTextValue(doc, "dateadded", m_show.dateAdded().toString("yyyy-MM-dd HH:mm:ss"));
    KodiXml::setTextValue(doc, "status", m_show.status());
    KodiXml::setTextValue(doc, "studio", m_show.network());

    if (m_show.runtime() > 0min) {
        KodiXml::setTextValue(doc, "runtime", QString::number(m_show.runtime().count()));
    } else if (!showElem.elementsByTagName("runtime").isEmpty()) {
        showElem.removeChild(showElem.elementsByTagName("runtime").at(0));
    }

    // TODO: add trailer support
    KodiXml::setTextValue(doc, "trailer", "");

    KodiXml::removeChildNodes(doc, "namedseason");

    for (auto namedSeason = m_show.seasonNameMappings().constBegin();
         namedSeason != m_show.seasonNameMappings().constEnd();
         ++namedSeason) {
        QDomElement seasonElement = doc.createElement("namedseason");

        seasonElement.setAttribute("number", namedSeason.key().toString());
        seasonElement.appendChild(doc.createTextNode(namedSeason.value()));

        KodiXml::appendXmlNode(doc, seasonElement);
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
    KodiXml::removeChildNodes(doc, "genre");
    for (const QString& genre : m_show.genres()) {
        QDomElement elem = doc.createElement("genre");
        elem.appendChild(doc.createTextNode(genre));
        KodiXml::appendXmlNode(doc, elem);
    }
    KodiXml::setListValue(doc, "tag", m_show.tags());

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        KodiXml::removeChildNodes(doc, "thumb");
        KodiXml::removeChildNodes(doc, "fanart");

        for (const Poster& poster : m_show.posters()) {
            QDomElement elem = doc.createElement("thumb");
            QString aspect = poster.aspect.isEmpty() ? "poster" : poster.aspect;
            elem.setAttribute("aspect", aspect);
            if (!poster.thumbUrl.isEmpty()) {
                elem.setAttribute("preview", poster.thumbUrl.toString());
            }
            if (!poster.language.isEmpty()) {
                elem.setAttribute("language", poster.language);
            }
            if (poster.season != SeasonNumber::NoSeason) {
                elem.setAttribute("type", "season");
                elem.setAttribute("season", poster.season.toString());
            }
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            KodiXml::appendXmlNode(doc, elem);
        }

        for (const Poster& banner : m_show.banners()) {
            QDomElement elem = doc.createElement("thumb");
            QString aspect = banner.aspect.isEmpty() ? "banner" : banner.aspect;
            elem.setAttribute("aspect", aspect);
            if (!banner.thumbUrl.isEmpty()) {
                elem.setAttribute("preview", banner.thumbUrl.toString());
            }
            if (!banner.language.isEmpty()) {
                elem.setAttribute("language", banner.language);
            }
            elem.appendChild(doc.createTextNode(banner.originalUrl.toString()));
            KodiXml::appendXmlNode(doc, elem);
        }

        for (const Poster& poster : m_show.seasonPosters(SeasonNumber::NoSeason, true)) {
            if (poster.season != SeasonNumber::NoSeason) {
                QDomElement elemSeason = doc.createElement("thumb");
                elemSeason.setAttribute("aspect", poster.aspect.isEmpty() ? "poster" : poster.aspect);
                elemSeason.setAttribute("type", "season");
                elemSeason.setAttribute("season", poster.season.toString());
                if (!poster.language.isEmpty()) {
                    elemSeason.setAttribute("language", poster.language);
                }
                elemSeason.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                KodiXml::appendXmlNode(doc, elemSeason);
            }
        }

        for (const Poster& banner : m_show.seasonBanners(SeasonNumber::NoSeason, true)) {
            if (banner.season != SeasonNumber::NoSeason) {
                QDomElement elemSeason = doc.createElement("thumb");
                elemSeason.setAttribute("aspect", banner.aspect.isEmpty() ? "poster" : banner.aspect);
                elemSeason.setAttribute("type", "season");
                elemSeason.setAttribute("season", banner.season.toString());
                if (!banner.language.isEmpty()) {
                    elemSeason.setAttribute("language", banner.language);
                }
                elemSeason.appendChild(doc.createTextNode(banner.originalUrl.toString()));
                KodiXml::appendXmlNode(doc, elemSeason);
            }
        }

        if (!m_show.backdrops().isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            for (const Poster& poster : m_show.backdrops()) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());

                if (poster.width > 0 && poster.height > 0) {
                    elem.setAttribute("dim", QString("%1x%2").arg(poster.width).arg(poster.height));
                }

                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            KodiXml::appendXmlNode(doc, fanartElem);
        }
    }

    KodiXml::removeChildNodes(doc, "actor");

    for (const Actor& actor : m_show.actors()) {
        QDomElement elem = doc.createElement("actor");
        QDomElement elemName = doc.createElement("name");
        QDomElement elemRole = doc.createElement("role");
        QDomElement elemOrder = doc.createElement("order");
        elemName.appendChild(doc.createTextNode(actor.name));
        elemRole.appendChild(doc.createTextNode(actor.role));
        elemOrder.appendChild(doc.createTextNode(QString::number(actor.order)));
        elem.appendChild(elemName);
        elem.appendChild(elemRole);
        elem.appendChild(elemOrder);
        if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
            QDomElement elemThumb = doc.createElement("thumb");
            elemThumb.appendChild(doc.createTextNode(actor.thumb));
            elem.appendChild(elemThumb);
        }
        KodiXml::appendXmlNode(doc, elem);
    }

    return doc.toByteArray(4);
}

} // namespace kodi
} // namespace mediaelch
