#pragma once

#include "scrapers/tv_show/ShowSearchJob.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace mediaelch {
namespace scraper {

class TvMazeApi;

class TvMazeShowSearchJob : public ShowSearchJob
{
    Q_OBJECT

public:
    explicit TvMazeShowSearchJob(TvMazeApi& api, ShowSearchJob::Config config, QObject* parent = nullptr);
    ~TvMazeShowSearchJob() override = default;

    void execute() override;

private:
    QVector<ShowSearchJob::Result> parseSearch(const QJsonDocument& json) const;
    ShowSearchJob::Result parseSingleSearchObject(const QJsonObject& json) const;

private:
    TvMazeApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
