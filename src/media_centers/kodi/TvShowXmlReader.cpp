#include "TvShowXmlReader.h"

#include "globals/Poster.h"
#include "tv_shows/TvShow.h"

#include <QDate>
#include <QDateTime>
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
        m_show.setTvdbId(TvDbId(domDoc.elementsByTagName("id").at(0).toElement().text()));
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
        } else if (type == "tmdb") {
            m_show.setTmdbId(TmdbId(value));
        } else if (type == "tvmaze") {
            m_show.setTvMazeId(TvMazeId(value));
        } else {
            qWarning() << "[TvShowXmlReader] Unsupported unique id type:" << type;
        }
    }
    if (!domDoc.elementsByTagName("title").isEmpty()) {
        m_show.setTitle(domDoc.elementsByTagName("title").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("sorttitle").isEmpty()) {
        m_show.setSortTitle(domDoc.elementsByTagName("sorttitle").at(0).toElement().text());
    }
    // since v17
    if (!domDoc.elementsByTagName("originaltitle").isEmpty()) {
        m_show.setOriginalTitle(domDoc.elementsByTagName("originaltitle").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("showtitle").isEmpty()) {
        m_show.setShowTitle(domDoc.elementsByTagName("showtitle").at(0).toElement().text());
    }
    QDomNodeList namedSeasons = domDoc.elementsByTagName("namedseason");
    for (int i = 0; i < namedSeasons.size(); ++i) {
        QDomElement seasonElement = namedSeasons.at(i).toElement();
        SeasonNumber season(seasonElement.attribute("number", SeasonNumber::NoSeason.toString()).toInt());
        QString name = seasonElement.text();
        if (season != SeasonNumber::NoSeason) {
            m_show.setSeasonName(season, name);
        }
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
        m_show.ratings().clear();

        for (int i = 0; i < ratings.length(); ++i) {
            Rating rating;
            auto ratingElement = ratings.at(i).toElement();
            rating.source = ratingElement.attribute("name", "default");
            bool ok = false;
            const int max = ratingElement.attribute("max", "0").toInt(&ok);
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
    if (!domDoc.elementsByTagName("userrating").isEmpty()) {
        m_show.setUserRating(domDoc.elementsByTagName("userrating").at(0).toElement().text().toDouble());
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
        QString value = domDoc.elementsByTagName("premiered").at(0).toElement().text().trimmed();
        QDate released = QDate::fromString(value, "yyyy-MM-dd");
        if (released.isValid()) {
            m_show.setFirstAired(released);
        }
    }
    if (!domDoc.elementsByTagName("dateadded").isEmpty()) {
        m_show.setDateAdded(QDateTime::fromString(
            domDoc.elementsByTagName("dateadded").at(0).toElement().text(), "yyyy-MM-dd HH:mm:ss"));
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
        const auto genres =
            domDoc.elementsByTagName("genre").at(i).toElement().text().split(" / ", ElchSplitBehavior::SkipEmptyParts);
        for (const QString& genre : genres) {
            m_show.addGenre(genre);
        }
    }
    for (int i = 0, n = domDoc.elementsByTagName("tag").size(); i < n; i++) {
        m_show.addTag(domDoc.elementsByTagName("tag").at(i).toElement().text());
    }
    for (int i = 0, n = domDoc.elementsByTagName("actor").size(); i < n; i++) {
        QDomElement actorElement = domDoc.elementsByTagName("actor").at(i).toElement();
        Actor a;
        a.imageHasChanged = false;
        if (!actorElement.elementsByTagName("name").isEmpty()) {
            a.name = actorElement.elementsByTagName("name").at(0).toElement().text();
        }
        if (!actorElement.elementsByTagName("role").isEmpty()) {
            a.role = actorElement.elementsByTagName("role").at(0).toElement().text();
        }
        if (!actorElement.elementsByTagName("thumb").isEmpty()) {
            a.thumb = actorElement.elementsByTagName("thumb").at(0).toElement().text();
        }
        if (!actorElement.elementsByTagName("order").isEmpty()) {
            a.order = actorElement.elementsByTagName("order").at(0).toElement().text().toInt();
        }
        m_show.addActor(a);
    }
    for (int i = 0, n = domDoc.elementsByTagName("thumb").size(); i < n; i++) {
        QString parentTag = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().tagName();
        QDomElement thumbElement = domDoc.elementsByTagName("thumb").at(i).toElement();

        if (parentTag == "tvshow") {
            showThumb(thumbElement);

        } else if (parentTag == "fanart") {
            QString url = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().attribute("url");
            showFanartThumb(thumbElement, url);
        }
    }

    QFileInfo fi(m_show.dir().filePath("theme.mp3"));
    m_show.setHasTune(fi.isFile());
}

void TvShowXmlReader::showThumb(const QDomElement& element)
{
    QString aspect = element.attribute("aspect", "poster").toLower().trimmed();

    Poster p;
    p.originalUrl = QUrl(element.text());
    p.thumbUrl = element.attribute("preview");
    p.language = element.attribute("language");
    p.aspect = aspect;

    if (element.hasAttribute("type") && element.attribute("type").toLower() == "season") {
        SeasonNumber season = SeasonNumber(element.attribute("season").toInt());
        if (season != SeasonNumber::NoSeason) {
            p.season = season;
            if (aspect == "banner") {
                m_show.addSeasonBanner(season, p);
            } else {
                m_show.addSeasonPoster(season, p);
            }
        }
        return;
    }

    if (aspect == "banner") {
        m_show.addBanner(p);
        return;
    }

    m_show.addPoster(p);
}

void TvShowXmlReader::showFanartThumb(const QDomElement& element, QString thumbUrl)
{
    Poster p;
    p.originalUrl = QUrl(thumbUrl + element.text());
    if (!element.attribute("preview").isEmpty()) {
        p.thumbUrl = QUrl(thumbUrl + element.attribute("preview"));
    }
    QStringList dimensions = element.attribute("dim").split("x");
    if (dimensions.size() == 2) {
        QSize size;
        size.setWidth(dimensions.first().toInt());
        size.setHeight(dimensions.last().toInt());
        p.originalSize = size;
    }

    m_show.addBackdrop(p);
}

} // namespace kodi
} // namespace mediaelch
