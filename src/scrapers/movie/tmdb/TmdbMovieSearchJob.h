#pragma once

#include "scrapers/movie/MovieSearchJob.h"

#include <QJsonObject>

namespace mediaelch {
namespace scraper {

class TmdbApi;

class TmdbMovieSearchJob : public MovieSearchJob
{
    Q_OBJECT

public:
    explicit TmdbMovieSearchJob(TmdbApi& api, MovieSearchJob::Config _config, QObject* parent = nullptr);
    ~TmdbMovieSearchJob() override = default;

    void execute() override;

private:
    /// \brief Parses the JSON search results
    /// \param nextPage This will hold the next page to get, -1 if there are no more pages
    void parseSearch(const QJsonObject& json, int* nextPage);

private:
    TmdbApi& m_api;
    int m_currentSearchPage = 0;
};

} // namespace scraper
} // namespace mediaelch
