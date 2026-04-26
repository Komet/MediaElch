#pragma once

#include "scrapers/movie/MovieSearchJob.h"

#include <QJsonDocument>

namespace mediaelch {
namespace scraper {

class OmdbApi;

class OmdbMovieSearchJob : public MovieSearchJob
{
    Q_OBJECT

public:
    explicit OmdbMovieSearchJob(OmdbApi& api, MovieSearchJob::Config _config, QObject* parent = nullptr);
    ~OmdbMovieSearchJob() override = default;

    void doStart() override;

private:
    void searchViaImdbId();
    void searchViaQuery();
    void parseSearch(const QJsonDocument& json);

private:
    OmdbApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
