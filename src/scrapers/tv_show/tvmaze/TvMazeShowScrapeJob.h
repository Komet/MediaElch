#pragma once

#include "scrapers/tv_show/ShowScrapeJob.h"
#include "scrapers/tv_show/tvmaze/TvMazeShowParser.h"

namespace mediaelch {
namespace scraper {

class TvMazeApi;

class TvMazeShowScrapeJob : public ShowScrapeJob
{
    Q_OBJECT

public:
    TvMazeShowScrapeJob(TvMazeApi& api, Config _config, QObject* parent = nullptr);
    ~TvMazeShowScrapeJob() override = default;
    void execute() override;

private:
    TvMazeApi& m_api;
    TvMazeShowParser m_parser;
};

} // namespace scraper
} // namespace mediaelch
