#pragma once

#include "scrapers/movie/MovieScrapeJob.h"

#include "data/Actor.h"

namespace mediaelch {
namespace scraper {

class ImdbApi;

class ImdbMovieScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    ImdbMovieScrapeJob(ImdbApi& api, Config _config, bool loadAllTags, QObject* parent = nullptr);
    ~ImdbMovieScrapeJob() override = default;
    void doStart() override;

public:
    void loadTags();

    void parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfo> infos) const;
    void parseAndAssignPoster(const QString& html);
    void parseAndStoreActors(const QString& html);
    void parseAndAssignTags(const QString& html);
    QString sanitizeAmazonMediaUrl(QString url);

    void decreaseDownloadCount();

private: // config
    ImdbApi& m_api;
    ImdbId m_imdbId;
    bool m_loadAllTags = false;

private: // initialized during scraping
    int m_itemsLeftToDownloads = 0;
    QVector<QPair<Actor, QUrl>> m_actorUrls;
};

} // namespace scraper
} // namespace mediaelch
