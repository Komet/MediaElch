#pragma once

#include "scrapers/movie/MovieScrapeJob.h"

namespace mediaelch {
namespace scraper {

class VideoBusterApi;

class VideoBusterScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    VideoBusterScrapeJob(VideoBusterApi& api, Config _config, QObject* parent = nullptr);
    ~VideoBusterScrapeJob() override = default;
    void execute() override;

private:
    void parseAndAssignInfos(const QString& html);

private:
    VideoBusterApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
