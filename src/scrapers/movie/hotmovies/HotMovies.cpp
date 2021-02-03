#include "scrapers/movie/hotmovies/HotMovies.h"

#include "scrapers/movie/hotmovies/HotMoviesScrapeJob.h"
#include "scrapers/movie/hotmovies/HotMoviesSearchJob.h"

#include <QDebug>
#include <QGridLayout>

namespace mediaelch {
namespace scraper {

HotMovies::HotMovies(QObject* parent) : MovieScraper(parent)
{
    m_meta.identifier = ID;
    m_meta.name = "HotMovies";
    m_meta.description = "HotMovies is a video database for adult content.";
    m_meta.website = "https://www.hotmovies.com";
    m_meta.termsOfService = "https://www.hotmovies.com";
    m_meta.privacyPolicy = "https://www.hotmovies.com";
    m_meta.help = "https://www.hotmovies.com";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Backdrop,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Director,
        MovieScraperInfo::Set};
    m_meta.supportedLanguages = {"en"};
    m_meta.defaultLocale = "en";
    m_meta.isAdult = true;
}

const MovieScraper::ScraperMeta& HotMovies::meta() const
{
    return m_meta;
}

void HotMovies::initialize()
{
    // no-op
    // HotMovies requires no initialization.
}

bool HotMovies::isInitialized() const
{
    // HotMovies requires no initialization.
    return true;
}

MovieSearchJob* HotMovies::search(MovieSearchJob::Config config)
{
    return new HotMoviesSearchJob(m_api, std::move(config), this);
}

MovieScrapeJob* HotMovies::loadMovie(MovieScrapeJob::Config config)
{
    return new HotMoviesScrapeJob(m_api, std::move(config), this);
}

QSet<MovieScraperInfo> HotMovies::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void HotMovies::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

bool HotMovies::hasSettings() const
{
    return false;
}

void HotMovies::loadSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

void HotMovies::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QWidget* HotMovies::settingsWidget()
{
    return nullptr;
}

} // namespace scraper
} // namespace mediaelch
