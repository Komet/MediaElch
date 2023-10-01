#include "CustomMovieScraper.h"

#include "globals/Manager.h"
#include "globals/ScraperManager.h"
#include "log/Log.h"
#include "scrapers/movie/custom/CustomMovieScrapeJob.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "settings/Settings.h"
#include "utils/Containers.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace mediaelch {
namespace scraper {

CustomMovieScraper::CustomMovieScraper(QObject* parent) : MovieScraper(parent)
{
    m_meta.identifier = ID;
    m_meta.name = tr("Custom Movie Scraper");
    m_meta.description = tr("This scraper uses a set of other scrapers to load data from different sources. Refer to "
                            "each scraper for more details.");
    m_meta.website = "https://mediaelch.github.io/mediaelch-doc/";
    m_meta.termsOfService = "";
    m_meta.privacyPolicy = "";
    m_meta.help = "https://mediaelch.github.io/mediaelch-doc/movie/index.html";
    m_meta.supportedDetails = allMovieScraperInfos();
    // Note: This scraper is handled in a special way.
    // Default languages of all sub-scrapers are used, or rather:
    // The scraper's language setting is used.
    m_meta.supportedLanguages = {mediaelch::Locale::NoLocale};
    m_meta.defaultLocale = mediaelch::Locale::NoLocale;
    m_meta.isAdult = false;
}

mediaelch::network::NetworkManager* CustomMovieScraper::network()
{
    return &m_network;
}

CustomMovieScraper* CustomMovieScraper::instance(QObject* parent)
{
    static auto* s_instance = new CustomMovieScraper(parent);
    return s_instance;
}

const MovieScraper::ScraperMeta& CustomMovieScraper::meta() const
{
    return m_meta;
}

void CustomMovieScraper::initialize()
{
    // TODO
}

bool CustomMovieScraper::isInitialized() const
{
    // TODO
    return true;
}

MovieSearchJob* CustomMovieScraper::search(MovieSearchJob::Config config)
{
    MovieScraper* scraper = titleScraper();
    if (scraper == nullptr) {
        // always use TMDB just in case
        scraper = Manager::instance()->scrapers().movieScraper(TmdbMovie::ID);
    }
    MediaElch_Assert(scraper != nullptr);
    return scraper->search(std::move(config));
}

MovieScrapeJob* CustomMovieScraper::loadMovie(MovieScrapeJob::Config config)
{
    QHash<MovieScraper*, MovieScrapeJob::Config> scraperMap;

    // TODO: Could we invert allDetails to QMap<Scraper*, QSet<Details>>?
    //       Would that make the loop below easier/faster?  We would need an intersection of both sets.
    const QMap<MovieScraperInfo, MovieScraper*> allDetails = detailsToScrapers();

    for (MovieScraperInfo detail : asConst(config.details)) {
        if (!allDetails.contains(detail)) {
            // If there is no scraper for this detail, just ignore it.
            continue;
        }

        MovieScraper* detailScraper = allDetails[detail];
        if (detailScraper == nullptr || !m_scraperMovieIds.contains(detailScraper)
            || m_scraperMovieIds[detailScraper].str().isEmpty()) {
            // If we don't have an identifier for the scraper, ignore it.
            // Also, empty identifiers are used as placeholders for failed searches.
            continue;
        }

        if (!scraperMap.contains(detailScraper)) {
            MovieScrapeJob::Config scraperConfig;
            scraperConfig.identifier = m_scraperMovieIds[detailScraper];
            scraperConfig.locale = detailScraper->meta().defaultLocale;
            scraperConfig.details = {};
            scraperMap.insert(detailScraper, scraperConfig);
        }
        scraperMap[detailScraper].details << detail;
    }

    CustomMovieScrapeJob::CustomScraperConfig scraperConfig;
    scraperConfig.scraperMap = scraperMap;

    return new CustomMovieScrapeJob(scraperConfig, this);
}

void CustomMovieScraper::setScraperMovieIds(QHash<MovieScraper*, MovieIdentifier> ids)
{
    m_scraperMovieIds = std::move(ids);
}

QSet<MovieScraperInfo> CustomMovieScraper::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void CustomMovieScraper::changeLanguage(mediaelch::Locale locale)
{
    Q_UNUSED(locale)
    // no-op
}

MovieScraper* CustomMovieScraper::titleScraper()
{
    return scraperForInfo(MovieScraperInfo::Title);
}

QMap<MovieScraperInfo, MovieScraper*> CustomMovieScraper::detailsToScrapers()
{
    QMap<MovieScraperInfo, MovieScraper*> detailMap;

    const QMap<MovieScraperInfo, QString>& details = Settings::instance()->customMovieScraper();
    QMap<MovieScraperInfo, QString>::const_iterator i = details.constBegin();
    for (; i != details.constEnd(); ++i) {
        // User may have set detail to "do not use".
        if (!i.value().isEmpty()) {
            // In case scraper is a nullptr, we ignore it at this point.
            // The custom movie scrape job will simply not scrape anything for this detail.
            MovieScraper* scraper = Manager::instance()->scrapers().movieScraper(i.value());
            if (scraper != nullptr) {
                detailMap.insert(i.key(), scraper);
            }
        }
    }

    return detailMap;
}

void CustomMovieScraper::updateSupportedDetails()
{
    // Title must always be available!
    m_meta.supportedDetails = {MovieScraperInfo::Title};

    const QMap<MovieScraperInfo, QString>& details = Settings::instance()->customMovieScraper();
    QMap<MovieScraperInfo, QString>::const_iterator i = details.constBegin();
    for (; i != details.constEnd(); ++i) {
        if (!i.value().isEmpty()) {
            m_meta.supportedDetails << i.key();
        }
    }
}

QVector<MovieScraper*> CustomMovieScraper::scrapersNeedSearch(const QSet<MovieScraperInfo>& infos)
{
    QSet<MovieScraper*> scrapeNeeded;
    const auto details = detailsToScrapers();

    for (const MovieScraperInfo detail : infos) {
        if (details.contains(detail)) {
            scrapeNeeded << details[detail];
        } else if (detail != MovieScraperInfo::Thumb && detail != MovieScraperInfo::Banner
                   && detail != MovieScraperInfo::Logo && detail != MovieScraperInfo::ClearArt
                   && detail != MovieScraperInfo::CdArt) {
            // Some details such as images aren't loaded from movie scrapers directly, hence
            // the list above.  But if it's missing for some other detail, we should log it.
            // The UI shouldn't allow scraping details that don't have a scraper set in the UI.
            qCDebug(generic) << "[CustomerMovieScraper] Missing scraper for detail:"
                             << movieScraperDetailToString(detail);
        }
    }

    return mediaelch::setToVector(scrapeNeeded);
}

MovieScraper* CustomMovieScraper::scraperForInfo(MovieScraperInfo info)
{
    QString identifier = Settings::instance()->customMovieScraper().value(info, "");
    MovieScraper* scraper = Manager::instance()->scrapers().movieScraper(identifier);
    return scraper;
}

bool CustomMovieScraper::hasSettings() const
{
    return false;
}

void CustomMovieScraper::loadSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
    updateSupportedDetails();
}

void CustomMovieScraper::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QWidget* CustomMovieScraper::settingsWidget()
{
    return nullptr;
}

} // namespace scraper
} // namespace mediaelch
