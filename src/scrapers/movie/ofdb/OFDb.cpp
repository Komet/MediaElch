#include "scrapers/movie/ofdb/OFDb.h"

#include "globals/Globals.h"
#include "scrapers/movie/ofdb/OfdbScrapeJob.h"
#include "scrapers/movie/ofdb/OfdbSearchJob.h"
#include "settings/Settings.h"

#include <QDomDocument>
#include <QRegularExpression>
#include <QWidget>
#include <QXmlStreamReader>

namespace mediaelch {
namespace scraper {

/// \brief OFDb scraper. Uses http://ofdbgw.metawave.ch directly because ttp://www.ofdbgw.org
/// is as of 2019-02-23 down.
OFDb::OFDb(QObject* parent) : MovieScraper(parent)
{
    m_meta.identifier = ID;
    m_meta.name = "OFDb";
    m_meta.description = tr("OFDb is a German online movie database.");
    m_meta.website = "https://ssl.ofdb.de/";
    m_meta.termsOfService = "https://ssl.ofdb.de/view.php?page=info#rechtliches";
    m_meta.privacyPolicy = "https://ssl.ofdb.de/view.php?page=info#datenschutz";
    m_meta.help = "https://www.gemeinschaftsforum.com/forum/";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Released,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Overview};
    m_meta.supportedLanguages = {"de"};
    m_meta.defaultLocale = "de";
    m_meta.isAdult = false;
}

const MovieScraper::ScraperMeta& OFDb::meta() const
{
    return m_meta;
}

void OFDb::initialize()
{
    // no-op
    // OFDb requires no initialization.
}

bool OFDb::isInitialized() const
{
    // OFDb requires no initialization.
    return true;
}

MovieSearchJob* OFDb::search(MovieSearchJob::Config config)
{
    return new OfdbSearchJob(m_api, std::move(config), this);
}

MovieScrapeJob* OFDb::loadMovie(MovieScrapeJob::Config config)
{
    return new OfdbScrapeJob(m_api, std::move(config), this);
}

bool OFDb::hasSettings() const
{
    return false;
}

void OFDb::loadSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

void OFDb::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QSet<MovieScraperInfo> OFDb::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void OFDb::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

QWidget* OFDb::settingsWidget()
{
    return nullptr;
}

} // namespace scraper
} // namespace mediaelch
