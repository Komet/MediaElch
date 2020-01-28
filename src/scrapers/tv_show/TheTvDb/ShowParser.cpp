#include "ShowParser.h"

#include "globals/Helper.h"
#include "scrapers/tv_show/TheTvDb/ApiRequest.h"
#include "scrapers/tv_show/TheTvDb/EpisodeParser.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <chrono>

namespace thetvdb {

void ShowParser::parseInfos(const QString& json)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    const auto showData = parsedJson.value("data").toObject();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing TheTvDb show data" << parseError.errorString();
        return;
    }

    m_show.setId(TvDbId(showData.value("id").toInt()));
    m_show.setTvdbId(TvDbId(showData.value("id").toInt()));
    m_show.setImdbId(ImdbId(showData.value("imdbId").toString()));

    if (m_infosToLoad.contains(TvShowScraperInfos::Certification)) {
        const auto cert = Certification(showData.value("rating").toString());
        m_show.setCertification(helper::mapCertification(cert));
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::FirstAired)) {
        // TheTVDb month and day don't have a leading zero
        m_show.setFirstAired(QDate::fromString(showData.value("firstAired").toString(), "yyyy-M-d"));
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Network)) {
        m_show.setNetwork(helper::mapStudio(showData.value("network").toString()));
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Overview)) {
        m_show.setOverview(showData.value("overview").toString());
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Rating)) {
        Rating rating;
        rating.rating = showData.value("siteRating").toDouble();
        rating.voteCount = showData.value("siteRatingCount").toInt();
        rating.source = "tvdb";
        rating.minRating = 0;
        rating.maxRating = 10;
        // @todo currently only one rating is supported
        m_show.ratings().clear();
        m_show.ratings().push_back(rating);
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Title)) {
        m_show.setName(showData.value("seriesName").toString().trimmed());
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Runtime)) {
        const auto runtime = std::chrono::minutes(showData.value("runtime").toString().toInt());
        m_show.setRuntime(runtime);
    }
    if (m_infosToLoad.contains(TvShowScraperInfos::Status)) {
        m_show.setStatus(showData.value("status").toString());
    }

    if (m_infosToLoad.contains(TvShowScraperInfos::Genres)) {
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
}

void ShowParser::parseActors(const QString& json)
{
    if (!m_infosToLoad.contains(TvShowScraperInfos::Actors)) {
        return;
    }

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
        actor.thumb = ApiRequest::getFullAssetUrl("/banners/" + actorObj.value("image").toString()).toString();

        m_show.addActor(actor);
    }
}

void ShowParser::parseImages(const QString& json)
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

        p.originalUrl = ApiRequest::getFullAssetUrl("/banners/" + imageObj.value("fileName").toString());
        p.thumbUrl = ApiRequest::getFullAssetUrl("/banners/" + imageObj.value("thumbnail").toString());

        const QStringList resolution = imageObj.value("resolution").toString().split('x');
        if (resolution.length() == 2) {
            p.originalSize.setWidth(resolution[0].toInt());
            p.originalSize.setHeight(resolution[1].toInt());
        }

        if (keyType == "fanart" && m_infosToLoad.contains(TvShowScraperInfos::Fanart)) {
            m_show.addBackdrop(p);

        } else if (keyType == "poster" && m_infosToLoad.contains(TvShowScraperInfos::Poster)) {
            m_show.addPoster(p);

        } else if (keyType == "season" && m_infosToLoad.contains(TvShowScraperInfos::SeasonPoster)) {
            const auto season = SeasonNumber(imageObj.value("subKey").toString().toInt());
            m_show.addSeasonPoster(season, p);

        } else if (keyType == "seasonwide" && m_infosToLoad.contains(TvShowScraperInfos::SeasonBanner)) {
            const auto season = SeasonNumber(imageObj.value("subKey").toString().toInt());
            m_show.addSeasonBanner(season, p);

        } else if (keyType == "series"
                   && (m_infosToLoad.contains(TvShowScraperInfos::Banner)
                       || m_infosToLoad.contains(TvShowScraperInfos::SeasonBanner))) {
            m_show.addBanner(p);
        }
    }
}

/**
 * @brief Parses episodes from the json and stores them in this object.
 * @see ShowParser::episodes()
 */
Paginate ShowParser::parseEpisodes(const QString& json, QVector<TvShowScraperInfos> episodeInfosToLoad)
{
    if (!m_show.tvdbId().isValid()) {
        qWarning() << "[TheTvDb][ShowParser] Can't parse episodes without TheTvDb id:" << m_show.tvdbId().toString();
        return Paginate{};
    }

    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    const auto paginateObj = parsedJson.value("links").toObject();
    const auto episodesArray = parsedJson.value("data").toArray();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[TheTvDb][ShowParser] Error parsing TheTvDb episode data:" << parseError.errorString() << json;
        return Paginate{};
    }

    for (const auto& episodeValue : episodesArray) {
        const auto episodeObj = episodeValue.toObject();
        auto episode = std::make_unique<TvShowEpisode>();
        EpisodeParser parser(*episode, episodeInfosToLoad);
        parser.parseInfos(episodeObj);
        m_episodes.push_back(std::move(episode));
    }

    Paginate p;
    p.first = paginateObj.value("first").toInt();
    p.last = paginateObj.value("last").toInt();
    p.next = paginateObj.value("next").toInt();
    p.prev = paginateObj.value("prev").toInt();
    return p;
}

} // namespace thetvdb
