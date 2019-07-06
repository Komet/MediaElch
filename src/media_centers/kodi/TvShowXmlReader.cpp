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
    // v17/v18 TvDbId
    if (!domDoc.elementsByTagName("id").isEmpty()) {
        m_show.setId(TvDbId(domDoc.elementsByTagName("id").at(0).toElement().text()));
    }
    // v16 TvDbId/ImdbId
    if (!domDoc.elementsByTagName("tvdbid").isEmpty()) {
        QString value = domDoc.elementsByTagName("tvdbid").at(0).toElement().text();
        if (!value.isEmpty()) {
            m_show.setTvdbId(TvDbId(value));
        }
    }
    if (!domDoc.elementsByTagName("imdbid").isEmpty()) {
        QString value = domDoc.elementsByTagName("imdbid").at(0).toElement().text();
        if (!value.isEmpty()) {
            m_show.setImdbId(ImdbId(value));
        }
    }
    // v17 ids
    auto uniqueIds = domDoc.elementsByTagName("uniqueid");
    for (int i = 0; i < uniqueIds.size(); ++i) {
        QDomElement element = uniqueIds.at(i).toElement();
        QString type = element.attribute("type");
        QString value = element.text().trimmed();
        if (type == "imdb") {
            m_show.setImdbId(ImdbId(value));
        } else if (type == "tvdb") {
            m_show.setTvdbId(TvDbId(value));
        }
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
    // check for new ratings syntax
    if (!domDoc.elementsByTagName("ratings").isEmpty()) {
        // <ratings>
        //   <rating name="default" default="true">
        //     <value>10</value>
        //     <votes>10</votes>
        //   </rating>
        // </ratings>
        auto ratings = domDoc.elementsByTagName("ratings").at(0).toElement().elementsByTagName("rating");
        for (int i = 0; i < ratings.length(); ++i) {
            Rating rating;
            auto ratingElement = ratings.at(i).toElement();
            rating.source = ratingElement.attribute("name", "default");
            bool ok = false;
            int max = ratingElement.attribute("max", "0").toInt(&ok);
            if (ok && max > 0) {
                rating.maxRating = max;
            }
            rating.rating =
                ratingElement.elementsByTagName("value").at(0).toElement().text().replace(",", ".").toDouble();
            rating.voteCount = ratingElement.elementsByTagName("votes")
                                   .at(0)
                                   .toElement()
                                   .text()
                                   .replace(",", "")
                                   .replace(".", "")
                                   .toInt();
            m_show.ratings().push_back(rating);
            m_show.setChanged(true);
        }

    } else if (!domDoc.elementsByTagName("rating").isEmpty()) {
        // otherwise use "old" syntax:
        // <rating>10.0</rating>
        // <votes>10.0</votes>
        QString value = domDoc.elementsByTagName("rating").at(0).toElement().text();
        if (!value.isEmpty()) {
            Rating rating;
            rating.rating = value.replace(",", ".").toDouble();
            if (!domDoc.elementsByTagName("votes").isEmpty()) {
                rating.voteCount = domDoc.elementsByTagName("votes")
                                       .at(0)
                                       .toElement()
                                       .text()
                                       .replace(",", "")
                                       .replace(".", "")
                                       .toInt();
            }
            m_show.ratings().clear();
            m_show.ratings().push_back(rating);
            m_show.setChanged(true);
        }
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
    if (!domDoc.elementsByTagName("year").isEmpty()) {
        m_show.setFirstAired(QDate::fromString(domDoc.elementsByTagName("year").at(0).toElement().text(), "yyyy"));
    }
    // will override the first-aired date set by <year>
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
