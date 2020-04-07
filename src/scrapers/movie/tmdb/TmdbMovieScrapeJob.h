#pragma once

#include "data/TmdbId.h"
#include "globals/Globals.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/tmdb/TmdbMovieSearch.h"

#include <QJsonParseError>
#include <QMutex>
#include <QSet>
#include <QVector>

namespace mediaelch {
namespace scraper {

class TmdbMovieScraper;

class TmdbMovieScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    TmdbMovieScrapeJob(MovieScraper& scraper,
        TmdbApi& api,
        Movie& movie,
        const MovieScrapeJob::Config& config,
        QObject* parent = nullptr);

    ~TmdbMovieScrapeJob() override = default;

    void execute() override;

private slots:
    /// \brief Called when all movie details are downloaded
    void loadFinished();
    void loadCollectionFinished();
    void loadCastsFinished();
    void loadTrailersFinished();
    void loadImagesFinished();
    void loadReleasesFinished();

    void loadPartFinished(ScraperData data);

private:
    /// \brief Parses JSON data and assigns it to the search's movie object
    ///        Handles all types of data from TmdbMovie (info, releases, trailers, casts, images)
    /// \param json JSON data
    void parseAndAssignInfos(QString json);
    void loadCollection(const TmdbId& collectionTmdbId);
    /// \brief Removes the given element from the list of downloadable items.
    /// \details If all elements were loaded then a success signal is emitted.
    void removeFromloadsLeft(ScraperData data);
    void abort(QNetworkReply* networkReply);
    void abort(QJsonParseError parseError);

private:
    TmdbApi& m_api;
    QSet<ScraperData> m_loadsLeft;
    QMutex m_loadMutex;
    bool m_aborted = false;
    int m_maxDownloads = 0;
};

} // namespace scraper
} // namespace mediaelch
