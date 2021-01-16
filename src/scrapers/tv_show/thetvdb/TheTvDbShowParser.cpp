#include "TheTvDbShowParser.h"

#include "scrapers/tv_show/thetvdb/TheTvDbApi.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <chrono>

namespace mediaelch {
namespace scraper {

void TheTvDbShowParser::parseInfos(const QJsonObject& json)
{
    const auto showData = json.value("data").toObject();

    m_show.setTvdbId(TvDbId(showData.value("id").toInt()));
    m_show.setImdbId(ImdbId(showData.value("imdbId").toString()));

    m_show.setCertification(Certification(showData.value("rating").toString()));

    // TheTVDb month and day don't have a leading zero
    m_show.setFirstAired(QDate::fromString(showData.value("firstAired").toString(), "yyyy-M-d"));
    m_show.setNetwork(showData.value("network").toString());
    m_show.setOverview(showData.value("overview").toString());

    Rating rating;
    rating.rating = showData.value("siteRating").toDouble();
    rating.voteCount = showData.value("siteRatingCount").toInt();
    rating.source = "tvdb";
    rating.minRating = 0;
    rating.maxRating = 10;
    // \todo currently only one rating is supported
    m_show.ratings().clear();
    m_show.ratings().push_back(rating);

    m_show.setTitle(showData.value("seriesName").toString().trimmed());


    const auto runtime = std::chrono::minutes(showData.value("runtime").toString().toInt());
    m_show.setRuntime(runtime);

    m_show.setStatus(showData.value("status").toString());

    QStringList genres;
    const auto jsonGenres = showData.value("genre").toArray();
    for (const auto& jsonGenre : jsonGenres) {
        const QString genre = jsonGenre.toString();
        if (!genre.isEmpty()) {
            genres << genre;
        }
    }
    m_show.setGenres(genres);
}

void TheTvDbShowParser::parseActors(const QJsonObject& json)
{
    const auto actors = json.value("data").toArray();

    for (const auto& actorValue : actors) {
        const auto actorObj = actorValue.toObject();

        Actor actor;
        actor.id = QString::number(actorObj.value("id").toInt());
        actor.name = actorObj.value("name").toString();
        actor.role = actorObj.value("role").toString();
        actor.thumb = TheTvDbApi::makeFullAssetUrl("/banners/" + actorObj.value("image").toString()).toString();

        m_show.addActor(actor);
    }
}

void TheTvDbShowParser::parseImages(const QJsonObject& json)
{
    const auto images = json.value("data").toArray();

    for (const auto& imageValue : images) {
        const auto imageObj = imageValue.toObject();
        const QString keyType = imageObj.value("keyType").toString();

        Poster p;
        p.id = QString::number(imageObj.value("id").toInt());
        p.originalUrl = TheTvDbApi::makeFullAssetUrl("/banners/" + imageObj.value("fileName").toString());
        p.thumbUrl = TheTvDbApi::makeFullAssetUrl("/banners/" + imageObj.value("thumbnail").toString());

        const QStringList resolution = imageObj.value("resolution").toString().split('x');
        if (resolution.length() == 2) {
            p.originalSize.setWidth(resolution[0].toInt());
            p.originalSize.setHeight(resolution[1].toInt());
        }

        if (keyType == "fanart") {
            m_show.addBackdrop(p);

        } else if (keyType == "poster") {
            m_show.addPoster(p);

        } else if (keyType == "season") {
            const auto season = SeasonNumber(imageObj.value("subKey").toString().toInt());
            m_show.addSeasonPoster(season, p);

        } else if (keyType == "seasonwide") {
            const auto season = SeasonNumber(imageObj.value("subKey").toString().toInt());
            m_show.addSeasonBanner(season, p);

        } else if (keyType == "series") {
            m_show.addBanner(p);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
