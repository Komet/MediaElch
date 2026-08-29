#pragma once

#include "scrapers/movie/MovieScrapeJob.h"

#include <QString>

namespace mediaelch {
namespace scraper {

class ImdbApi;

class ImdbMovieScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    ImdbMovieScrapeJob(ImdbApi& api, Config _config, bool loadAllTags, QObject* parent = nullptr);
    ~ImdbMovieScrapeJob() override = default;
    void doStart() override;

private:
    void parseAndAssignInfos(const QString& json);

private:
    ImdbApi& m_api;
    ImdbId m_imdbId;
    bool m_loadAllTags = false;
};

} // namespace scraper
} // namespace mediaelch
