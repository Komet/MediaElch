#include "AdultDvdEmpire.h"

#include "data/Storage.h"
#include "scrapers/movie/adultdvdempire/AdultDvdEmpireScrapeJob.h"
#include "scrapers/movie/adultdvdempire/AdultDvdEmpireSearchJob.h"
#include "settings/Settings.h"

#include <QDebug>

namespace mediaelch {
namespace scraper {

AdultDvdEmpire::AdultDvdEmpire(QObject* parent) : MovieScraper(parent)
{
    m_meta.identifier = ID;
    m_meta.name = "Adult DVD Empire";
    m_meta.description = "Adult DVD Empire is a video database for adult content.";
    m_meta.website = "https://www.adultempire.com/";
    m_meta.termsOfService = "https://www.adultempire.com/";
    m_meta.privacyPolicy = "https://www.adultempire.com/";
    m_meta.help = "https://www.adultempire.com/";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Backdrop,
        MovieScraperInfo::Set,
        MovieScraperInfo::Director};
    m_meta.supportedLanguages = {"en"};
    m_meta.defaultLocale = "en";
    m_meta.isAdult = true;
}

const MovieScraper::ScraperMeta& AdultDvdEmpire::meta() const
{
    return m_meta;
}

void AdultDvdEmpire::initialize()
{
    // no-op
}

bool AdultDvdEmpire::isInitialized() const
{
    // Does not need to be initialized.
    return true;
}

MovieSearchJob* AdultDvdEmpire::search(MovieSearchJob::Config config)
{
    return new AdultDvdEmpireSearchJob(m_api, std::move(config), this);
}

MovieScrapeJob* AdultDvdEmpire::loadMovie(MovieScrapeJob::Config config)
{
    return new AdultDvdEmpireScrapeJob(m_api, std::move(config), this);
}

QSet<MovieScraperInfo> AdultDvdEmpire::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void AdultDvdEmpire::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: only one language is supported and hard-coded.
}

bool AdultDvdEmpire::hasSettings() const
{
    return false;
}

void AdultDvdEmpire::loadSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

void AdultDvdEmpire::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QWidget* AdultDvdEmpire::settingsWidget()
{
    return nullptr;
}

} // namespace scraper
} // namespace mediaelch
