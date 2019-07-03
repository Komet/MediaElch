#include "TvShowXmlReader.h"

#include "tv_shows/TvShow.h"

#include <QDate>
#include <QDomDocument>
#include <QFileInfo>
#include <QUrl>

namespace mediaelch {
namespace kodi {

TvShowXmlReader::TvShowXmlReader(TvShow& tvShow) : m_show{tvShow}
{
}

void TvShowXmlReader::parseNfoDom(QDomDocument domDoc)
{
    if (!domDoc.elementsByTagName("id").isEmpty()) {
        m_show.setId(TvDbId(domDoc.elementsByTagName("id").at(0).toElement().text()));
    }
    if (!domDoc.elementsByTagName("tvdbid").isEmpty()) {
        m_show.setTvdbId(TvDbId(domDoc.elementsByTagName("tvdbid").at(0).toElement().text()));
    }
    if (!domDoc.elementsByTagName("imdbid").isEmpty()) {
        m_show.setImdbId(domDoc.elementsByTagName("imdbid").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("title").isEmpty()) {
        m_show.setName(domDoc.elementsByTagName("title").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("sorttitle").isEmpty()) {
        m_show.setSortTitle(domDoc.elementsByTagName("sorttitle").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("showtitle").isEmpty()) {
        m_show.setShowTitle(domDoc.elementsByTagName("showtitle").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("rating").isEmpty()) {
        m_show.setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().replace(",", ".").toDouble());
    }
    if (!domDoc.elementsByTagName("votes").isEmpty()) {
        m_show.setVotes(
            domDoc.elementsByTagName("votes").at(0).toElement().text().replace(",", "").replace(".", "").toInt());
    }
    if (!domDoc.elementsByTagName("top250").isEmpty()) {
        m_show.setTop250(domDoc.elementsByTagName("top250").at(0).toElement().text().toInt());
    }
    if (!domDoc.elementsByTagName("plot").isEmpty()) {
        m_show.setOverview(domDoc.elementsByTagName("plot").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("mpaa").isEmpty()) {
        m_show.setCertification(Certification(domDoc.elementsByTagName("mpaa").at(0).toElement().text()));
    }
    if (!domDoc.elementsByTagName("premiered").isEmpty()) {
        m_show.setFirstAired(
            QDate::fromString(domDoc.elementsByTagName("premiered").at(0).toElement().text(), "yyyy-MM-dd"));
    }
    if (!domDoc.elementsByTagName("studio").isEmpty()) {
        m_show.setNetwork(domDoc.elementsByTagName("studio").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("episodeguide").isEmpty()
        && !domDoc.elementsByTagName("episodeguide").at(0).toElement().elementsByTagName("url").isEmpty()) {
        m_show.setEpisodeGuideUrl(domDoc.elementsByTagName("episodeguide")
                                      .at(0)
                                      .toElement()
                                      .elementsByTagName("url")
                                      .at(0)
                                      .toElement()
                                      .text());
    }
    if (!domDoc.elementsByTagName("runtime").isEmpty()) {
        m_show.setRuntime(std::chrono::minutes(domDoc.elementsByTagName("runtime").at(0).toElement().text().toInt()));
    }
    if (!domDoc.elementsByTagName("status").isEmpty()) {
        m_show.setStatus(domDoc.elementsByTagName("status").at(0).toElement().text());
    }

    for (int i = 0, n = domDoc.elementsByTagName("genre").size(); i < n; i++) {
        for (const QString& genre :
            domDoc.elementsByTagName("genre").at(i).toElement().text().split(" / ", QString::SkipEmptyParts)) {
            m_show.addGenre(genre);
        }
    }
    for (int i = 0, n = domDoc.elementsByTagName("tag").size(); i < n; i++) {
        m_show.addTag(domDoc.elementsByTagName("tag").at(i).toElement().text());
    }
    for (int i = 0, n = domDoc.elementsByTagName("actor").size(); i < n; i++) {
        Actor a;
        a.imageHasChanged = false;
        if (!domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("name").isEmpty()) {
            a.name =
                domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("name").at(0).toElement().text();
        }
        if (!domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("role").isEmpty()) {
            a.role =
                domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("role").at(0).toElement().text();
        }
        if (!domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("thumb").isEmpty()) {
            a.thumb =
                domDoc.elementsByTagName("actor").at(i).toElement().elementsByTagName("thumb").at(0).toElement().text();
        }
        m_show.addActor(a);
    }
    for (int i = 0, n = domDoc.elementsByTagName("thumb").size(); i < n; i++) {
        QString parentTag = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().tagName();
        if (parentTag == "tvshow") {
            QDomElement elem = domDoc.elementsByTagName("thumb").at(i).toElement();
            Poster p;
            p.originalUrl = QUrl(elem.text());
            p.thumbUrl = QUrl(elem.text());
            if (elem.hasAttribute("type") && elem.attribute("type") == "season") {
                SeasonNumber season = SeasonNumber(elem.attribute("season").toInt());
                if (season != SeasonNumber::NoSeason) {
                    m_show.addSeasonPoster(season, p);
                }
            } else {
                m_show.addPoster(p);
            }
        } else if (parentTag == "fanart") {
            QString url = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().attribute("url");
            Poster p;
            p.originalUrl = QUrl(url + domDoc.elementsByTagName("thumb").at(i).toElement().text());
            p.thumbUrl = QUrl(url + domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            m_show.addBackdrop(p);
        }
    }

    QFileInfo fi(m_show.dir() + "/theme.mp3");
    m_show.setHasTune(fi.isFile());
}

} // namespace kodi
} // namespace mediaelch
