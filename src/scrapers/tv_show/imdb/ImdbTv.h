#pragma once

#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/imdb/ImdbTvConfiguration.h"
#include "utils/Meta.h"

namespace mediaelch {
namespace scraper {

class ImdbTv : public TvScraper
{
    Q_OBJECT

public:
    static QString ID;

public:
    explicit ImdbTv(ImdbTvConfiguration& settings, QObject* parent = nullptr);
    ~ImdbTv() override = default;

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ShowSearchJob* search(ShowSearchJob::Config config) override;
    ShowScrapeJob* loadShow(ShowScrapeJob::Config config) override;
    SeasonScrapeJob* loadSeasons(SeasonScrapeJob::Config config) override;
    EpisodeScrapeJob* loadEpisode(EpisodeScrapeJob::Config config) override;

private:
    ImdbTvConfiguration& m_settings;
    ScraperMeta m_meta;
    ImdbApi m_api;
};

} // namespace scraper
} // namespace mediaelch
