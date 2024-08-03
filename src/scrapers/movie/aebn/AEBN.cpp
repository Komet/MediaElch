#include "AEBN.h"

#include "log/Log.h"
#include "scrapers/movie/aebn/AebnConfiguration.h"
#include "scrapers/movie/aebn/AebnScrapeJob.h"
#include "scrapers/movie/aebn/AebnSearchJob.h"
#include "ui/main/MainWindow.h"

#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

AEBN::AEBN(AebnConfiguration& settings, QObject* parent) : MovieScraper(parent), m_settings{settings}
{
    m_meta.identifier = ID;
    m_meta.name = "AEBN";
    m_meta.description = tr("AEBN is a video database for adult content.");
    m_meta.website = "https://aebn.net";
    m_meta.termsOfService = "https://straight.aebn.com/straight/policy/terms";
    m_meta.privacyPolicy = "https://straight.aebn.com/straight/policy/privacy";
    m_meta.help = "https://straight.aebn.com/straight/customer-service";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Director,
        MovieScraperInfo::Set,
        MovieScraperInfo::Tags};
    m_meta.supportedLanguages = AebnConfiguration::supportedLanguages();
    m_meta.defaultLocale = AebnConfiguration::defaultLocale();
    m_meta.isAdult = true;
}

AEBN::~AEBN()
{
}

const MovieScraper::ScraperMeta& AEBN::meta() const
{
    return m_meta;
}

void AEBN::initialize()
{
    // no-op
    // AEBN requires no initialization.
}

bool AEBN::isInitialized() const
{
    // AEBN requires no initialization.
    return true;
}

MovieSearchJob* AEBN::search(MovieSearchJob::Config config)
{
    return new AebnSearchJob(m_api, std::move(config), m_settings.genreId(), this);
}

MovieScrapeJob* AEBN::loadMovie(MovieScrapeJob::Config config)
{
    if (config.locale == Locale::NoLocale) {
        config.locale = meta().defaultLocale;
    }
    return new AebnScrapeJob(m_api, std::move(config), m_settings.genreId(), this);
}

void AEBN::changeLanguage(mediaelch::Locale locale)
{
    m_settings.setLanguage(locale);
}

QSet<MovieScraperInfo> AEBN::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

} // namespace scraper
} // namespace mediaelch
