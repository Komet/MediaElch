#include "test/scrapers/testScraperHelpers.h"

#include "test/test_helpers.h"

#include <sstream>

using namespace mediaelch;

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

std::ostream& operator<<(std::ostream& os, const QVector<Actor*>& value)
{
    return os << "Actors vector (" << value.size() << " actors)";
}

bool HasActorMatcher::match(const QVector<Actor*>& actors) const
{
    for (const Actor* actor : asConst(actors)) {
        if (actor->name == m_name) {
            return (actor->role == m_role);
        }
    }
    return false;
}

std::string HasActorMatcher::describe() const
{
    std::ostringstream ss;
    ss << "has actor " << m_name << " with role " << m_role << ".";
    return ss.str();
}

HasActorMatcher HasActor(const QString& name, const QString& role)
{
    return HasActorMatcher(name, role);
}
