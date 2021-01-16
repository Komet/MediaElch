#pragma once

#include "scrapers/movie/MovieSearchJob.h"

namespace mediaelch {
namespace scraper {

class AdultDvdEmpireApi;

class AdultDvdEmpireSearchJob : public MovieSearchJob
{
    Q_OBJECT

public:
    explicit AdultDvdEmpireSearchJob(AdultDvdEmpireApi& api, MovieSearchJob::Config _config, QObject* parent = nullptr);
    ~AdultDvdEmpireSearchJob() override = default;

    void execute() override;

private:
    void parseSearch(const QString& html);

private:
    AdultDvdEmpireApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
