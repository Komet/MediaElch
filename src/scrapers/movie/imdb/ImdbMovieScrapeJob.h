#pragma once

#include "scrapers/movie/MovieScrapeJob.h"

#include "data/Actor.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QString>
#include <QVector>

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

private:
    void loadTags();

    QJsonDocument extractJsonFromHtml(const QString& html);
    QJsonValue followJsonPath(const QJsonDocument& json, const QVector<QString>& paths);
    QJsonValue followJsonPath(const QJsonObject& json, const QVector<QString>& paths);

    void parseAndAssignInfos(const QJsonDocument& json);
    void parseAndStoreActors(const QJsonDocument& json);
    void parseAndAssignDirectors(const QJsonDocument& json);
    void parseAndAssignWriters(const QJsonDocument& json);

    void parseAndAssignTags(const QString& html);

    static QString sanitizeAmazonMediaUrl(QString url);

    void decreaseDownloadCount();

private: // config
    ImdbApi& m_api;
    ImdbId m_imdbId;
    bool m_loadAllTags = false;

private: // initialized during scraping
    int m_itemsLeftToDownloads = 0;
};

} // namespace scraper
} // namespace mediaelch
