#include "scrapers/tv_show/omdb/OmdbTvShowScrapeJob.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/omdb/OmdbApi.h"

#include <QJsonArray>
#include <QJsonObject>

namespace mediaelch {
namespace scraper {

OmdbTvShowScrapeJob::OmdbTvShowScrapeJob(OmdbApi& api, ShowScrapeJob::Config _config, QObject* parent) :
    ShowScrapeJob(std::move(_config), parent), m_api{api}
{
}

void OmdbTvShowScrapeJob::doStart()
{
    const QString& id = config().identifier.str();

    if (!ImdbId::isValidFormat(id)) {
        ScraperError error;
        error.error = ScraperError::Type::ConfigError;
        error.message = tr("OMDb requires an IMDb ID for loading show details");
        setScraperError(error);
        emitFinished();
        return;
    }

    m_api.loadShow(ImdbId(id), [this](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignInfos(json);
        } else {
            setScraperError(error);
        }
        emitFinished();
    });
}

void OmdbTvShowScrapeJob::parseAndAssignInfos(const QJsonDocument& json)
{
    QJsonObject obj = json.object();
    const auto& details = config().details;

    if (details.contains(ShowScraperInfo::Title)) {
        m_tvShow->setTitle(obj.value("Title").toString());
    }

    if (details.contains(ShowScraperInfo::Overview)) {
        const QString plot = obj.value("Plot").toString();
        if (plot != "N/A") {
            m_tvShow->setOverview(plot);
        }
    }

    if (details.contains(ShowScraperInfo::FirstAired)) {
        // OMDb format: "01 Jan 2009"
        QDate date = QDate::fromString(obj.value("Released").toString(), "dd MMM yyyy");
        if (!date.isValid()) {
            date = QDate::fromString(obj.value("Year").toString().left(4), "yyyy");
        }
        if (date.isValid()) {
            m_tvShow->setFirstAired(date);
        }
    }

    if (details.contains(ShowScraperInfo::Runtime)) {
        const QString runtimeStr = obj.value("Runtime").toString();
        const QStringList parts = runtimeStr.split(' ');
        if (!parts.isEmpty()) {
            bool ok = false;
            int minutes = parts.first().toInt(&ok);
            if (ok) {
                m_tvShow->setRuntime(std::chrono::minutes(minutes));
            }
        }
    }

    if (details.contains(ShowScraperInfo::Certification)) {
        const QString rated = obj.value("Rated").toString();
        if (rated != "N/A") {
            m_tvShow->setCertification(Certification(rated));
        }
    }

    if (details.contains(ShowScraperInfo::Genres)) {
        const QString genreStr = obj.value("Genre").toString();
        if (genreStr != "N/A") {
            const QStringList genres = genreStr.split(", ");
            for (const QString& genre : genres) {
                m_tvShow->addGenre(genre.trimmed());
            }
        }
    }

    if (details.contains(ShowScraperInfo::Actors)) {
        const QString actorsStr = obj.value("Actors").toString();
        if (actorsStr != "N/A") {
            const QStringList actorNames = actorsStr.split(", ");
            for (const QString& name : actorNames) {
                Actor a;
                a.name = name.trimmed();
                m_tvShow->addActor(a);
            }
        }
    }

    if (details.contains(ShowScraperInfo::Poster)) {
        const QString posterUrl = obj.value("Poster").toString();
        if (posterUrl != "N/A" && !posterUrl.isEmpty()) {
            Poster p;
            p.originalUrl = QUrl(posterUrl);
            p.thumbUrl = QUrl(posterUrl);
            m_tvShow->addPoster(p);
        }
    }

    if (details.contains(ShowScraperInfo::Rating)) {
        parseRatings(obj);
    }
}

void OmdbTvShowScrapeJob::parseRatings(const QJsonObject& obj)
{
    const QString imdbRatingStr = obj.value("imdbRating").toString();
    const QString imdbVotesStr = obj.value("imdbVotes").toString();
    if (imdbRatingStr != "N/A") {
        Rating imdbRating;
        imdbRating.source = "imdb";
        imdbRating.rating = imdbRatingStr.toDouble();
        imdbRating.maxRating = 10.0;
        imdbRating.voteCount = QString(imdbVotesStr).remove(',').toInt();
        m_tvShow->ratings().setOrAddRating(imdbRating);
    }

    const QJsonArray ratingsArray = obj.value("Ratings").toArray();
    for (const QJsonValue& val : ratingsArray) {
        QJsonObject ratingObj = val.toObject();
        const QString source = ratingObj.value("Source").toString();
        const QString value = ratingObj.value("Value").toString();

        if (source == "Rotten Tomatoes") {
            Rating rtRating;
            rtRating.source = "tomatometerallcritics";
            rtRating.rating = value.chopped(1).toDouble();
            rtRating.maxRating = 100.0;
            m_tvShow->ratings().setOrAddRating(rtRating);
        } else if (source == "Metacritic") {
            Rating mcRating;
            mcRating.source = "metacritic";
            mcRating.rating = value.split('/').first().toDouble();
            mcRating.maxRating = 100.0;
            m_tvShow->ratings().setOrAddRating(mcRating);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
