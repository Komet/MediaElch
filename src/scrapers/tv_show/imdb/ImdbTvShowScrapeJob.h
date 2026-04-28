#pragma once

#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/tv_show/ShowScrapeJob.h"

namespace mediaelch {
namespace scraper {

class ImdbTvShowScrapeJob : public ShowScrapeJob
{
    Q_OBJECT

public:
    ImdbTvShowScrapeJob(ImdbApi& api, Config _config, QObject* parent = nullptr);
    ~ImdbTvShowScrapeJob() override = default;
    void doStart() override;

private:
    void parseAndAssignInfos(const QString& json);

private:
    ImdbApi& m_api;
    ImdbId m_id;
};

} // namespace scraper
} // namespace mediaelch
