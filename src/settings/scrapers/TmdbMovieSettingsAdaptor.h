#pragma once

#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "settings/scrapers/ScraperSettingsAdaptor.h"

#include <QObject>

namespace mediaelch {
namespace settings {

class TmdbMovieSettingsAdaptor : public ScraperSettingsAdaptor
{
    Q_OBJECT
public:
    explicit TmdbMovieSettingsAdaptor(scraper::TmdbMovie& scraper, QObject* parent = nullptr) :
        ScraperSettingsAdaptor(parent),
        m_scraper{scraper}
    {
    }
    ~TmdbMovieSettingsAdaptor() override = default;

public slots:
    void loadSettings(QSettings& settingsStore) override;
    void saveSettings(QSettings& settingsStore) override;

private:
    scraper::TmdbMovie& m_scraper;
};

} // namespace settings
}
