#include "EpisodeXmlReader.h"

#include "globals/Globals.h"
#include "tv_shows/TvShowEpisode.h"

#include <QDate>
#include <QDomElement>
#include <QFileInfo>
#include <QTime>
#include <QUrl>

namespace mediaelch {
namespace kodi {

EpisodeXmlReader::EpisodeXmlReader(TvShowEpisode& episode) : m_episode{episode}
{
}

void EpisodeXmlReader::parseNfoDom(QDomElement episodeDetails)
{
    // v17/v18 TvDbId
    if (!episodeDetails.elementsByTagName("id").isEmpty()) {
        m_episode.setTvdbId(TvDbId(episodeDetails.elementsByTagName("id").at(0).toElement().text()));
    }

    // v16 TvDbId/ImdbId
    if (!episodeDetails.elementsByTagName("tvdbid").isEmpty()) {
        QString value = episodeDetails.elementsByTagName("tvdbid").at(0).toElement().text();
        if (!value.isEmpty()) {
            m_episode.setTvdbId(TvDbId(value));
        }
    }
    if (!episodeDetails.elementsByTagName("imdbid").isEmpty()) {
        QString value = episodeDetails.elementsByTagName("imdbid").at(0).toElement().text();
        if (!value.isEmpty()) {
            m_episode.setImdbId(ImdbId(value));
        }
    }

    // v17 ids
    auto uniqueIds = episodeDetails.elementsByTagName("uniqueid");
    for (int i = 0; i < uniqueIds.size(); ++i) {
        QDomElement element = uniqueIds.at(i).toElement();
        QString type = element.attribute("type");
        QString value = element.text().trimmed();
        if (type == "imdb") {
            m_episode.setImdbId(ImdbId(value));
        } else if (type == "tvdb") {
            m_episode.setTvdbId(TvDbId(value));
        } else if (type == "tmdb") {
            m_episode.setTmdbId(TmdbId(value));
        } else if (type == "tvmaze") {
            m_episode.setTvMazeId(TvMazeId(value));
        } else {
            qWarning() << "[EpisodeXmlReader] Unsupported unique id type:" << type;
        }
    }

    if (!episodeDetails.elementsByTagName("title").isEmpty()) {
        m_episode.setTitle(episodeDetails.elementsByTagName("title").at(0).toElement().text());
    }
    if (!episodeDetails.elementsByTagName("showtitle").isEmpty()) {
        m_episode.setShowTitle(episodeDetails.elementsByTagName("showtitle").at(0).toElement().text());
    }
    if (!episodeDetails.elementsByTagName("season").isEmpty()) {
        m_episode.setSeason(SeasonNumber(episodeDetails.elementsByTagName("season").at(0).toElement().text().toInt()));
    }
    if (!episodeDetails.elementsByTagName("episode").isEmpty()) {
        m_episode.setEpisode(
            EpisodeNumber(episodeDetails.elementsByTagName("episode").at(0).toElement().text().toInt()));
    }
    if (!episodeDetails.elementsByTagName("displayseason").isEmpty()) {
        m_episode.setDisplaySeason(
            SeasonNumber(episodeDetails.elementsByTagName("displayseason").at(0).toElement().text().toInt()));
    }
    if (!episodeDetails.elementsByTagName("displayepisode").isEmpty()) {
        m_episode.setDisplayEpisode(
            EpisodeNumber(episodeDetails.elementsByTagName("displayepisode").at(0).toElement().text().toInt()));
    }

    // check for new ratings syntax
    if (!episodeDetails.elementsByTagName("ratings").isEmpty()) {
        // <ratings>
        //   <rating name="default" default="true">
        //     <value>10</value>
        //     <votes>10</votes>
        //   </rating>
        // </ratings>
        auto ratings = episodeDetails.elementsByTagName("ratings").at(0).toElement().elementsByTagName("rating");
        m_episode.ratings().clear();

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
            m_episode.ratings().push_back(rating);
            m_episode.setChanged(true);
        }

    } else if (!episodeDetails.elementsByTagName("rating").isEmpty()) {
        // otherwise use "old" syntax:
        // <rating>10.0</rating>
        // <votes>10.0</votes>
        QString value = episodeDetails.elementsByTagName("rating").at(0).toElement().text();
        if (!value.isEmpty()) {
            Rating rating;
            rating.rating = value.replace(",", ".").toDouble();
            if (!episodeDetails.elementsByTagName("votes").isEmpty()) {
                rating.voteCount = episodeDetails.elementsByTagName("votes")
                                       .at(0)
                                       .toElement()
                                       .text()
                                       .replace(",", "")
                                       .replace(".", "")
                                       .toInt();
            }
            m_episode.ratings().clear();
            m_episode.ratings().push_back(rating);
            m_episode.setChanged(true);
        }
    }

