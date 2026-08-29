#pragma once

#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/tv_show/TvScraper.h"

namespace mediaelch {
namespace scraper {

class ImdbTvShowSearchJob : public ShowSearchJob
{
    Q_OBJECT

public:
    explicit ImdbTvShowSearchJob(ImdbApi& api, ShowSearchJob::Config _config, QObject* parent = nullptr);
    ~ImdbTvShowSearchJob() override = default;
    void doStart() override;

private:
    void searchViaImdbId();
    void searchViaQuery();

    void parseSuggestResults(const QString& json);
    void parseGraphQLResult(const QString& json);

private:
    ImdbApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
