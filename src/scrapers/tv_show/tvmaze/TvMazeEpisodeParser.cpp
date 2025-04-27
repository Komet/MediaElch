#include "scrapers/tv_show/tvmaze/TvMazeEpisodeParser.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/tv_show/tvmaze/TvMazeApi.h"

#include <QJsonArray>

namespace mediaelch {
namespace scraper {

void TvMazeEpisodeParser::parseEpisode(TvShowEpisode& episode, const QJsonDocument& json)
{
    TvMazeEpisodeParser::parseEpisode(episode, json.object());
}

void TvMazeEpisodeParser::parseEpisode(TvShowEpisode& episode, const QJsonObject& json)
{
    if (json.isEmpty()) {
        return;
    }

    const int id = json["id"].toInt(-1);
    const int seasonNumber = json["season"].toInt(-1);
    const int episodeNumber = json["number"].toInt(-1);
    if (episodeNumber < 0 || seasonNumber < 0 || id < 0) {
        // If these details are not set then we can't properly scrape the episode.
        return;
    }

    episode.setTvMazeId(TvMazeId(QString::number(id)));
    episode.setEpisode(EpisodeNumber(episodeNumber));
    episode.setSeason(SeasonNumber(seasonNumber));
    episode.setTitle(json["name"].toString());
    episode.setOverview(TvMazeApi::removeBasicHtmlElements(json["summary"].toString()));
    episode.setFirstAired(QDate::fromString(json["airdate"].toString(), "yyyy-MM-dd"));

    const auto& ratingObj = json["rating"].toObject();
    if (ratingObj.contains("average")) {
        Rating rating;
        rating.source = "tvmaze";
        rating.rating = ratingObj["average"].toDouble(-1.0);
        rating.minRating = 0;
        rating.maxRating = 10;
        if (rating.rating >= 0.0) {
            episode.ratings().addRating(rating);
        }
    }

    // -------------------------------------

    QString thumbOriginal = json["image"].toObject()["original"].toString();
    if (!thumbOriginal.isEmpty() && thumbOriginal.startsWith("http")) {
        episode.setThumbnail(QUrl(thumbOriginal));
    }

    // -------------------------------------
    QJsonObject embedded = json["_embedded"].toObject();
    {
        // TODO: Combine with TvMazeShowParser
        const QJsonArray cast = embedded["guestcast"].toArray();
        for (const QJsonValue& val : cast) {
            QJsonObject castObj = val.toObject();
            QJsonObject person = castObj["person"].toObject();
            QJsonObject character = castObj["character"].toObject();

            Actor actor;
            actor.name = person["name"].toString();
            actor.role = character["name"].toString();
            actor.id = QString::number(person["id"].toInt());
            actor.thumb = person["image"].toObject()["original"].toString();
            if (actor.thumb.isEmpty()) { // no image of the person available -> use character
                actor.thumb = character["image"].toObject()["original"].toString();
            }
            episode.addActor(actor);
        }
    }
    {
        const QJsonArray crew = embedded["guestcrew"].toArray();
        for (const QJsonValue& val : crew) {
            QJsonObject castObj = val.toObject();
            QJsonObject person = castObj["person"].toObject();
            QString name = person["name"].toString();
            QString guestCrewType = castObj["guestCrewType"].toString();

            if (guestCrewType == "Writer") {
                episode.addWriter(name);
            }
            else if (guestCrewType == "Director") {
                episode.addDirector(name);
            }
        }
    }


    // TODO: Support "runtime" when TvShowEpisode supports it.
}

void TvMazeEpisodeParser::parseEpisodeFromOverview(TvShowEpisode& episode, const QJsonDocument& json)
{
    QJsonArray episodes = json.array();

    const int episodeNumber = episode.episodeNumber().toInt();
    const int seasonNumber = episode.seasonNumber().toInt();

    for (const QJsonValue& val : asConst(episodes)) {
        QJsonObject episodeObj = val.toObject();
        // Use default value -2 because -1 has special meaning.
        if (episodeObj["season"].toInt(-2) == seasonNumber && episodeObj["number"].toInt(-2) == episodeNumber) {
            TvMazeEpisodeParser::parseEpisode(episode, episodeObj);
            break;
        }
    }
}

} // namespace scraper
} // namespace mediaelch
