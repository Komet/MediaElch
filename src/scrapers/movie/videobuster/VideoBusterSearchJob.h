#pragma once

#include "scrapers/movie/MovieSearchJob.h"

namespace mediaelch {
namespace scraper {

class VideoBusterApi;

class VideoBusterSearchJob : public MovieSearchJob
{
    Q_OBJECT

public:
    explicit VideoBusterSearchJob(VideoBusterApi& api, MovieSearchJob::Config _config, QObject* parent = nullptr);
    ~VideoBusterSearchJob() override = default;

    void execute() override;

private:
    void parseSearch(const QString& html);

private:
    VideoBusterApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
