#include "scrapers/movie/omdb/OmdbMovie.h"

#include "scrapers/movie/omdb/OmdbMovieConfiguration.h"
#include "scrapers/movie/omdb/OmdbMovieScrapeJob.h"
#include "scrapers/movie/omdb/OmdbMovieSearchJob.h"

namespace mediaelch {
namespace scraper {

OmdbMovie::OmdbMovie(OmdbMovieConfiguration& settings, QObject* parent) :
    MovieScraper(parent),
    m_settings{settings},
    m_scraperNativelySupports{MovieScraperInfo::Title,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Certification,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Director,
        MovieScraperInfo::Writer,
        MovieScraperInfo::Countries}
{
    m_meta.identifier = ID;
    m_meta.name = "OMDb";
    m_meta.description = tr("The Open Movie Database (OMDb) is a free web service to obtain movie information. "
                            "It provides ratings from IMDB, Rotten Tomatoes, and Metacritic in a single API call. "
                            "Requires a free personal API key from omdbapi.com (free: 1,000 requests/day). "
                            "High-resolution poster images are available for OMDb Patreon supporters "
                            "(see patreon.com/omdb).");
    m_meta.website = "https://www.omdbapi.com/";
    m_meta.termsOfService = "https://www.omdbapi.com/legal.htm";
    m_meta.privacyPolicy = "https://www.omdbapi.com/legal.htm";
    m_meta.help = "https://www.omdbapi.com/";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Certification,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Director,
        MovieScraperInfo::Writer,
        MovieScraperInfo::Countries};
    m_meta.supportedLanguages = {Locale::English};
    m_meta.defaultLocale = Locale::English;
    m_meta.isAdult = false;

    // Update API key from settings and keep it in sync
    m_api.setApiKey(m_settings.apiKey());
    connect(&m_settings, &OmdbMovieConfiguration::apiKeyChanged, this, [this](const QString& apiKey) {
        m_api.setApiKey(apiKey);
    });

    initialize();
}

OmdbMovie::~OmdbMovie()
{
}

const MovieScraper::ScraperMeta& OmdbMovie::meta() const
{
    return m_meta;
}

void OmdbMovie::initialize()
{
    m_api.initialize();
}

bool OmdbMovie::isInitialized() const
{
    return m_api.isInitialized();
}

MovieSearchJob* OmdbMovie::search(MovieSearchJob::Config config)
{
    return new OmdbMovieSearchJob(m_api, std::move(config), this);
}

MovieScrapeJob* OmdbMovie::loadMovie(MovieScrapeJob::Config config)
{
    return new OmdbMovieScrapeJob(m_api, std::move(config), this);
}

QSet<MovieScraperInfo> OmdbMovie::scraperNativelySupports()
{
    return m_scraperNativelySupports;
}

void OmdbMovie::changeLanguage(mediaelch::Locale locale)
{
    // OMDb only supports English
    Q_UNUSED(locale);
}

} // namespace scraper
} // namespace mediaelch
