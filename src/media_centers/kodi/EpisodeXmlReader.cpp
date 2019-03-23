#include "EpisodeXmlReader.h"

#include "globals/Globals.h"
#include "tvShows/TvShowEpisode.h"

#include <QDate>
#include <QDomElement>
#include <QFileInfo>
#include <QTime>
#include <QUrl>

namespace Kodi {

EpisodeXmlReader::EpisodeXmlReader(TvShowEpisode& episode) : m_episode{episode}
{
}

void EpisodeXmlReader::parseNfoDom(QDomDocument domDoc, QDomElement episodeDetails)
{
    if (!episodeDetails.elementsByTagName("imdbid").isEmpty()) {
        m_episode.setImdbId(ImdbId(episodeDetails.elementsByTagName("imdbid").at(0).toElement().text()));
    }
    if (!episodeDetails.elementsByTagName("title").isEmpty()) {
        m_episode.setName(episodeDetails.elementsByTagName("title").at(0).toElement().text());
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
    if (!episodeDetails.elementsByTagName("rating").isEmpty()) {
        m_episode.setRating(
            episodeDetails.elementsByTagName("rating").at(0).toElement().text().replace(",", ".").toDouble());
    }
    if (!domDoc.elementsByTagName("votes").isEmpty()) {
        m_episode.setVotes(
            domDoc.elementsByTagName("votes").at(0).toElement().text().replace(",", "").replace(".", "").toInt());
    }
    if (!domDoc.elementsByTagName("top250").isEmpty()) {
        m_episode.setTop250(domDoc.elementsByTagName("top250").at(0).toElement().text().toInt());
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
            const QDateTime date = QDateTime::fromString(lastplayed.text(), "yyyy-MM-dd HH:mm:ss");
            if (date.isValid()) {
                m_episode.setLastPlayed(date);
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
    if (!episodeDetails.elementsByTagName("thumb").isEmpty()) {
        m_episode.setThumbnail(QUrl(episodeDetails.elementsByTagName("thumb").at(0).toElement().text()));
    }
    for (int i = 0, n = episodeDetails.elementsByTagName("credits").size(); i < n; i++) {
        m_episode.addWriter(episodeDetails.elementsByTagName("credits").at(i).toElement().text());
    }
    for (int i = 0, n = episodeDetails.elementsByTagName("director").size(); i < n; i++) {
        m_episode.addDirector(episodeDetails.elementsByTagName("director").at(i).toElement().text());
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
        m_episode.addActor(a);
    }
}

} // namespace Kodi
