#include "scrapers/movie/tmdb/TmdbMovie.h"

#include "log/Log.h"
#include "scrapers/movie/tmdb/TmdbMovieConfiguration.h"
#include "scrapers/movie/tmdb/TmdbMovieScrapeJob.h"
#include "scrapers/movie/tmdb/TmdbMovieSearchJob.h"
#include "ui/main/MainWindow.h"

#include <QDebug>

namespace mediaelch {
namespace scraper {

TmdbMovie::TmdbMovie(TmdbMovieConfiguration& settings, QObject* parent) :
    MovieScraper(parent),
    m_settings{settings},
    m_scraperNativelySupports{MovieScraperInfo::Title,
        MovieScraperInfo::Tagline,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Certification,
        MovieScraperInfo::Trailer,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Backdrop,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Director,
        MovieScraperInfo::Writer,
        MovieScraperInfo::Set}
{
    m_meta.identifier = ID;
    m_meta.name = "The Movie DB";
    m_meta.description = tr("The Movie Database (TMDB) is a community built movie and TV database. "
                            "Every piece of data has been added by our amazing community dating back to 2008. "
                            "TMDB's strong international focus and breadth of data is largely unmatched and "
                            "something we're incredibly proud of. Put simply, we live and breathe community "
                            "and that's precisely what makes us different.");
    m_meta.website = "https://www.themoviedb.org/tv";
    m_meta.termsOfService = "https://www.themoviedb.org/terms-of-use";
    m_meta.privacyPolicy = "https://www.themoviedb.org/privacy-policy";
    m_meta.help = "https://www.themoviedb.org/talk";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Tagline,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Certification,
        MovieScraperInfo::Trailer,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Backdrop,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Director,
        MovieScraperInfo::Writer,
        MovieScraperInfo::Logo,
        MovieScraperInfo::Banner,
        MovieScraperInfo::Thumb,
        MovieScraperInfo::CdArt,
        MovieScraperInfo::ClearArt,
        MovieScraperInfo::Set};
    m_meta.supportedLanguages = TmdbMovieConfiguration::supportedLanguages();
    m_meta.defaultLocale = TmdbMovieConfiguration::defaultLocale();
    m_meta.isAdult = false;

    // TODO: Should not be called by the constructor
    initialize();
}

TmdbMovie::~TmdbMovie()
{
}

const MovieScraper::ScraperMeta& TmdbMovie::meta() const
{
    return m_meta;
}

void TmdbMovie::initialize()
{
    m_api.initialize();
}

bool TmdbMovie::isInitialized() const
{
    return m_api.isInitialized();
}

MovieSearchJob* TmdbMovie::search(MovieSearchJob::Config config)
{
    return new TmdbMovieSearchJob(m_api, std::move(config), this);
}

MovieScrapeJob* TmdbMovie::loadMovie(MovieScrapeJob::Config config)
{
    if (config.locale == Locale::NoLocale) {
        config.locale = meta().defaultLocale;
    }
    return new TmdbMovieScrapeJob(m_api, std::move(config), this);
}

QSet<MovieScraperInfo> TmdbMovie::scraperNativelySupports()
{
    return m_scraperNativelySupports;
}

void TmdbMovie::changeLanguage(mediaelch::Locale locale)
{
    if (m_meta.supportedLanguages.contains(locale)) {
        m_meta.defaultLocale = locale;
    } else {
        qCInfo(generic) << "[TMDB] Cannot change language because it is not supported:" << locale;
    }
}

} // namespace scraper
} // namespace mediaelch
