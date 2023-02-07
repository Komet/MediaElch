#pragma once

#include "scrapers/movie/MovieScrapeJob.h"

namespace mediaelch {
namespace scraper {

class AdultDvdEmpireApi;

class AdultDvdEmpireScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    AdultDvdEmpireScrapeJob(AdultDvdEmpireApi& api, Config _config, QObject* parent = nullptr);
    ~AdultDvdEmpireScrapeJob() override = default;

    void doStart() override;

public:
    void parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfo> infos);
    QString replaceEntities(QString str) const;

private:
    AdultDvdEmpireApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
