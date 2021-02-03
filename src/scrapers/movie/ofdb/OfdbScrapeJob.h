#pragma once

#include "scrapers/movie/MovieScrapeJob.h"

namespace mediaelch {
namespace scraper {

class OfdbApi;

class OfdbScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    OfdbScrapeJob(OfdbApi& api, Config _config, QObject* parent = nullptr);
    ~OfdbScrapeJob() override = default;
    void execute() override;

private:
    void parseAndAssignInfos(const QString& data);

private:
    OfdbApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
