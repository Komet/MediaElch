#pragma once

#include "scrapers/tv_show/TvScraper.h"

namespace mediaelch {
namespace scraper {

class EmptyTv : public TvScraper
{
    Q_OBJECT

public:
    static QString ID;

public:
    explicit EmptyTv(QObject* parent = nullptr);
    ~EmptyTv() override = default;

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ShowSearchJob* search(ShowSearchJob::Config config) override;
    ShowScrapeJob* loadShow(ShowScrapeJob::Config config) override;
    SeasonScrapeJob* loadSeasons(SeasonScrapeJob::Config config) override;
    EpisodeScrapeJob* loadEpisode(EpisodeScrapeJob::Config config) override;

private:
    ScraperMeta m_meta;
};


/// \brief   Empty show search job for testing purposes.
/// \details This class can be used as a placeholder for new scrapers that did
///          not yet implement certain features.
class EmptyShowSearchJob : public ShowSearchJob
{
    Q_OBJECT

public:
    explicit EmptyShowSearchJob(ShowSearchJob::Config _config, QObject* parent = nullptr);
    ~EmptyShowSearchJob() override = default;
    void doStart() override;
};

/// \brief   Empty show scrape job for testing purposes.
/// \details This class can be used as a placeholder for new scrapers that did
///          not yet implement certain features.
class EmptyShowScrapeJob : public ShowScrapeJob
{
    Q_OBJECT

public:
    EmptyShowScrapeJob(Config _config, QObject* parent = nullptr);
    ~EmptyShowScrapeJob() override = default;
    void doStart() override;
};

/// \brief   Empty season scrape job for testing purposes.
/// \details This class can be used as a placeholder for new scrapers that did
///          not yet implement certain features.
class EmptySeasonScrapeJob : public SeasonScrapeJob
{
    Q_OBJECT

public:
    EmptySeasonScrapeJob(Config _config, QObject* parent = nullptr);
    ~EmptySeasonScrapeJob() = default;
    void doStart() override;
};

/// \brief   Empty episode scrape job for testing purposes.
/// \details This class can be used as a placeholder for new scrapers that did
///          not yet implement certain features.
class EmptyEpisodeScrapeJob : public EpisodeScrapeJob
{
    Q_OBJECT

public:
    EmptyEpisodeScrapeJob(Config _config, QObject* parent = nullptr);
    ~EmptyEpisodeScrapeJob() = default;
    void doStart() override;
};

} // namespace scraper
} // namespace mediaelch
