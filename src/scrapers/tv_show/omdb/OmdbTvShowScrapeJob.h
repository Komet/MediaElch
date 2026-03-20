#pragma once

#include "scrapers/tv_show/ShowScrapeJob.h"

#include <QJsonDocument>

namespace mediaelch {
namespace scraper {

class OmdbApi;

class OmdbTvShowScrapeJob : public ShowScrapeJob
{
    Q_OBJECT

public:
    OmdbTvShowScrapeJob(OmdbApi& api, Config _config, QObject* parent = nullptr);
    ~OmdbTvShowScrapeJob() override = default;
    void doStart() override;

private:
    void parseAndAssignInfos(const QJsonDocument& json);
    void parseRatings(const QJsonObject& obj);

private:
    OmdbApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
