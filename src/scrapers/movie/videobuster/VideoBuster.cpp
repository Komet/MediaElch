#include "scrapers/movie/videobuster/VideoBuster.h"

#include "scrapers/movie/videobuster/VideoBusterScrapeJob.h"
#include "scrapers/movie/videobuster/VideoBusterSearchJob.h"
#include "settings/Settings.h"

namespace mediaelch {
namespace scraper {

VideoBuster::VideoBuster(QObject* parent) : MovieScraper(parent)
{
    m_meta.identifier = ID;
    m_meta.name = "VideoBuster";
    m_meta.description = tr("VideoBuster is a German movie database.");
    m_meta.website = "https://www.videobuster.de";
    m_meta.termsOfService = "https://www.videobuster.de/agb";
    m_meta.privacyPolicy = "https://www.videobuster.de/datenschutz";
    m_meta.help = "https://www.videobuster.de/helpcenter/";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Released,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Certification,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Tagline,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Backdrop,
        MovieScraperInfo::Tags,
        MovieScraperInfo::Director};
    m_meta.supportedLanguages = {"de"};
    m_meta.defaultLocale = "de";
    m_meta.isAdult = false;
}

const MovieScraper::ScraperMeta& VideoBuster::meta() const
{
    return m_meta;
}

void VideoBuster::initialize()
{
    // VideoBuster requires no initialization.
}

bool VideoBuster::isInitialized() const
{
    // VideoBuster requires no initialization.
    return true;
}

MovieSearchJob* VideoBuster::search(MovieSearchJob::Config config)
{
    return new VideoBusterSearchJob(m_api, std::move(config), this);
}

MovieScrapeJob* VideoBuster::loadMovie(MovieScrapeJob::Config config)
{
    return new VideoBusterScrapeJob(m_api, std::move(config), this);
}

QSet<MovieScraperInfo> VideoBuster::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void VideoBuster::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

/**
 * \brief Returns if the scraper has settings
 * \return Scraper has settings
 */
bool VideoBuster::hasSettings() const
{
    return false;
}

void VideoBuster::loadSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

void VideoBuster::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QWidget* VideoBuster::settingsWidget()
{
    return nullptr;
}

} // namespace scraper
} // namespace mediaelch
