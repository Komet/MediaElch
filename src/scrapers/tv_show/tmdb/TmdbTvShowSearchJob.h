#pragma once

#include "scrapers/tv_show/ShowSearchJob.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace mediaelch {
namespace scraper {

class TmdbTvApi;

class TmdbTvShowSearchJob : public ShowSearchJob
{
    Q_OBJECT

public:
    explicit TmdbTvShowSearchJob(TmdbTvApi& api, ShowSearchJob::Config _config, QObject* parent = nullptr);
    ~TmdbTvShowSearchJob() override = default;
    void execute() override;

private:
    QVector<ShowSearchJob::Result> parseSearch(const QJsonDocument& json);
    ShowSearchJob::Result parseSingleSearchObject(const QJsonObject& json);

private:
    TmdbTvApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
