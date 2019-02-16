#include "EpisodeXmlReader.h"

#include "data/TvShowEpisode.h"
#include "globals/Globals.h"

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
        m_episode.setFirstAired(
            QDate::fromString(episodeDetails.elementsByTagName("aired").at(0).toElement().text(), "yyyy-MM-dd"));
    }
    if (!episodeDetails.elementsByTagName("playcount").isEmpty()) {
        m_episode.setPlayCount(episodeDetails.elementsByTagName("playcount").at(0).toElement().text().toInt());
    }
    if (!episodeDetails.elementsByTagName("epbookmark").isEmpty()) {
        m_episode.setEpBookmark(
            QTime(0, 0, 0).addSecs(episodeDetails.elementsByTagName("epbookmark").at(0).toElement().text().toInt()));
    }
    if (!episodeDetails.elementsByTagName("lastplayed").isEmpty()) {
        m_episode.setLastPlayed(QDateTime::fromString(
            episodeDetails.elementsByTagName("lastplayed").at(0).toElement().text(), "yyyy-MM-dd HH:mm:ss"));
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
        m_episode.addActor(a);
    }
}

} // namespace Kodi
