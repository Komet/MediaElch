#pragma once

#include "scrapers/movie/MovieScrapeJob.h"

#include <QHash>

namespace mediaelch {
namespace scraper {

class CustomMovieScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    CustomMovieScrapeJob(Config _config, QObject* parent = nullptr);
    ~CustomMovieScrapeJob() override = default;

    void execute() override;
};

} // namespace scraper
} // namespace mediaelch
