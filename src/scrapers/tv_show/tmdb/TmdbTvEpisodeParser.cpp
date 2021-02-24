#include "scrapers/tv_show/tmdb/TmdbTvEpisodeParser.h"

#include "globals/Poster.h"
#include "scrapers/tmdb/TmdbApi.h"
#include "tv_shows/TvDbId.h"
#include "tv_shows/TvShowEpisode.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValueRef>

namespace mediaelch {
namespace scraper {

void TmdbTvEpisodeParser::parseInfos(const TmdbApi& api, TvShowEpisode& episode, const QJsonObject& data)
{
    if (data.isEmpty()) {
        return;
    }

    const int episodeNumber = data["episode_number"].toInt(-1);
    const int seasonNumber = data["season_number"].toInt(-1);
    if (episodeNumber < 0 || seasonNumber < 0) {
        return;
    }

    episode.setEpisode(EpisodeNumber(episodeNumber));
    episode.setSeason(SeasonNumber(seasonNumber));
    episode.setTmdbId(TmdbId(QString::number(data["id"].toInt(-1))));
    episode.setTitle(data["name"].toString());
    episode.setOverview(data["overview"].toString());
    episode.setFirstAired(QDate::fromString(data["air_date"].toString(), "yyyy-MM-dd"));

    // -------------------------------------
    QJsonObject externalIds = data["external_ids"].toObject();

    ImdbId imdbId(externalIds["imdb_id"].toString());
    if (imdbId.isValid()) {
        episode.setImdbId(imdbId);
    }

    QString tvdbId(QStringLiteral("id%1").arg(externalIds["tvdb_id"].toInt(-1)));
    if (TvDbId::isValidPrefixedFormat(tvdbId)) {
        episode.setTvdbId(TvDbId(tvdbId));
    }

    // -------------------------------------
    Rating rating;
    rating.source = "tmdb";
    rating.minRating = 0;
    rating.maxRating = 10;
    rating.voteCount = data["vote_count"].toInt(0);
    rating.rating = data["vote_average"].toDouble();
    if (rating.rating != 0.0 || rating.voteCount != 0) {
        episode.ratings().push_back(rating);
    }

    // -------------------------------------
    QString thumbnailSuffix = data.value("still_path").toString();
    if (!thumbnailSuffix.isEmpty()) {
        QUrl thumbnail = api.makeImageUrl(thumbnailSuffix).toString();
        episode.setThumbnail(thumbnail);
    }

    // -------------------------------------
    const QJsonObject credits = data["credits"].toObject();
    QJsonArray cast = credits["cast"].toArray();

    for (QJsonValueRef val : cast) {
        QJsonObject actorObj = val.toObject();
        Actor actor;
        actor.name = actorObj["name"].toString();
        actor.role = actorObj["character"].toString();
        actor.id = QString::number(actorObj["id"].toInt(-1));
        actor.thumb = api.makeImageUrl(actorObj["profile_path"].toString()).toString();
        episode.addActor(actor);
    }

    QJsonArray guestStars = credits["guest_stars"].toArray();

    for (QJsonValueRef val : guestStars) {
        QJsonObject actorObj = val.toObject();
        Actor actor;
        actor.name = actorObj["name"].toString();
        actor.role = actorObj["character"].toString();
        actor.id = QString::number(actorObj["id"].toInt(-1));
        actor.thumb = api.makeImageUrl(actorObj["profile_path"].toString()).toString();
        episode.addActor(actor);
    }

    // -------------------------------------
    // Directors and Writers
    QJsonArray crew = credits["crew"].toArray();
    if (crew.isEmpty()) {
        // We request "credits" for single episode scrape jobs.  Then "credits"
        // should also contain "crew".  But if "credits" does not exist there
        // may still be "crew" on the top-level.
        crew = data["crew"].toArray();
    }

    for (QJsonValueRef val : crew) {
        QJsonObject crewObj = val.toObject();

        const QString name = crewObj["name"].toString();
        if (name.isEmpty()) {
            continue;
        }

        const QString type = crewObj["job"].toString();
        if (type == "Writer") {
            episode.addWriter(name);
        } else if (type == "Director") {
            episode.addDirector(name);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
