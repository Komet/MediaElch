#include "scrapers/movie/imdb/ImdbMovie.h"

#include "scrapers/imdb/ImdbReferencePage.h"
#include "scrapers/movie/imdb/ImdbMovieConfiguration.h"
#include "scrapers/movie/imdb/ImdbMovieScrapeJob.h"
#include "scrapers/movie/imdb/ImdbMovieSearchJob.h"
#include "ui/main/MainWindow.h"


namespace mediaelch {
namespace scraper {

ImdbMovie::ImdbMovie(ImdbMovieConfiguration& settings, QObject* parent) : MovieScraper(parent), m_settings{settings}
{
    m_meta.identifier = ID;
    m_meta.name = "IMDb";
    m_meta.description = tr("IMDb is the world's most popular and authoritative source for movie, TV "
                            "and celebrity content, designed to help fans explore the world of movies "
                            "and shows and decide what to watch.");
    m_meta.website = "https://www.imdb.com/whats-on-tv/";
    m_meta.termsOfService = "https://www.imdb.com/conditions";
    m_meta.privacyPolicy = "https://www.imdb.com/privacy";
    m_meta.help = "https://help.imdb.com";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Director,
        MovieScraperInfo::Writer,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Tags,
        MovieScraperInfo::Released,
        MovieScraperInfo::Certification,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Tagline,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Poster};
    m_meta.supportedLanguages = ImdbMovieConfiguration::supportedLanguages();
    m_meta.defaultLocale = ImdbMovieConfiguration::defaultLocale();
    m_meta.isAdult = false;
}

ImdbMovie::~ImdbMovie()
{
}

const MovieScraper::ScraperMeta& ImdbMovie::meta() const
{
    return m_meta;
}

void ImdbMovie::initialize()
{
    // no-op
    // IMDb requires no initialization.
}

bool ImdbMovie::isInitialized() const
{
    // IMDb requires no initialization.
    return true;
}

MovieSearchJob* ImdbMovie::search(MovieSearchJob::Config config)
{
    return new ImdbMovieSearchJob(m_api, std::move(config), this);
}

MovieScrapeJob* ImdbMovie::loadMovie(MovieScrapeJob::Config config)
{
    if (config.locale == Locale::NoLocale) {
        config.locale = meta().defaultLocale;
    }
    return new ImdbMovieScrapeJob(m_api, std::move(config), m_settings.shouldLoadAllTags(), this);
}

QSet<MovieScraperInfo> ImdbMovie::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void ImdbMovie::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

} // namespace scraper
} // namespace mediaelch
