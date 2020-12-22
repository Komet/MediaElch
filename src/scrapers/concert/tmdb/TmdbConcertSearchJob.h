#pragma once

#include "scrapers/concert/ConcertSearchJob.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace mediaelch {
namespace scraper {

class TmdbApi;

class TmdbConcertSearchJob : public ConcertSearchJob
{
    Q_OBJECT

public:
    explicit TmdbConcertSearchJob(TmdbApi& api, ConcertSearchJob::Config _config, QObject* parent = nullptr);
    ~TmdbConcertSearchJob() override = default;
    void execute() override;

private:
    QVector<ConcertSearchJob::Result> parseSearch(const QJsonDocument& json);
    ConcertSearchJob::Result parseSingleSearchObject(const QJsonObject& json);

private:
    TmdbApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
