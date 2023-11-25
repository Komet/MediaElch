#pragma once

#include "scrapers/movie/MovieSearchJob.h"

namespace mediaelch {
namespace scraper {

class ImdbApi;

class ImdbMovieSearchJob : public MovieSearchJob
{
    Q_OBJECT

public:
    explicit ImdbMovieSearchJob(ImdbApi& api, MovieSearchJob::Config _config, QObject* parent = nullptr);
    ~ImdbMovieSearchJob() override = default;

    void doStart() override;

private:
    void searchViaImdbId();
    void searchViaQuery();

    void parseSearch(const QString& html);
    void parseIdFromMovieReferencePage(const QString& html);

private:
    ImdbApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
