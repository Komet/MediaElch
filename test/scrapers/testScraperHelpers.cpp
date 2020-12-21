#include "test/scrapers/testScraperHelpers.h"

#include "test/test_helpers.h"

using namespace mediaelch;

QPair<QVector<mediaelch::scraper::ShowSearchJob::Result>, ScraperError>
searchTvScraperSync(mediaelch::scraper::ShowSearchJob* searchJob, bool mayError)
{
    QVector<mediaelch::scraper::ShowSearchJob::Result> results;
    ScraperError error;
    QEventLoop loop;
    QEventLoop::connect(
        searchJob, &mediaelch::scraper::ShowSearchJob::sigFinished, [&](mediaelch::scraper::ShowSearchJob* /*unused*/) {
            results = searchJob->results();
            error = searchJob->error();
            searchJob->deleteLater();
            loop.quit();
        });
    searchJob->execute();
    loop.exec();
    if (!mayError) {
        CAPTURE(error.message);
        CHECK(error.error == ScraperError::Type::NoError);
    }
    return {results, error};
}

void scrapeTvScraperSync(mediaelch::scraper::ShowScrapeJob* scrapeJob, bool mayError)
{
    QEventLoop loop;
    QEventLoop::connect(scrapeJob,
        &mediaelch::scraper::ShowScrapeJob::sigFinished,
        [&](mediaelch::scraper::ShowScrapeJob* /*unused*/) { loop.quit(); });
    scrapeJob->execute();
    loop.exec();
    if (!mayError) {
        CAPTURE(scrapeJob->error().message);
        CHECK(!scrapeJob->hasError());
    }
}
