#include "scrapers/movie/imdb/ImdbMovieScrapeJob.h"

#include "data/movie/Movie.h"
#include "log/Log.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/imdb/ImdbJsonParser.h"
#include "utils/Containers.h"

namespace mediaelch {
namespace scraper {

ImdbMovieScrapeJob::ImdbMovieScrapeJob(ImdbApi& api,
    MovieScrapeJob::Config _config,
    bool loadAllTags,
    QObject* parent) :
    MovieScrapeJob(std::move(_config), parent),
    m_api{api},
    m_imdbId{config().identifier.str()},
    m_loadAllTags{loadAllTags}
{
}

void ImdbMovieScrapeJob::doStart()
{
    m_movie->clear(config().details);
    m_movie->setImdbId(m_imdbId);

    m_api.loadTitleViaGraphQL(m_imdbId, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
            return;
        }
        parseAndAssignInfos(data);
        emitFinished();
    });
}

void ImdbMovieScrapeJob::parseAndAssignInfos(const QString& json)
{
    ImdbData data = ImdbJsonParser::parseFromGraphQL(json, config().locale);

    if (data.imdbId.isValid()) {
        m_movie->setImdbId(data.imdbId);
    }

    // Title: use localized title if available, keep original as originalTitle
    if (data.localizedTitle.hasValue()) {
        m_movie->setTitle(data.localizedTitle.value);
        if (data.originalTitle.hasValue()) {
            m_movie->setOriginalTitle(data.originalTitle.value);
        } else if (data.title.hasValue()) {
            m_movie->setOriginalTitle(data.title.value);
        }
    } else if (data.title.hasValue()) {
        m_movie->setTitle(data.title.value);
        if (data.originalTitle.hasValue()) {
            m_movie->setOriginalTitle(data.originalTitle.value);
        }
    }

    if (data.overview.hasValue()) {
        m_movie->setOverview(data.overview.value);
    }
    if (data.outline.hasValue()) {
        m_movie->setOutline(data.outline.value);
    }
    if (data.tagline.hasValue()) {
        m_movie->setTagline(data.tagline.value);
    }
    if (data.runtime.hasValue()) {
        m_movie->setRuntime(data.runtime.value);
    }
    if (data.released.hasValue()) {
        m_movie->setReleased(data.released.value);
    }
    for (const Rating& rating : data.ratings) {
        m_movie->ratings().addRating(rating);
    }

    m_movie->setTop250(data.top250.getOrValue(-1));

    if (data.certification.hasValue()) {
        m_movie->setCertification(data.certification.value);
    }
    if (data.poster.hasValue()) {
        m_movie->images().addPoster(data.poster.value);
    }
    for (const Poster& backdrop : data.backdrops) {
        m_movie->images().addBackdrop(backdrop);
    }
    if (data.trailer.hasValue()) {
        m_movie->setTrailer(data.trailer.value);
    }
    for (const Actor& actor : data.actors) {
        m_movie->addActor(actor);
    }
    if (!data.directors.isEmpty()) {
        m_movie->setDirector(setToStringList(data.directors).join(", "));
    }
    if (!data.writers.isEmpty()) {
        m_movie->setWriter(setToStringList(data.writers).join(", "));
    }
    for (const QString& genre : data.genres) {
        m_movie->addGenre(genre);
    }
    for (const QString& studio : data.studios) {
        m_movie->addStudio(studio);
    }
    for (const QString& country : data.countries) {
        m_movie->addCountry(country);
    }
    if (!m_loadAllTags) {
        // When "Load All Tags" is disabled, only add tags that are part of IMDB's
        // default set (first ~20). The GraphQL query fetches up to 100 keywords.
        // Since we can't distinguish "default" from "extended" keywords, we limit
        // to the first 20 when the setting is off.
        int tagLimit = 20;
        int tagCount = 0;
        for (const QString& keyword : data.keywords) {
            if (tagCount >= tagLimit) {
                break;
            }
            m_movie->addTag(keyword);
            ++tagCount;
        }
    } else {
        for (const QString& keyword : data.keywords) {
            m_movie->addTag(keyword);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
