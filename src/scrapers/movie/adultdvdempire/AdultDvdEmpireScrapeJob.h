#pragma once

#include "scrapers/movie/MovieScrapeJob.h"

namespace mediaelch {
namespace scraper {

class AdultDvdEmpireApi;

class AdultDvdEmpireScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    AdultDvdEmpireScrapeJob(AdultDvdEmpireApi& api,
        Config _config,
        bool storeBackCoverAsFanart,
        QObject* parent = nullptr);
    ~AdultDvdEmpireScrapeJob() override = default;

    void doStart() override;

private:
    void parseAndAssignInfos(const QString& html);
    QString replaceEntities(QString str) const;

private:
    AdultDvdEmpireApi& m_api;
    bool m_storeBackCoverAsFanart{false};
};

} // namespace scraper
} // namespace mediaelch
