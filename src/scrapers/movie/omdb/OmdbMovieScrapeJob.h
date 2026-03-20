#pragma once

#include "scrapers/movie/MovieScrapeJob.h"

#include <QJsonDocument>

namespace mediaelch {
namespace scraper {

class OmdbApi;

class OmdbMovieScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    OmdbMovieScrapeJob(OmdbApi& api, Config _config, QObject* parent = nullptr);
    ~OmdbMovieScrapeJob() override = default;
    void doStart() override;

private:
    void parseAndAssignInfos(const QJsonDocument& json);
    void parseRatings(const QJsonObject& obj);

private:
    OmdbApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
