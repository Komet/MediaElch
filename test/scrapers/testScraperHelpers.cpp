#include "test/scrapers/testScraperHelpers.h"

#include "test/test_helpers.h"

using namespace mediaelch;

QPair<QVector<mediaelch::scraper::ConcertSearchJob::Result>, ScraperError>
searchConcertScraperSync(mediaelch::scraper::ConcertSearchJob* searchJob, bool mayError)
{
    QVector<mediaelch::scraper::ConcertSearchJob::Result> results;
    ScraperError error;
    QEventLoop loop;
    QEventLoop::connect(searchJob,
        &mediaelch::scraper::ConcertSearchJob::sigFinished,
        [&](mediaelch::scraper::ConcertSearchJob* /*unused*/) {
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
        CAPTURE(error.technical);
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
        CAPTURE(scrapeJob->error().technical);
        CHECK(!scrapeJob->hasError());
    }
}
