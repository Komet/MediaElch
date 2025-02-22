#pragma once

#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/adultdvdempire/AdultDvdEmpireApi.h"

#include <QObject>

namespace mediaelch {
namespace scraper {

class AdultDvdEmpireConfiguration;

class AdultDvdEmpire : public MovieScraper
{
    Q_OBJECT
public:
    explicit AdultDvdEmpire(AdultDvdEmpireConfiguration& settings, QObject* parent = nullptr);
    static const char* const ID;

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD MovieSearchJob* search(MovieSearchJob::Config config) override;
    ELCH_NODISCARD MovieScrapeJob* loadMovie(MovieScrapeJob::Config config) override;

public:
    QSet<MovieScraperInfo> scraperNativelySupports() override;

    void changeLanguage(mediaelch::Locale locale) override;

private:
    AdultDvdEmpireConfiguration& m_settings;
    ScraperMeta m_meta;
    AdultDvdEmpireApi m_api;
};

} // namespace scraper
} // namespace mediaelch
