#pragma once

#include "scrapers/tv_show/ShowSearchJob.h"

#include <QJsonObject>
#include <QString>

namespace mediaelch {
namespace scraper {

class TheTvDbApi;

class TheTvDbShowSearchJob : public ShowSearchJob
{
    Q_OBJECT

public:
    explicit TheTvDbShowSearchJob(TheTvDbApi& api, ShowSearchJob::Config config, QObject* parent = nullptr);
    ~TheTvDbShowSearchJob() override = default;

    void execute() override;

private:
    TheTvDbApi& m_api;

    QVector<ShowSearchJob::Result> parseSearch(const QJsonDocument& json);
    ShowSearchJob::Result parseSingleSearchResult(const QJsonObject& showObject);
};

} // namespace scraper
} // namespace mediaelch
