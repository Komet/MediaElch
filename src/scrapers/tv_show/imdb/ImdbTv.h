#pragma once

#include "globals/Meta.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/tv_show/TvScraper.h"

namespace mediaelch {
namespace scraper {

class ImdbTv : public TvScraper
{
    Q_OBJECT

public:
    static QString ID;

public:
    explicit ImdbTv(QObject* parent = nullptr);
    ~ImdbTv() override = default;

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ShowSearchJob* search(ShowSearchJob::Config config) override;
    ShowScrapeJob* loadShow(ShowScrapeJob::Config config) override;
    SeasonScrapeJob* loadSeasons(SeasonScrapeJob::Config config) override;
    EpisodeScrapeJob* loadEpisode(EpisodeScrapeJob::Config config) override;

private:
    ScraperMeta m_meta;
    ImdbApi m_api;
};

} // namespace scraper
} // namespace mediaelch
