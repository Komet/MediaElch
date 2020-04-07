#include "settings/scrapers/TmdbMovieSettingsAdaptor.h"

namespace mediaelch
{

namespace settings
{

TmdbMovieSettingsAdaptor::loadSettings(QSettings& settingsStore) {
    auto config = m_scraper->config();
    // TODO: modify settings
    m_scraper->setConfig(config);
}

TmdbMovieSettingsAdaptor::saveSettings(QSettings& settingsStore) {
    auto config = m_scraper->config();
    // TODO: save
}

} // namespace settigns
} // namespace mediaelch
