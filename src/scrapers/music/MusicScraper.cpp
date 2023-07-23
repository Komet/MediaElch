#include "scrapers/music/MusicScraper.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"

namespace mediaelch {
namespace scraper {

ArtistSearchJob::ArtistSearchJob(ArtistSearchJob::Config config, QObject* parent) :
    worker::Job(parent), m_config{std::move(config)}
{
    // Wrapper to avoid static_assert calls.
    connect(this, &Job::finished, this, [this]() { emit searchFinished(this, QPrivateSignal{}); });

    // TODO: Change to true / remove once all usages of ShowSearchJob are updated.
    setAutoDelete(false);
}

const ArtistSearchJob::Config& ArtistSearchJob::config() const
{
    return m_config;
}

const ScraperError& ArtistSearchJob::scraperError() const
{
    return m_scraperError;
}

const QVector<ArtistSearchJob::Result>& ArtistSearchJob::results() const
{
    return m_results;
}

void ArtistSearchJob::setScraperError(ScraperError error)
{
    m_scraperError = std::move(error);
    setError(static_cast<int>(m_scraperError.error));
    setErrorString(m_scraperError.message);
    setErrorText(m_scraperError.technical);
}


ArtistScrapeJob::ArtistScrapeJob(ArtistScrapeJob::Config config, QObject* parent) :
    worker::Job(parent), m_artist{new Artist({}, this)}, m_config{std::move(config)}
{
    // Wrapper to avoid static_assert calls.
    connect(this, &Job::finished, this, [this]() { emit loadFinished(this, QPrivateSignal{}); });

    // TODO: Change to true / remove once all usages of ArtistScrapeJob are updated.
    setAutoDelete(false);
}

const ScraperError& ArtistScrapeJob::scraperError() const
{
    return m_scraperError;
}

void ArtistScrapeJob::setScraperError(ScraperError error)
{
    m_scraperError = std::move(error);
    setError(static_cast<int>(m_scraperError.error));
    setErrorString(m_scraperError.message);
    setErrorText(m_scraperError.technical);
}


AlbumSearchJob::AlbumSearchJob(AlbumSearchJob::Config config, QObject* parent) :
    worker::Job(parent), m_config{std::move(config)}
{
    // Wrapper to avoid static_assert calls.
    connect(this, &Job::finished, this, [this]() { emit searchFinished(this, QPrivateSignal{}); });

    // TODO: Change to true / remove once all usages of ShowSearchJob are updated.
    setAutoDelete(false);
}

const AlbumSearchJob::Config& AlbumSearchJob::config() const
{
    return m_config;
}

const ScraperError& AlbumSearchJob::scraperError() const
{
    return m_scraperError;
}

const QVector<AlbumSearchJob::Result>& AlbumSearchJob::results() const
{
    return m_results;
}

void AlbumSearchJob::setScraperError(ScraperError error)
{
    m_scraperError = std::move(error);
    setError(static_cast<int>(m_scraperError.error));
    setErrorString(m_scraperError.message);
    setErrorText(m_scraperError.technical);
}


AlbumScrapeJob::AlbumScrapeJob(AlbumScrapeJob::Config config, QObject* parent) :
    worker::Job(parent), m_album{new Album({}, this)}, m_config{std::move(config)}
{
    // Wrapper to avoid static_assert calls.
    connect(this, &Job::finished, this, [this]() { emit loadFinished(this, QPrivateSignal{}); });

    // TODO: Change to true / remove once all usages of AlbumScrapeJob are updated.
    setAutoDelete(false);
}

const ScraperError& AlbumScrapeJob::scraperError() const
{
    return m_scraperError;
}

void AlbumScrapeJob::setScraperError(ScraperError error)
{
    m_scraperError = std::move(error);
    setError(static_cast<int>(m_scraperError.error));
    setErrorString(m_scraperError.message);
    setErrorText(m_scraperError.technical);
}


QVector<ScraperSearchResult> toOldScraperSearchResult(const QVector<ArtistSearchJob::Result>& searchResults)
{
    QVector<ScraperSearchResult> results;
    for (const auto& searchResult : searchResults) {
        ScraperSearchResult result;
        result.id = searchResult.identifier;
        result.name = searchResult.title;
        result.released = searchResult.released;
        results.push_back(result);
    }
    return results;
}

QVector<ScraperSearchResult> toOldScraperSearchResult(const QVector<AlbumSearchJob::Result>& searchResults)
{
    QVector<ScraperSearchResult> results;
    for (const auto& searchResult : searchResults) {
        ScraperSearchResult result;
        result.id = searchResult.identifier;
        result.name = searchResult.title;
        result.released = searchResult.released;
        results.push_back(result);
    }
    return results;
}

} // namespace scraper
} // namespace mediaelch
