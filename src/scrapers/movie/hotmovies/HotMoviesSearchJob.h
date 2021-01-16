#pragma once

#include "scrapers/movie/MovieSearchJob.h"

namespace mediaelch {
namespace scraper {

class HotMoviesApi;

class HotMoviesSearchJob : public MovieSearchJob
{
    Q_OBJECT

public:
    explicit HotMoviesSearchJob(HotMoviesApi& api, MovieSearchJob::Config _config, QObject* parent = nullptr);
    ~HotMoviesSearchJob() override = default;

    void execute() override;

private:
    void parseSearch(const QString& html);

private:
    HotMoviesApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
