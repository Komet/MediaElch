#include "scrapers/movie/videobuster/VideoBuster.h"

#include "globals/Helper.h"
#include "log/Log.h"
#include "scrapers/movie/videobuster/VideoBusterSearchJob.h"
#include "settings/Settings.h"

#include <QSet>
#include <QTextDocument>

namespace mediaelch {
namespace scraper {

VideoBuster::VideoBuster(QObject* parent) : MovieScraper(parent), m_scrapeJob(m_api, {}, nullptr)
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

QSet<MovieScraperInfo> VideoBuster::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void VideoBuster::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

void VideoBuster::loadData(QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
    Movie* movie,
    QSet<MovieScraperInfo> infos)
{
    if (ids.isEmpty()) {
        // TODO: Should not happen.
        return;
    }

    m_api.loadMovie(ids.constBegin().value().str(), [movie, infos, this](QString data, ScraperError error) {
        movie->clear(infos);

        if (!error.hasError()) {
            data = replaceEntities(data);
            parseAndAssignInfos(data, movie, infos);

        } else {
            // TODO
            showNetworkError(error);
        }
        movie->controller()->scraperLoadDone(this, error);
    });
}

void VideoBuster::parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfo> infos)
{
    m_scrapeJob.parseAndAssignInfos(html, movie, infos);
}

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

/**
 * \brief This function replaces entities with their unicode counterparts
 * \param msg String with entities
 * \return String without entities
 */
QString VideoBuster::replaceEntities(const QString& msg)
{
    // not nice but I don't know other methods which don't require the gui module
    QString m = msg;
    m.replace("&#039;", "'");
    return m;
}

} // namespace scraper
} // namespace mediaelch
