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
    void doStart() override;

public:
    void parseAndAssignInfos(const QString& html, Movie* movie, const QSet<MovieScraperInfo>& infos);

private:
    QString decodeAndTrim(const QString& htmlEncodedString);

private:
    HotMoviesApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
