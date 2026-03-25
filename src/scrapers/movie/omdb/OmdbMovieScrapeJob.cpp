#include "scrapers/movie/omdb/OmdbMovieScrapeJob.h"

#include "data/movie/Movie.h"
#include "scrapers/omdb/OmdbApi.h"

#include <QJsonArray>
#include <QJsonObject>

namespace mediaelch {
namespace scraper {

OmdbMovieScrapeJob::OmdbMovieScrapeJob(OmdbApi& api, MovieScrapeJob::Config _config, QObject* parent) :
    MovieScrapeJob(std::move(_config), parent), m_api{api}
{
}

void OmdbMovieScrapeJob::doStart()
{
    const QString& id = config().identifier.str();

    if (!ImdbId::isValidFormat(id)) {
        ScraperError error;
        error.error = ScraperError::Type::ConfigError;
        error.message = tr("OMDb requires an IMDb ID for loading movie details");
        setScraperError(error);
        emitFinished();
        return;
    }

    m_movie->setImdbId(ImdbId(id));

    m_api.loadMovie(ImdbId(id), [this](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignInfos(json);
        } else {
            setScraperError(error);
        }
        emitFinished();
    });
}

void OmdbMovieScrapeJob::parseAndAssignInfos(const QJsonDocument& json)
{
    QJsonObject obj = json.object();
    const auto& details = config().details;

    if (details.contains(MovieScraperInfo::Title)) {
        m_movie->setTitle(obj.value("Title").toString());
    }

    if (details.contains(MovieScraperInfo::Overview)) {
        const QString plot = obj.value("Plot").toString();
        if (plot != "N/A") {
            m_movie->setOverview(plot);
        }
    }

    if (details.contains(MovieScraperInfo::Released)) {
        // OMDb format: "01 Jan 2009"
        QDate date = QDate::fromString(obj.value("Released").toString(), "dd MMM yyyy");
        if (!date.isValid()) {
            // Fallback to year-only
            date = QDate::fromString(obj.value("Year").toString(), "yyyy");
        }
        if (date.isValid()) {
            m_movie->setReleased(date);
        }
    }

    if (details.contains(MovieScraperInfo::Runtime)) {
        // OMDb format: "148 min"
        const QString runtimeStr = obj.value("Runtime").toString();
        const QStringList parts = runtimeStr.split(' ');
        if (!parts.isEmpty()) {
            bool ok = false;
            int minutes = parts.first().toInt(&ok);
            if (ok) {
                m_movie->setRuntime(std::chrono::minutes(minutes));
            }
        }
    }

    if (details.contains(MovieScraperInfo::Certification)) {
        const QString rated = obj.value("Rated").toString();
        if (rated != "N/A") {
            m_movie->setCertification(Certification(rated));
        }
    }

    if (details.contains(MovieScraperInfo::Genres)) {
        // OMDb format: "Action, Adventure, Sci-Fi"
        const QString genreStr = obj.value("Genre").toString();
        if (genreStr != "N/A") {
            const QStringList genres = genreStr.split(", ");
            for (const QString& genre : genres) {
                m_movie->addGenre(genre.trimmed());
            }
        }
    }

    if (details.contains(MovieScraperInfo::Director)) {
        const QString director = obj.value("Director").toString();
        if (director != "N/A") {
            m_movie->setDirector(director);
        }
    }

    if (details.contains(MovieScraperInfo::Writer)) {
        const QString writer = obj.value("Writer").toString();
        if (writer != "N/A") {
            m_movie->setWriter(writer);
        }
    }

    if (details.contains(MovieScraperInfo::Actors)) {
        // OMDb format: "Actor1, Actor2, Actor3"
        const QString actorsStr = obj.value("Actors").toString();
        if (actorsStr != "N/A") {
            const QStringList actorNames = actorsStr.split(", ");
            QVector<Actor> actors;
            for (const QString& name : actorNames) {
                Actor a;
                a.name = name.trimmed();
                actors.push_back(a);
            }
            m_movie->setActors(actors);
        }
    }

    if (details.contains(MovieScraperInfo::Countries)) {
        // OMDb format: "United States, United Kingdom"
        const QString countryStr = obj.value("Country").toString();
        if (countryStr != "N/A") {
            const QStringList countries = countryStr.split(", ");
            for (const QString& country : countries) {
                m_movie->addCountry(country.trimmed());
            }
        }
    }

    if (details.contains(MovieScraperInfo::Poster)) {
        const QString posterUrl = obj.value("Poster").toString();
        if (posterUrl != "N/A" && !posterUrl.isEmpty()) {
            Poster p;
            p.originalUrl = QUrl(posterUrl);
            p.thumbUrl = QUrl(posterUrl);
            m_movie->images().addPoster(p);
        }
    }

    if (details.contains(MovieScraperInfo::Rating)) {
        parseRatings(obj);
    }
}

void OmdbMovieScrapeJob::parseRatings(const QJsonObject& obj)
{
    // OMDb provides up to 3 ratings: IMDB, Rotten Tomatoes, Metacritic

    // IMDB Rating (from the top-level fields)
    const QString imdbRatingStr = obj.value("imdbRating").toString();
    const QString imdbVotesStr = obj.value("imdbVotes").toString();
    if (imdbRatingStr != "N/A") {
        Rating imdbRating;
        imdbRating.source = "imdb";
        imdbRating.rating = imdbRatingStr.toDouble();
        imdbRating.maxRating = 10.0;
        imdbRating.voteCount = QString(imdbVotesStr).remove(',').toInt();
        m_movie->ratings().setOrAddRating(imdbRating);
    }

    // Rotten Tomatoes & Metacritic from the Ratings array
    const QJsonArray ratingsArray = obj.value("Ratings").toArray();
    for (const QJsonValue& val : ratingsArray) {
        QJsonObject ratingObj = val.toObject();
        const QString source = ratingObj.value("Source").toString();
        const QString value = ratingObj.value("Value").toString();

        if (source == "Rotten Tomatoes") {
            // Format: "85%"
            Rating rtRating;
            rtRating.source = "tomatometerallcritics";     // Kodi NFO standard source name
            rtRating.rating = value.chopped(1).toDouble(); // remove trailing '%'
            rtRating.maxRating = 100.0;
            m_movie->ratings().setOrAddRating(rtRating);

        } else if (source == "Metacritic") {
            // Format: "74/100"
            Rating mcRating;
            mcRating.source = "metacritic";
            mcRating.rating = value.split('/').first().toDouble();
            mcRating.maxRating = 100.0;
            m_movie->ratings().setOrAddRating(mcRating);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
