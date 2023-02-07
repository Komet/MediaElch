#pragma once

#include "globals/Helper.h"
#include "scrapers/movie/MovieScrapeJob.h"

namespace mediaelch {
namespace scraper {

class TmdbApi;

class TmdbMovieScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    TmdbMovieScrapeJob(TmdbApi& api, Config _config, QObject* parent = nullptr);
    ~TmdbMovieScrapeJob() override = default;
    void doStart() override;

public:
    void parseAndAssignInfos(const QString& json,
        Movie* movie,
        const QSet<MovieScraperInfo>& infos,
        const QString& language,
        const QString& country);
    /// \brief Load the collection (TMDb id) and store the content in the movie.
    void loadCollection(const TmdbId& collectionTmdbId);

private:
    void onDownloadDone(ScraperData data);

private:
    TmdbApi& m_api;
    QString m_baseUrl;
    QVector<ScraperData> m_loadsLeft;
};

} // namespace scraper
} // namespace mediaelch
