#pragma once

#include "scrapers/movie/MovieSearchJob.h"

namespace mediaelch {
namespace scraper {

class OfdbApi;

class OfdbSearchJob : public MovieSearchJob
{
    Q_OBJECT

public:
    explicit OfdbSearchJob(OfdbApi& api, MovieSearchJob::Config _config, QObject* parent = nullptr);
    ~OfdbSearchJob() override = default;

    void execute() override;

private:
    void parseSearch(const QString& html);

private:
    OfdbApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
