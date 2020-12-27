#pragma once

#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/tv_show/ShowScrapeJob.h"
#include "scrapers/tv_show/imdb/ImdbTvShowParser.h"

namespace mediaelch {
namespace scraper {

class ImdbTvShowScrapeJob : public ShowScrapeJob
{
    Q_OBJECT

public:
    ImdbTvShowScrapeJob(ImdbApi& api, Config _config, QObject* parent = nullptr);
    ~ImdbTvShowScrapeJob() override = default;
    void execute() override;

private:
    void loadTvShow();

    bool shouldLoad(ShowScraperInfo info);
    void setIsLoaded(ShowScraperInfo info);
    void checkIfDone();

private:
    ImdbApi& m_api;
    ImdbTvShowParser m_parser;
    QSet<ShowScraperInfo> m_notLoaded;
    QSet<ShowScraperInfo> m_supports;
    ImdbId m_id;
    /// \brief Lock for the list of loaded details.
    QMutex m_networkMutex;
};

} // namespace scraper
} // namespace mediaelch
