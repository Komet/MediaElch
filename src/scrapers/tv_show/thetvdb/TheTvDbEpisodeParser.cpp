#include "TheTvDbEpisodeParser.h"

#include "globals/Helper.h"
#include "scrapers/tv_show/thetvdb/TheTvDbApi.h"
#include "tv_shows/TvShowEpisode.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace mediaelch {
namespace scraper {

void TheTvDbEpisodeParser::parseInfos(const QJsonDocument& json)
{
    const auto parsedJson = json.object();
    const auto episodeObj = parsedJson.value("data").toObject();

    parseInfos(episodeObj);
}

void TheTvDbEpisodeParser::parseInfos(const QJsonObject& episodeObj)
{
    m_episode.setTvdbId(TvDbId(episodeObj.value("id").toInt(-1)));
    m_episode.setImdbId(ImdbId(episodeObj.value("imdbId").toString()));

    const bool isDvdOrder = (m_order == SeasonOrder::Dvd);

    // See TvShowEpisode constructor for initial values
    if (m_episode.seasonNumber() == SeasonNumber::NoSeason) {
        const auto season = episodeObj.value(isDvdOrder ? "dvdSeason" : "airedSeason").toInt(-1);
        m_episode.setSeason(season >= 0 ? SeasonNumber(season) : SeasonNumber::NoSeason);
    }
    if (m_episode.episodeNumber() == EpisodeNumber::NoEpisode) {
        const auto episode = episodeObj.value(isDvdOrder ? "dvdEpisodeNumber" : "airedEpisodeNumber").toInt(-1);
        m_episode.setEpisode(episode >= 0 ? EpisodeNumber(episode) : EpisodeNumber::NoEpisode);
    }

    {
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

    m_episode.setTitle(episodeObj.value("episodeName").toString());

    // TheTVDb month and day don't have a leading zero
    m_episode.setFirstAired(QDate::fromString(episodeObj.value("firstAired").toString(), "yyyy-M-d"));

    m_episode.setOverview(episodeObj.value("overview").toString());

    {
        Rating rating;
        rating.rating = episodeObj.value("siteRating").toDouble();
        rating.voteCount = episodeObj.value("siteRatingCount").toInt(0);
        rating.maxRating = 10;
        rating.minRating = 0;
        rating.source = "tvdb";
        // \todo currently only one rating is supported
        m_episode.ratings().clear();
        m_episode.ratings().push_back(rating);
    }
    {
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

    Certification certification(episodeObj.value("contentRating").toString());
    if (certification.isValid()) {
        m_episode.setCertification(certification);
    }

    { // Only guest stars are available, not actors...
        QStringList guestStars;
        const auto guestStarsArray = episodeObj.value("guestStars").toArray();
        for (const auto& writerValue : guestStarsArray) {
            QString guestStarName = writerValue.toString().trimmed();
            if (!guestStarName.isEmpty()) {
                Actor guestStar;
                guestStar.name = guestStarName;
                m_episode.addActor(guestStar);
            }
        }
    }

    m_episode.setThumbnail(
        TheTvDbApi::makeFullAssetUrl(QStringLiteral("/banners/%2").arg(episodeObj.value("filename").toString())));

    m_episode.setInfosLoaded(true);
}

void TheTvDbEpisodeParser::parseIdFromSeason(const QJsonDocument& json)
{
    const auto parsedJson = json.object();
    const auto seasonData = parsedJson.value("data").toArray();

    const bool isDvdOrder = (m_order == SeasonOrder::Dvd);

    const QString seasonKey = isDvdOrder ? "dvdSeason" : "airedSeason";
    const QString episodeKey = isDvdOrder ? "dvdEpisodeNumber" : "airedEpisodeNumber";

    for (const auto& episodeValue : seasonData) {
        const auto episode = episodeValue.toObject();
        const auto seasonNumber = SeasonNumber(episode.value(seasonKey).toInt(-2));
        const auto episodeNumber = EpisodeNumber(episode.value(episodeKey).toInt(-2));
        if (seasonNumber == m_episode.seasonNumber() && episodeNumber == m_episode.episodeNumber()) {
            const int id = episode.value("id").toInt(-1);
            if (id >= 0) {
                m_episode.setTvdbId(TvDbId(id));
            }
            break;
        }
    }
}

} // namespace scraper
} // namespace mediaelch
