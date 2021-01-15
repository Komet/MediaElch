#include "scrapers/tv_show/tmdb/TmdbTvShowParser.h"

#include "globals/Helper.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/tmdb/TmdbApi.h"
#include "tv_shows/TvDbId.h"
#include "tv_shows/TvShow.h"

#include <QDate>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValueRef>
#include <QRegularExpression>

using namespace std::chrono_literals;

namespace mediaelch {
namespace scraper {

void TmdbTvShowParser::parseInfos(const QJsonDocument& json, const Locale& locale)
{
    if (json.isEmpty() || !json.isObject()) {
        return;
    }

    QJsonObject data = json.object();

    m_show.setTmdbId(TmdbId(QString::number(data["id"].toInt())));
    m_show.setTitle(data["name"].toString());
    m_show.setOriginalTitle(data["original_name"].toString());
    m_show.setOverview(data["overview"].toString());
    m_show.setFirstAired(QDate::fromString(data["first_air_date"].toString(), "yyyy-MM-dd"));

    // -------------------------------------
    {
        const QString status = data["status"].toString();
        if (!status.isEmpty()) {
            m_show.setStatus(status);
        } else {
            const bool isInProduction = data["in_production"].toBool(false);
            m_show.setStatus(isInProduction ? tr("Continuing") : tr("Ended"));
        }
    }

    // -------------------------------------
    QJsonObject externalIds = data["external_ids"].toObject();
    ImdbId imdbId(externalIds["imdb_id"].toString());
    if (imdbId.isValid()) {
        m_show.setImdbId(imdbId);
    }
    TvDbId tvdbId(QStringLiteral("id%1").arg(externalIds["tvdb_id"].toInt()));
    if (tvdbId.isValid()) {
        m_show.setTvdbId(tvdbId);
    }

    // -------------------------------------
    QJsonArray runtimes = data["episode_run_time"].toArray();
    if (!runtimes.empty()) {
        m_show.setRuntime(std::chrono::minutes(runtimes.first().toInt()));
    }

    // -------------------------------------
    {
        Rating rating;
        rating.source = "tmdb";
        rating.minRating = 0;
        rating.maxRating = 10;
        rating.voteCount = data["vote_count"].toInt();
        rating.rating = data["vote_average"].toDouble();
        if (rating.rating != 0.0 || rating.voteCount != 0) {
            m_show.ratings().push_back(rating);
        }
    }

    // -------------------------------------
    {
        QJsonObject keywordObj = data.value("keywords").toObject();
        QJsonArray keywordResults = keywordObj.value("results").toArray();
        for (QJsonValueRef tagVal : keywordResults) {
            QString tag = tagVal.toObject()["name"].toString();
            if (!tag.isEmpty()) {
                m_show.addTag(tag);
            }
        }
    }

    // -------------------------------------
    {
        Poster showPoster;
        showPoster.id = m_api.makeImageUrl(data.value("poster_path").toString()).toString();
        showPoster.thumbUrl = showPoster.id;
        showPoster.originalUrl = showPoster.id;
        if (!showPoster.id.isEmpty()) {
            m_show.addPoster(showPoster);
        }
    }
    {
        Poster showBackdrop;
        showBackdrop.id = m_api.makeImageUrl(data["backdrop_path"].toString()).toString();
        showBackdrop.thumbUrl = showBackdrop.id;
        showBackdrop.originalUrl = showBackdrop.id;
        if (!showBackdrop.id.isEmpty()) {
            m_show.addBackdrop(showBackdrop);
        }
    }

    QJsonObject images = data["images"].toObject();
    {
        QJsonArray posters = images["posters"].toArray();
        for (QJsonValueRef posterVal : posters) {
            QJsonObject posterObj = posterVal.toObject();
            Poster poster;
            poster.id = m_api.makeImageUrl(posterObj.value("file_path").toString()).toString();
            poster.thumbUrl = poster.id;
            poster.originalUrl = poster.id;
            poster.language = posterObj["iso_639_1"].toString();
            poster.aspect = QString::number(posterObj["aspect"].toDouble());
            poster.originalSize = {posterObj["width"].toInt(), posterObj["height"].toInt()};
            if (!poster.id.isEmpty()) {
                m_show.addPoster(poster);
            }
        }
    }
    {
        QJsonArray backdrops = images["backdrops"].toArray();
        for (QJsonValueRef backdropVal : backdrops) {
            QJsonObject backdropObj = backdropVal.toObject();
            Poster backdrop;
            backdrop.id = m_api.makeImageUrl(backdropObj.value("file_path").toString()).toString();
            backdrop.thumbUrl = backdrop.id;
            backdrop.originalUrl = backdrop.id;
            backdrop.language = backdropObj["iso_639_1"].toString();
            backdrop.aspect = QString::number(backdropObj["aspect"].toDouble());
            backdrop.originalSize = {backdropObj["width"].toInt(), backdropObj["height"].toInt()};
            if (!backdrop.id.isEmpty()) {
                m_show.addBackdrop(backdrop);
            }
        }
    }

    // -------------------------------------
    {
        QJsonArray seasonArray = data["seasons"].toArray();

        for (QJsonValueRef seasonVal : seasonArray) {
            QJsonObject seasonObj = seasonVal.toObject();

            const int seasonInt = seasonObj["season_number"].toInt();
            if (seasonInt < 0) {
                continue;
            }

            SeasonNumber season = SeasonNumber(seasonInt);

            Poster seasonPoster;
            seasonPoster.id = m_api.makeImageUrl(seasonObj["poster_path"].toString()).toString();
            seasonPoster.thumbUrl = seasonPoster.id;
            seasonPoster.originalUrl = seasonPoster.id;
            if (!seasonPoster.id.isEmpty()) {
                m_show.addSeasonPoster(season, seasonPoster);
            }
        }
    }

    // -------------------------------------
    {
        QJsonArray genres = data["genres"].toArray();
        for (QJsonValueRef genreRef : genres) {
            QString genre = genreRef.toObject()["name"].toString();
            if (!genre.isEmpty()) {
                m_show.addGenre(genre);
            }
        }
    }

    // -------------------------------------
    {
        const QJsonObject credits = data["credits"].toObject();
        QJsonArray cast = credits["cast"].toArray();

        for (QJsonValueRef val : cast) {
            QJsonObject actorObj = val.toObject();
            Actor actor;
            actor.name = actorObj["name"].toString();
            actor.role = actorObj["character"].toString();
            actor.id = QString::number(actorObj["id"].toInt());
            actor.thumb = m_api.makeImageUrl(actorObj["profile_path"].toString()).toString();
            m_show.addActor(actor);
        }
    }

    // -------------------------------------
    {
        QJsonArray networks = data["networks"].toArray();
        QStringList gatheredNetworks;
        for (QJsonValueRef val : networks) {
            QString name = val.toObject()["name"].toString();
            gatheredNetworks.append(name);
        }
        if (!gatheredNetworks.isEmpty()) {
            m_show.setNetwork(gatheredNetworks.join(", "));
        }
    }

    // -------------------------------------
    {
        // There is a content rating (aka certification) for each country.
        // We try to find the certification for the requested country.  If that cannot
        // be found we use the US certification.  If the US version does not exist
        // then simply take the first one to be on a safe side.
        QJsonArray certifications = data["content_ratings"].toObject()["results"].toArray();
        Certification cert;
        bool foundCert = false;
        const QString& country = locale.country();
        for (QJsonValueRef val : certifications) {
            const QJsonObject certification = val.toObject();
            const QString isoCountry = certification["iso_3166_1"].toString();
            if (isoCountry == country || isoCountry == "US") {
                cert = Certification(certification["rating"].toString());
                foundCert = true;
                if (isoCountry == country) {
                    break;
                }
            }
        }
        if (!foundCert) {
            cert = Certification(certifications.first().toObject()["rating"].toString());
        }
        m_show.setCertification(cert);
    }
}


} // namespace scraper
} // namespace mediaelch
