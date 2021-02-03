#include "ImdbMovie.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QWidget>

#include "data/Storage.h"
#include "globals/Helper.h"
#include "scrapers/movie/imdb/ImdbMovieScrapeJob.h"
#include "scrapers/movie/imdb/ImdbMovieSearchJob.h"
#include "settings/Settings.h"
#include "ui/main/MainWindow.h"

namespace mediaelch {
namespace scraper {

ImdbMovie::ImdbMovie(QObject* parent) : MovieScraper(parent)
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
    m_meta.supportedLanguages = {"en"};
    m_meta.defaultLocale = "en";
    m_meta.isAdult = false;

    m_settingsWidget = new QWidget(MainWindow::instance());
    m_loadAllTagsWidget = new QCheckBox(tr("Load all tags"), m_settingsWidget);
    auto* layout = new QGridLayout(m_settingsWidget);
    layout->addWidget(m_loadAllTagsWidget, 0, 0);
    layout->setContentsMargins(12, 0, 12, 12);
    m_settingsWidget->setLayout(layout);
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

bool ImdbMovie::hasSettings() const
{
    return true;
}

MovieSearchJob* ImdbMovie::search(MovieSearchJob::Config config)
{
    return new ImdbMovieSearchJob(m_api, std::move(config), this);
}

MovieScrapeJob* ImdbMovie::loadMovie(MovieScrapeJob::Config config)
{
    return new ImdbMovieScrapeJob(m_api, std::move(config), m_loadAllTags, this);
}

QWidget* ImdbMovie::settingsWidget()
{
    return m_settingsWidget;
}

void ImdbMovie::loadSettings(ScraperSettings& settings)
{
    m_loadAllTags = settings.valueBool("LoadAllTags", false);
    m_loadAllTagsWidget->setChecked(m_loadAllTags);
}

void ImdbMovie::saveSettings(ScraperSettings& settings)
{
    m_loadAllTags = m_loadAllTagsWidget->isChecked();
    settings.setBool("LoadAllTags", m_loadAllTags);
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
