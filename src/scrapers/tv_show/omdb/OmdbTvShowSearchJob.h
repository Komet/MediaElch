#pragma once

#include "scrapers/tv_show/ShowSearchJob.h"

#include <QJsonDocument>

namespace mediaelch {
namespace scraper {

class OmdbApi;

class OmdbTvShowSearchJob : public ShowSearchJob
{
    Q_OBJECT

public:
    explicit OmdbTvShowSearchJob(OmdbApi& api, ShowSearchJob::Config _config, QObject* parent = nullptr);
    ~OmdbTvShowSearchJob() override = default;
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
