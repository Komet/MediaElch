#pragma once

#include "scrapers/movie/MovieScrapeJob.h"

#include "globals/Actor.h"

namespace mediaelch {
namespace scraper {

class ImdbApi;

class ImdbMovieScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    ImdbMovieScrapeJob(ImdbApi& api, Config _config, bool loadAllTags, QObject* parent = nullptr);
    ~ImdbMovieScrapeJob() override = default;
    void execute() override;

private:
    void loadPoster(const QUrl& posterViewerUrl);
    void loadTags();
    void loadActorImageUrls();

    void parseAndAssignInfos(const QString& html);
    void parseAndAssignPoster(const QString& html);
    void parseAndStoreActors(const QString& html);
    QUrl parsePosterViewerUrl(const QString& html);
    void parseAndAssignTags(const QString& html);
    QString parseActorImageUrl(const QString& html);

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
