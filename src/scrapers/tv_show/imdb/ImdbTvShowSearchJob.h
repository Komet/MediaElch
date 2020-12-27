#pragma once

#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/tv_show/TvScraper.h"

namespace mediaelch {
namespace scraper {

class ImdbTvShowSearchJob : public ShowSearchJob
{
    Q_OBJECT

public:
    explicit ImdbTvShowSearchJob(ImdbApi& api, ShowSearchJob::Config _config, QObject* parent = nullptr);
    ~ImdbTvShowSearchJob() override = default;
    void execute() override;

private:
    QVector<ShowSearchJob::Result> parseSearch(const QString& html);
    QVector<ShowSearchJob::Result> parseResultFromShowPage(const QString& html);
    /// \brief   Check if the HTML page is a 404 page
    /// \details IMDb does not return a 404 status code but instead a 204 one with
    ///          a page that says "404 Error".
    bool is404(const QString& html) const;

private:
    ImdbApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
