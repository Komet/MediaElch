#include "scraper_helpers.h"

#include "test/test_helpers.h"

#include <sstream>

using namespace mediaelch;

namespace test {

QPair<QVector<mediaelch::scraper::ConcertSearchJob::Result>, ScraperError>
searchConcertScraperSync(mediaelch::scraper::ConcertSearchJob* searchJob, bool mayError)
{
    QVector<mediaelch::scraper::ConcertSearchJob::Result> results;
    ScraperError error;
    QEventLoop loop;
    QEventLoop::connect(searchJob,
        &mediaelch::scraper::ConcertSearchJob::searchFinished,
        &loop,
        [&](mediaelch::scraper::ConcertSearchJob* /*unused*/) {
            results = searchJob->results();
            error = searchJob->scraperError();
            searchJob->deleteLater();
            loop.quit();
        });
    searchJob->start();
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
        searchJob, &mediaelch::scraper::ShowSearchJob::finished, [&](mediaelch::worker::Job* /*unused*/) {
            results = searchJob->results();
            error = searchJob->scraperError();
            searchJob->deleteLater();
            loop.quit();
        });
    searchJob->start();
    loop.exec();
    if (!mayError) {
        CAPTURE(error.message);
        CAPTURE(error.technical);
        CHECK(error.error == ScraperError::Type::NoError);
    }
    return {results, error};
}

QPair<QVector<mediaelch::scraper::MovieSearchJob::Result>, ScraperError>
searchMovieScraperSync(mediaelch::scraper::MovieSearchJob* searchJob, bool mayError)
{
    QVector<mediaelch::scraper::MovieSearchJob::Result> results;
    ScraperError error;
    QEventLoop loop;
    QEventLoop::connect(searchJob,
        &mediaelch::scraper::MovieSearchJob::searchFinished,
        &loop,
        [&](mediaelch::scraper::MovieSearchJob* /*unused*/) {
            results = searchJob->results();
            error = searchJob->scraperError();
            searchJob->deleteLater();
            loop.quit();
        });
    searchJob->start();
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
    QEventLoop::connect(scrapeJob, &mediaelch::scraper::ShowScrapeJob::loadFinished, &loop, &QEventLoop::quit);
    scrapeJob->start();
    loop.exec();
    if (!mayError) {
        CAPTURE(scrapeJob->errorCode());
        CAPTURE(scrapeJob->errorString());
        CAPTURE(scrapeJob->errorText());
        CHECK(!scrapeJob->hasError());
    }
}

void scrapeEpisodeSync(mediaelch::scraper::EpisodeScrapeJob* scrapeJob, bool mayError)
{
    QEventLoop loop;
    QEventLoop::connect(scrapeJob, &mediaelch::scraper::EpisodeScrapeJob::loadFinished, &loop, &QEventLoop::quit);
    scrapeJob->start();
    loop.exec();
    if (!mayError) {
        CAPTURE(scrapeJob->errorCode());
        CAPTURE(scrapeJob->errorString());
        CAPTURE(scrapeJob->errorText());
        REQUIRE(!scrapeJob->hasError());
    }
}

void scrapeSeasonSync(mediaelch::scraper::SeasonScrapeJob* scrapeJob, bool mayError)
{
    QEventLoop loop;
    QEventLoop::connect(scrapeJob, &mediaelch::scraper::SeasonScrapeJob::loadFinished, &loop, &QEventLoop::quit);
    scrapeJob->start();
    loop.exec();
    if (!mayError) {
        CAPTURE(scrapeJob->errorCode());
        CAPTURE(scrapeJob->errorString());
        CAPTURE(scrapeJob->errorText());
        REQUIRE(!scrapeJob->hasError());
    }
}

void scrapeMovieScraperSync(mediaelch::scraper::MovieScrapeJob* scrapeJob, bool mayError)
{
    QEventLoop loop;
    scrapeJob->setAutoDelete(false); // otherwise the job is deleted after `loop` has finished.
    QEventLoop::connect(scrapeJob, &mediaelch::scraper::MovieScrapeJob::loadFinished, &loop, &QEventLoop::quit);
    scrapeJob->start();
    loop.exec();
    if (!mayError) {
        CAPTURE(scrapeJob->errorCode());
        CAPTURE(scrapeJob->errorString());
        CAPTURE(scrapeJob->errorText());
        CHECK(!scrapeJob->hasError());
    }
}

} // namespace test
