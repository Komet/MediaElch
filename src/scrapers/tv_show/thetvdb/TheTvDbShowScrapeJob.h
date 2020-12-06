#pragma once

#include "scrapers/tv_show/ShowScrapeJob.h"
#include "scrapers/tv_show/thetvdb/TheTvDbShowParser.h"

#include <QMutex>
#include <QSet>

namespace mediaelch {
namespace scraper {

class TheTvDbApi;

class TheTvDbShowScrapeJob : public ShowScrapeJob
{
    Q_OBJECT

public:
    TheTvDbShowScrapeJob(TheTvDbApi& api, Config _config, QObject* parent = nullptr);
    ~TheTvDbShowScrapeJob() override = default;
    void execute() override;

private:
    void loadTvShow();
    void loadActors();
    void loadImages(ShowScraperInfo imageType);

    bool shouldLoad(ShowScraperInfo info);
    void setIsLoaded(ShowScraperInfo info);
    void checkIfDone();

private:
    TheTvDbApi& m_api;
    TheTvDbShowParser m_parser;
    QSet<ShowScraperInfo> m_notLoaded;
    QSet<ShowScraperInfo> m_supports;
    TvDbId m_id;
    /// \brief Lock for the list of loaded details.
    QMutex m_networkMutex;
};

} // namespace scraper
} // namespace mediaelch
