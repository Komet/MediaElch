#pragma once

#include "scrapers/movie/MovieScrapeJob.h"

namespace mediaelch {
namespace scraper {

class HotMoviesApi;

class HotMoviesScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    HotMoviesScrapeJob(HotMoviesApi& api, Config _config, QObject* parent = nullptr);
    ~HotMoviesScrapeJob() override = default;
    void execute() override;

private:
    void parseAndAssignInfos(const QString& html);

private:
    HotMoviesApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
