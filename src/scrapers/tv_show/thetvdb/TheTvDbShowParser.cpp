#include "TheTvDbShowParser.h"

#include "globals/Helper.h"
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

void TheTvDbShowParser::parseInfos(const QString& json)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    const auto showData = parsedJson.value("data").toObject();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing TheTvDb show data" << parseError.errorString();
        return;
    }

    m_show.setTvdbId(TvDbId(showData.value("id").toInt()));
    m_show.setImdbId(ImdbId(showData.value("imdbId").toString()));

    const auto cert = Certification(showData.value("rating").toString());
    m_show.setCertification(helper::mapCertification(cert));

    // TheTVDb month and day don't have a leading zero
    m_show.setFirstAired(QDate::fromString(showData.value("firstAired").toString(), "yyyy-M-d"));
    m_show.setNetwork(helper::mapStudio(showData.value("network").toString()));
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
    m_show.setGenres(helper::mapGenre(genres));
}

void TheTvDbShowParser::parseActors(const QString& json)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    const auto actors = parsedJson.value("data").toArray();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing TheTvDb actor data" << parseError.errorString();
        return;
    }

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

void TheTvDbShowParser::parseImages(const QString& json)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    const auto images = parsedJson.value("data").toArray();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing TheTvDb image data" << parseError.errorString();
        return;
    }

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
