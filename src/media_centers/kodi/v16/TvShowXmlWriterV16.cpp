#include "media_centers/kodi/v16/TvShowXmlWriterV16.h"

#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "settings/Settings.h"
#include "tv_shows/TvShow.h"

#include <QDomDocument>

namespace mediaelch {
namespace kodi {

TvShowXmlWriterV16::TvShowXmlWriterV16(TvShow& tvShow) : m_show{tvShow}
{
}

QByteArray TvShowXmlWriterV16::getTvShowXml()
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

    if (!m_show.ratings().empty()) {
        // v16 only supports one rating/vote node
        const auto& rating = m_show.ratings().front();
        KodiXml::setTextValue(doc, "rating", QString::number(rating.rating));
        KodiXml::setTextValue(doc, "votes", QString::number(rating.voteCount));
    } else {
        KodiXml::setTextValue(doc, "rating", "");
        KodiXml::setTextValue(doc, "votes", "");
    }

    KodiXml::setTextValue(doc, "top250", QString::number(m_show.top250()));
    KodiXml::setTextValue(doc, "episode", QString("%1").arg(m_show.episodes().count()));
    KodiXml::setTextValue(doc, "plot", m_show.overview());
    KodiXml::setTextValue(doc, "outline", m_show.overview());
    KodiXml::setTextValue(doc, "mpaa", m_show.certification().toString());
    KodiXml::setTextValue(doc, "premiered", m_show.firstAired().toString("yyyy-MM-dd"));
    KodiXml::setTextValue(doc, "studio", m_show.network());
    KodiXml::setTextValue(doc, "tvdbid", m_show.tvdbId().toString());
    KodiXml::setTextValue(doc, "id", m_show.id().toString());
    KodiXml::setTextValue(doc, "imdbid", m_show.imdbId().toString());
    if (!m_show.status().isEmpty()) {
        KodiXml::setTextValue(doc, "status", m_show.status());
    } else {
        KodiXml::removeChildNodes(doc, "status");
    }
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

    KodiXml::setTextValue(doc, "genre", m_show.genres().join(" / "));
    KodiXml::setListValue(doc, "tag", m_show.tags());

    KodiXml::removeChildNodes(doc, "actor");

    for (const Actor* actor : m_show.actors()) {
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

    KodiXml::removeChildNodes(doc, "thumb");
    KodiXml::removeChildNodes(doc, "fanart");

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
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

    return doc.toByteArray(4);
}

} // namespace kodi
} // namespace mediaelch