    if (!episodeDetails.elementsByTagName("top250").isEmpty()) {
        m_episode.setTop250(episodeDetails.elementsByTagName("top250").at(0).toElement().text().toInt());
    }
    if (!episodeDetails.elementsByTagName("plot").isEmpty()) {
        m_episode.setOverview(episodeDetails.elementsByTagName("plot").at(0).toElement().text());
    }
    if (!episodeDetails.elementsByTagName("mpaa").isEmpty()) {
        m_episode.setCertification(Certification(episodeDetails.elementsByTagName("mpaa").at(0).toElement().text()));
    }
    if (!episodeDetails.elementsByTagName("aired").isEmpty()) {
        const QDomElement aired = episodeDetails.elementsByTagName("aired").at(0).toElement();
        if (!aired.isNull() && !aired.text().isEmpty()) {
            const QDate date = QDate::fromString(aired.text(), "yyyy-MM-dd");
            if (date.isValid()) {
                m_episode.setFirstAired(date);
            }
        }
    }
    if (!episodeDetails.elementsByTagName("playcount").isEmpty()) {
        m_episode.setPlayCount(episodeDetails.elementsByTagName("playcount").at(0).toElement().text().toInt());
    }
    if (!episodeDetails.elementsByTagName("epbookmark").isEmpty()) {
        m_episode.setEpBookmark(
            QTime(0, 0, 0).addSecs(episodeDetails.elementsByTagName("epbookmark").at(0).toElement().text().toInt()));
    }
    if (!episodeDetails.elementsByTagName("lastplayed").isEmpty()) {
        const QDomElement lastplayed = episodeDetails.elementsByTagName("lastplayed").at(0).toElement();
        if (!lastplayed.isNull() && !lastplayed.text().isEmpty()) {
            const QDateTime dateTime = QDateTime::fromString(lastplayed.text(), "yyyy-MM-dd HH:mm:ss");
            if (dateTime.isValid()) {
                m_episode.setLastPlayed(dateTime);
            } else {
                const QDateTime date = QDateTime::fromString(lastplayed.text(), "yyyy-MM-dd");
                if (date.isValid()) {
                    m_episode.setLastPlayed(date);
                }
            }
        }
    }
    if (!episodeDetails.elementsByTagName("studio").isEmpty()) {
        m_episode.setNetwork(episodeDetails.elementsByTagName("studio").at(0).toElement().text());
    }

    // tags are officially not yet supported, even by Kodi 19 but scraper providers start
    // to support them
    for (int i = 0, n = episodeDetails.elementsByTagName("tag").size(); i < n; i++) {
        m_episode.addTag(episodeDetails.elementsByTagName("tag").at(i).toElement().text());
    }

    if (!episodeDetails.elementsByTagName("thumb").isEmpty()) {
        m_episode.setThumbnail(QUrl(episodeDetails.elementsByTagName("thumb").at(0).toElement().text()));
    }
    for (int i = 0, n = episodeDetails.elementsByTagName("credits").size(); i < n; i++) {
        m_episode.addWriter(episodeDetails.elementsByTagName("credits").at(i).toElement().text());
    }
    for (int i = 0, n = episodeDetails.elementsByTagName("director").size(); i < n; i++) {
        m_episode.addDirector(episodeDetails.elementsByTagName("director").at(i).toElement().text());
    }
    for (int i = 0, n = episodeDetails.elementsByTagName("actor").size(); i < n; i++) {
        QDomElement actorElement = episodeDetails.elementsByTagName("actor").at(i).toElement();
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
        m_episode.addActor(a);
    }
}

QString EpisodeXmlReader::makeValidEpisodeXml(const QString& nfoContent)
{
    QString def;
    QStringList baseNfoContent;
    const auto& lines = nfoContent.split("\n");
    for (const QString& line : lines) {
        if (!line.startsWith("<?xml")) {
            baseNfoContent << line;
        } else {
            def = line;
        }
    }

    // This is a HACK around Kodi's invalid XML files. There can only be ONE root element in valid XML files.
    // see https://kodi.wiki/view/NFO_files/TV_shows#TV_Episode_Tags
    const QString nfoContentWithRoot =
        QStringLiteral("%1\n<episodes>\n  %2</episodes>\n").arg(def, baseNfoContent.join("\n"));

    return nfoContentWithRoot;
}

} // namespace kodi
} // namespace mediaelch
