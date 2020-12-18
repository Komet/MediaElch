#include "TvMazeShowParser.h"

#include "globals/Helper.h"
#include "scrapers/tv_show/tvmaze/TvMazeApi.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <chrono>

namespace mediaelch {
namespace scraper {

void TvMazeShowParser::parseInfos(const QJsonDocument& json)
{
    if (json.isEmpty() || !json.isObject()) {
        return;
    }

    QJsonObject data = json.object();

    m_show.setTvMazeId(TvMazeId(data["id"].toInt()));
    m_show.setTitle(data["name"].toString());
    m_show.setOverview(TvMazeApi::removeBasicHtmlElements(data["summary"].toString()));
    m_show.setFirstAired(QDate::fromString(data["premiered"].toString(), "yyyy-MM-dd"));

    // -------------------------------------
    {
        const QString status = data["status"].toString();
        if (!status.isEmpty()) {
            m_show.setStatus(status);
        }
    }

    // -------------------------------------
    {
        QJsonObject externalIds = data["externals"].toObject();
        ImdbId imdbId(externalIds["imdb"].toString());
        if (imdbId.isValid()) {
            m_show.setImdbId(imdbId);
        }
        TvDbId tvdbId(externalIds["thetvdb"].toInt());
        if (tvdbId.isValid()) {
            m_show.setTvdbId(tvdbId);
        }
    }

    // -------------------------------------
    {
        const int runtime = data["runtime"].toInt();
        if (runtime > 0) {
            m_show.setRuntime(std::chrono::minutes(runtime));
        }
    }

    // -------------------------------------
    {
        Rating rating;
        rating.source = "tvmaze";
        rating.minRating = 0;
        rating.maxRating = 10;
        rating.rating = data["rating"].toObject()["average"].toDouble();
        if (rating.rating > 0.0) {
            m_show.ratings().push_back(rating);
        }
    }

    // -------------------------------------
    {
        QJsonObject imageObj = data["image"].toObject();
        Poster showPoster;
        showPoster.id = imageObj["original"].toString();
        showPoster.originalUrl = showPoster.id;
        showPoster.thumbUrl = imageObj["medium"].toString();
        if (!showPoster.id.isEmpty() && showPoster.id.startsWith("http")) {
            m_show.addPoster(showPoster);
        }
    }

    // -------------------------------------

    QJsonObject embedded = data["_embedded"].toObject();

    {
        QJsonArray images = embedded["images"].toArray();

        for (QJsonValueRef posterVal : images) {
            QJsonObject posterObj = posterVal.toObject();
            QJsonObject resolutions = posterObj["resolutions"].toObject();

            Poster poster;
            poster.id = resolutions["original"].toObject()["url"].toString();
            poster.thumbUrl = resolutions["medium"].toObject()["url"].toString();
            poster.originalUrl = poster.id;
            poster.language = "en"; // only English is supported


            QString imageType = posterObj["type"].toString();

            if (imageType.isEmpty() || poster.id.isEmpty()) {
                continue;
            }

            if (imageType == "poster") {
                m_show.addPoster(poster);

            } else if (imageType == "banner") {
                m_show.addBanner(poster);

            } else if (imageType == "background") {
                m_show.addBackdrop(poster);
            }
        }
    }

    // -------------------------------------
    {
        QJsonArray seasonArray = embedded["seasons"].toArray();

        for (QJsonValueRef seasonVal : seasonArray) {
            QJsonObject seasonObj = seasonVal.toObject();

            const int seasonInt = seasonObj["number"].toInt(-1);
            if (seasonInt < 0) {
                continue;
            }

            SeasonNumber season = SeasonNumber(seasonInt);

            QJsonObject seasonImageObj = seasonObj["image"].toObject();

            Poster seasonPoster;
            seasonPoster.id = seasonImageObj["original"].toString();
            seasonPoster.thumbUrl = seasonImageObj["medium"].toString();
            seasonPoster.originalUrl = seasonPoster.id;
            seasonPoster.language = "en"; // only English is supported

            if (!seasonPoster.id.isEmpty()) {
                m_show.addSeasonPoster(season, seasonPoster);
            }
        }
    }

    // -------------------------------------
    {
        QJsonArray cast = embedded["cast"].toArray();
        for (QJsonValueRef val : cast) {
            QJsonObject castObj = val.toObject();
            QJsonObject person = castObj["person"].toObject();

            Actor actor;
            actor.name = person["name"].toString();
            actor.role = castObj["character"].toObject()["name"].toString();
            actor.id = QString::number(person["id"].toInt());
            actor.thumb = person["image"].toObject()["original"].toString();
            m_show.addActor(actor);
        }
    }

    // -------------------------------------
    {
        QJsonArray genres = data["genres"].toArray();
        for (QJsonValueRef genreRef : genres) {
            QString genre = genreRef.toString();
            if (!genre.isEmpty()) {
                m_show.addGenre(genre);
            }
        }
    }

    // -------------------------------------
    {
        QString network = data["network"].toObject()["name"].toString();
        if (network.isEmpty()) {
            network = data["webChannel"].toObject()["name"].toString();
        }
        if (!network.isEmpty()) {
            m_show.setNetwork(network);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
