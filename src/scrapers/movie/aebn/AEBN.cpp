#include "AEBN.h"

#include "data/movie/Movie.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/movie/aebn/AebnConfiguration.h"
#include "scrapers/movie/aebn/AebnScrapeJob.h"
#include "scrapers/movie/aebn/AebnSearchJob.h"
#include "settings/Settings.h"
#include "ui/main/MainWindow.h"

#include <QGridLayout>
#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

AEBN::AEBN(AebnConfiguration& settings, QObject* parent) :
    MovieScraper(parent),
    m_settings{settings},
    m_widget{new QWidget},
    m_box{new QComboBox(m_widget)},
    m_genreBox{new QComboBox(m_widget)}
{
    m_meta.identifier = ID;
    m_meta.name = "AEBN";
    m_meta.description = tr("AEBN is a video database for adult content.");
    m_meta.website = "https://aebn.net";
    m_meta.termsOfService = "https://aebn.net";
    m_meta.privacyPolicy = "https://aebn.net";
    m_meta.help = "https://aebn.net";
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

    for (const mediaelch::Locale& lang : asConst(m_meta.supportedLanguages)) {
        m_box->addItem(lang.languageTranslated(), lang.toString());
    }

    // Genre IDs overrides URL (http://[straight|gay]...)
    m_genreBox->addItem(tr("Straight"), "101");
    m_genreBox->addItem(tr("Gay"), "102");

    auto* layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->addWidget(new QLabel(tr("Genre")), 1, 0);
    layout->addWidget(m_genreBox, 1, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);
}

AEBN::~AEBN()
{
    if (!m_widget.isNull() && m_widget->parent() == nullptr) {
        // We set MainWindow::instance() as this Widget's parent.
        // But at construction time, the instance is not setup, yet.
        // See settingsWidget()
        m_widget->deleteLater();
    }
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

bool AEBN::hasSettings() const
{
    return true;
}

void AEBN::loadSettings(ScraperSettings& settings)
{
    mediaelch::Locale language = m_settings.language();
    for (int i = 0, n = m_box->count(); i < n; ++i) {
        if (m_box->itemData(i).toString() == language) {
            m_box->setCurrentIndex(i);
        }
    }
    QString genreId = m_settings.genreId();
    for (int i = 0, n = m_genreBox->count(); i < n; ++i) {
        if (m_genreBox->itemData(i).toString() == genreId) {
            m_genreBox->setCurrentIndex(i);
        }
    }
}

void AEBN::saveSettings(ScraperSettings& settings)
{
    mediaelch::Locale language = m_box->itemData(m_box->currentIndex()).toString();
    m_settings.setLanguage(language);

    QString genreId = m_genreBox->itemData(m_genreBox->currentIndex()).toString();
    m_settings.setGenreId(genreId);
}

QWidget* AEBN::settingsWidget()
{
    if (m_widget->parent() == nullptr) {
        m_widget->setParent(MainWindow::instance());
    }
    return m_widget;
}

} // namespace scraper
} // namespace mediaelch
