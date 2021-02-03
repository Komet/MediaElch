#pragma once

#include "globals/Helper.h"
#include "scrapers/movie/MovieScrapeJob.h"

#include <QJsonDocument>

namespace mediaelch {
namespace scraper {

class TmdbApi;

class TmdbMovieScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    TmdbMovieScrapeJob(TmdbApi& api, Config _config, QObject* parent = nullptr);
    ~TmdbMovieScrapeJob() override = default;
    void execute() override;

private:
    void parseAndAssignInfos(const QJsonDocument& json);
    /// \brief Load the collection (TMDb id) and store the content in the movie.
    void loadCollection(const TmdbId& collectionTmdbId);

private:
    void onDownloadDone(ScraperData data);

private:
    TmdbApi& m_api;
    QVector<ScraperData> m_loadsLeft;
};

} // namespace scraper
} // namespace mediaelch
