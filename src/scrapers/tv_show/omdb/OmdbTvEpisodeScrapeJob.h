#pragma once

#include "scrapers/tv_show/EpisodeScrapeJob.h"

namespace mediaelch {
namespace scraper {

class OmdbApi;

class OmdbTvEpisodeScrapeJob : public EpisodeScrapeJob
{
    Q_OBJECT

public:
    OmdbTvEpisodeScrapeJob(OmdbApi& api, Config _config, QObject* parent = nullptr);
    ~OmdbTvEpisodeScrapeJob() override = default;
    void doStart() override;

private:
    void parseAndAssignInfos(const QJsonDocument& json);

private:
    OmdbApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
