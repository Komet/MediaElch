#pragma once

#include "scrapers/movie/MovieSearchJob.h"

#include <QString>

namespace mediaelch {
namespace scraper {

class AebnApi;

class AebnSearchJob : public MovieSearchJob
{
    Q_OBJECT

public:
    explicit AebnSearchJob(AebnApi& api, MovieSearchJob::Config _config, QString genre, QObject* parent = nullptr);
    ~AebnSearchJob() override = default;

    void execute() override;

private:
    void parseSearch(const QString& html);

private:
    AebnApi& m_api;
    QString m_genreId;
};

} // namespace scraper
} // namespace mediaelch
