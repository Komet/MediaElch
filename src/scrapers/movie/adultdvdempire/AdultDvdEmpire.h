#pragma once

#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/adultdvdempire/AdultDvdEmpireApi.h"

#include <QObject>
#include <QWidget>

namespace mediaelch {
namespace scraper {

class AdultDvdEmpire : public MovieScraper
{
    Q_OBJECT
public:
    explicit AdultDvdEmpire(QObject* parent = nullptr);
    static constexpr const char* ID = "adult-dvd-empire";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD MovieSearchJob* search(MovieSearchJob::Config config) override;
    ELCH_NODISCARD MovieScrapeJob* loadMovie(MovieScrapeJob::Config config) override;

public:
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;

    QSet<MovieScraperInfo> scraperNativelySupports() override;

    void changeLanguage(mediaelch::Locale locale) override;
    QWidget* settingsWidget() override;

private:
    ScraperMeta m_meta;
    AdultDvdEmpireApi m_api;
};

} // namespace scraper
} // namespace mediaelch
