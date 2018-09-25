#include "TvShowXmlWriter.h"

#include "data/TvShow.h"
#include "globals/Helper.h"
#include "mediaCenterPlugins/XbmcXml.h"
#include "settings/Settings.h"

#include <QDomDocument>

namespace Kodi {

TvShowXmlWriter::TvShowXmlWriter(TvShow &tvShow) : m_show{tvShow}
{
}

QByteArray TvShowXmlWriter::getTvShowXml()
{
    QDomDocument doc;
    doc.setContent(m_show.nfoContent());
    if (m_show.nfoContent().isEmpty()) {
        QDomNode node = doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8" standalone="yes")");
        doc.insertBefore(node, doc.firstChild());
        doc.appendChild(doc.createElement("tvshow"));
    }

    QDomElement showElem = doc.elementsByTagName("tvshow").at(0).toElement();

    XbmcXml::setTextValue(doc, "title", m_show.name());
    XbmcXml::setTextValue(doc, "showtitle", m_show.showTitle());
    if (!m_show.sortTitle().isEmpty()) {
        QDomElement elem = XbmcXml::setTextValue(doc, "sorttitle", m_show.sortTitle());
        elem.setAttribute("clear", "true");
    } else {
        XbmcXml::removeChildNodes(doc, "sorttitle");
    }
    XbmcXml::setTextValue(doc, "rating", QString("%1").arg(m_show.rating()));
    XbmcXml::setTextValue(doc, "votes", QString::number(m_show.votes()));
    XbmcXml::setTextValue(doc, "top250", QString::number(m_show.top250()));
    XbmcXml::setTextValue(doc, "episode", QString("%1").arg(m_show.episodes().count()));
    XbmcXml::setTextValue(doc, "plot", m_show.overview());
    XbmcXml::setTextValue(doc, "outline", m_show.overview());
    XbmcXml::setTextValue(doc, "mpaa", QString("%1").arg(m_show.certification()));
    XbmcXml::setTextValue(doc, "premiered", m_show.firstAired().toString("yyyy-MM-dd"));
    XbmcXml::setTextValue(doc, "studio", m_show.network());
    XbmcXml::setTextValue(doc, "tvdbid", m_show.tvdbId());
    XbmcXml::setTextValue(doc, "id", m_show.id());
    XbmcXml::setTextValue(doc, "imdbid", m_show.imdbId());
    if (!m_show.status().isEmpty()) {
        XbmcXml::setTextValue(doc, "status", m_show.status());
    } else {
        XbmcXml::removeChildNodes(doc, "status");
    }
    if (m_show.runtime() > 0) {
        XbmcXml::setTextValue(doc, "runtime", QString("%1").arg(m_show.runtime()));
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
        XbmcXml::appendXmlNode(doc, elem);
    } else {
        XbmcXml::removeChildNodes(doc, "episodeguide");
    }

    XbmcXml::setTextValue(doc, "genre", m_show.genres().join(" / "));
    XbmcXml::setListValue(doc, "tag", m_show.tags());

    XbmcXml::removeChildNodes(doc, "actor");

    foreach (const Actor &actor, m_show.actors()) {
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

    if (Settings::instance()->advanced()->writeThumbUrlsToNfo()) {
        XbmcXml::removeChildNodes(doc, "thumb");
        XbmcXml::removeChildNodes(doc, "fanart");

        foreach (const Poster &poster, m_show.posters()) {
            QDomElement elem = doc.createElement("thumb");
            elem.setAttribute("preview", poster.thumbUrl.toString());
            elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            XbmcXml::appendXmlNode(doc, elem);

            QDomElement elemSeason = doc.createElement("thumb");
            elemSeason.setAttribute("type", "season");
            elemSeason.setAttribute("season", "-1");
            elemSeason.appendChild(doc.createTextNode(poster.originalUrl.toString()));
            XbmcXml::appendXmlNode(doc, elemSeason);
        }

        if (!m_show.backdrops().isEmpty()) {
            QDomElement fanartElem = doc.createElement("fanart");
            foreach (const Poster &poster, m_show.backdrops()) {
                QDomElement elem = doc.createElement("thumb");
                elem.setAttribute("preview", poster.thumbUrl.toString());
                elem.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                fanartElem.appendChild(elem);
            }
            XbmcXml::appendXmlNode(doc, fanartElem);
        }

        foreach (int season, m_show.seasons()) {
            foreach (const Poster &poster, m_show.seasonPosters(season)) {
                QDomElement elemSeason = doc.createElement("thumb");
                elemSeason.setAttribute("type", "season");
                elemSeason.setAttribute("season", QString("%1").arg(season));
                elemSeason.appendChild(doc.createTextNode(poster.originalUrl.toString()));
                XbmcXml::appendXmlNode(doc, elemSeason);
            }
        }
    }

    return doc.toByteArray(4);
}

} // namespace Kodi
