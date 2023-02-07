#pragma once

#include "scrapers/movie/MovieScrapeJob.h"

#include <QString>
#include <QStringList>

namespace mediaelch {
namespace scraper {

class AebnApi;

class AebnScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    AebnScrapeJob(AebnApi& api, Config _config, QString genre, QObject* parent = nullptr);
    ~AebnScrapeJob() override = default;
    void doStart() override;

public:
    void downloadActors(QStringList actorIds);
    void parseAndAssignInfos(const QString& html, QStringList& actorIds, Movie* movie, QSet<MovieScraperInfo> infos);
    void parseAndAssignActor(const QString& html, QString id);

private:
    AebnApi& m_api;
    QString m_genre;
};

} // namespace scraper
} // namespace mediaelch
