#pragma once

#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/tmdb/TmdbApi.h"

#include <QPair>
#include <QVector>

namespace mediaelch {
namespace scraper {

class TmdbMovieSearchJob : public MovieSearchJob
{
    Q_OBJECT

public:
    TmdbMovieSearchJob(TmdbApi& api, MovieSearchJob::Config config, QObject* parent = nullptr) :
        MovieSearchJob(std::move(config), parent), m_api{api}
    {
    }

    ~TmdbMovieSearchJob() override = default;

    void execute() override;

private slots:
    void handleSearchResponse();

private:
    QString normalizedQuery();

    /// \brief Parses the JSON search results
    /// \param json JSON string
    /// \param nextPage This will hold the next page to get, -1 if there are no more pages
    /// \return List of search results
    QVector<MovieSearchJob::Result> parseSearch(QString json, int* nextPage);

private:
    TmdbApi& m_api;

    QString m_searchTitle;
    QString m_searchYear;
    QVector<MovieSearchJob::Result> m_results;
    int m_currentTmdbSearchPage = 1;

    /// \brief Maximum number of TMDb search pages to load. One page contains ~20 entries.
    const int m_maxPages = 3;
};

} // namespace scraper
} // namespace mediaelch
