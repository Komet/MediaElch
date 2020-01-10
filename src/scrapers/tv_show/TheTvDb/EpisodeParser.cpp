#include "EpisodeParser.h"

#include "globals/Helper.h"
#include "scrapers/tv_show/TheTvDb/ApiRequest.h"
#include "settings/Settings.h"
#include "tv_shows/TvShowEpisode.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace thetvdb {

void EpisodeParser::parseInfos(const QString& json)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    const auto episodeObj = parsedJson.value("data").toObject();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[TheTvDb][EpisodeParser] Error parsing TheTvDb episode data:" << parseError.errorString();
        return;
    }

    parseInfos(episodeObj);
}

void EpisodeParser::parseInfos(const QJsonObject& episodeObj)
{
    m_episode.setTvdbId(TvDbId(episodeObj.value("id").toInt()));
    m_episode.setImdbId(ImdbId(episodeObj.value("imdbId").toString()));

    const bool isDvdOrder = Settings::instance()->tvShowDvdOrder();

    // See TvShowEpisode constructor for initial values
    if (m_episode.season() == SeasonNumber::NoSeason) {
        const auto season = episodeObj.value(isDvdOrder ? "dvdSeason" : "airedSeason").toInt(-1);
        m_episode.setSeason(season >= 0 ? SeasonNumber(season) : SeasonNumber::NoSeason);
    }
    if (m_episode.episode() == EpisodeNumber::NoEpisode) {
        const auto episode = episodeObj.value(isDvdOrder ? "dvdEpisodeNumber" : "airedEpisodeNumber").toInt(-1);
        m_episode.setEpisode(episode >= 0 ? EpisodeNumber(episode) : EpisodeNumber::NoEpisode);
    }

    if (m_infosToLoad.contains(TvShowScraperInfos::Director)) {
        QStringList directors;
        const auto directorsArray = episodeObj.value("directors").toArray();
        for (const auto& directorValue : directorsArray) {
            QString director = directorValue.toString().trimmed();
            if (!director.isEmpty()) {
                directors.append(director);
            }
        }
        m_episode.setDirectors(directors);
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Title)) {
        m_episode.setName(episodeObj.value("episodeName").toString());
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::FirstAired)) {
        // TheTVDb month and day don't have a leading zero
        m_episode.setFirstAired(QDate::fromString(episodeObj.value("firstAired").toString(), "yyyy-M-d"));
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Overview)) {
        m_episode.setOverview(episodeObj.value("overview").toString());
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Rating)) {
        Rating rating;
        rating.rating = episodeObj.value("siteRating").toDouble();
        rating.voteCount = episodeObj.value("siteRatingCount").toInt();
        rating.maxRating = 10;
        rating.minRating = 0;
        rating.source = "tvdb";
        // @todo currently only one rating is supported
        m_episode.ratings().clear();
        m_episode.ratings().push_back(rating);
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Writer)) {
        QStringList writers;
        const auto writersArray = episodeObj.value("writers").toArray();
        for (const auto& writerValue : writersArray) {
            QString writer = writerValue.toString().trimmed();
            if (!writer.isEmpty()) {
                writers.append(writer);
            }
        }
        m_episode.setWriters(writers);
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Thumbnail)) {
        m_episode.setThumbnail(
            ApiRequest::getFullAssetUrl(QStringLiteral("/banners/%2").arg(episodeObj.value("filename").toString())));
    }

    m_episode.setInfosLoaded(true);
}

void EpisodeParser::parseIdFromSeason(const QString& json)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    const auto seasonData = parsedJson.value("data").toArray();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[TheTvDb][EpisodeParser] Error parsing TheTvDb season data" << parseError.errorString();
        return;
    }

    QString seasonKey = "airedSeason";
    QString episodeKey = "airedEpisodeNumber";
    if (Settings::instance()->tvShowDvdOrder()) {
        seasonKey = "dvdSeason";
        episodeKey = "dvdEpisodeNumber";
    }

    for (const auto& episodeValue : seasonData) {
        const auto episode = episodeValue.toObject();
        const auto seasonNumber = SeasonNumber(episode.value(seasonKey).toInt(-2));
        const auto episodeNumber = EpisodeNumber(episode.value(episodeKey).toInt(-2));
        if (seasonNumber == m_episode.season() && episodeNumber == m_episode.episode()) {
            m_episode.setTvdbId(TvDbId(episode.value("id").toInt(-2)));
            break;
        }
    }
}

} // namespace thetvdb
