#include "test/scrapers/testScraperHelper.h"

#include <QEventLoop>

namespace test {

QVector<mediaelch::scraper::MovieSearchJob::Result> searchMovieSync(mediaelch::scraper::MovieScraper& scraper,
    const mediaelch::scraper::MovieSearchJob::Config& config)
{
    using namespace mediaelch::scraper;
    QVector<MovieSearchJob::Result> results;
    QEventLoop loop;
    auto* searchJob = scraper.search(config);
    loop.connect(searchJob, &MovieSearchJob::sigSearchSuccess, [&](QVector<MovieSearchJob::Result> res) {
        results = std::move(res);
        loop.quit();
    });
    loop.connect(searchJob, &MovieSearchJob::sigSearchError, &loop, &QEventLoop::quit);
    loop.exec();
    return results;
}

void loadMovieSync(mediaelch::scraper::MovieScraper& scraper,
    Movie& movie,
    QString identifier,
    QString locale,
    mediaelch::scraper::MovieScraper::LoadDetails details)
{
    using namespace mediaelch::scraper;
    QEventLoop loop;

    MovieScrapeJob::Config config(identifier, Locale(locale), details);
    auto* scrapeJob = scraper.scrape(movie, config);

    loop.connect(scrapeJob, &MovieScrapeJob::sigScrapeSuccess, &loop, &QEventLoop::quit);
    loop.connect(scrapeJob, &MovieScrapeJob::sigScrapeError, &loop, &QEventLoop::quit);
    loop.exec();
}


} // namespace test
