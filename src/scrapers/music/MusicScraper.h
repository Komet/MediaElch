#pragma once

#include "data/Locale.h"
#include "data/MusicBrainzId.h"
#include "globals/Globals.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/ScraperResult.h"
#include "workers/Job.h"

#include <QSet>
#include <QString>
#include <QVector>

class Album;
class Artist;

namespace mediaelch {
namespace scraper {

/// \brief An artist search request resolved by a scraper.
class ArtistSearchJob : public worker::Job
{
    Q_OBJECT

public:
    /// \brief Configuration object for an artist search.
    struct Config
    {
        /// \brief The search string
        QString query;
        /// \brief Language key for the scraper, e.g. "en-US", "de-DE", ...
        Locale locale = Locale::English;
    };

    /// \brief Search result of an artist search request.
    struct Result
    {
        /// \brief Title shown to the user.
        QString title;
        /// \brief Release date of the artist.
        /// \details Date is used for showing the year behind the search result.
        QDate released;
        /// \brief Scraper specific identifier that may be used for scraping an artist.
        /// \details The identifier can be passed to a load job.
        QString identifier;
    };

public:
    /// \brief Create an artist search.
    explicit ArtistSearchJob(Config config, QObject* parent = nullptr);
    ~ArtistSearchJob() override = default;

public:
    ELCH_NODISCARD const Config& config() const;
    ELCH_NODISCARD const ScraperError& scraperError() const;
    ELCH_NODISCARD const QVector<ArtistSearchJob::Result>& results() const;

signals:
    /// \brief   Signal emitted when the search() request has finished.
    /// \details A simple wrapper around finished() to avoid static_asserts
    ///          from Job* to ArtistSearchJob*.
    ///          Use hasError() and results() to know whether the request was successful.
    void searchFinished(mediaelch::scraper::ArtistSearchJob* searchJob, QPrivateSignal);

protected:
    void setScraperError(ScraperError error);

protected:
    QVector<ArtistSearchJob::Result> m_results;

private:
    ScraperError m_scraperError;
    const Config m_config;
};


class ArtistScrapeJob : public worker::Job
{
    Q_OBJECT

public:
    /// \brief Configuration object for an artist scrape job.
    struct Config
    {
        /// \brief A string that can be consumed by the artist scraper.
        /// \details It is used to uniquely identify the artist. May be an IMDb ID in
        ///          string representation or an URL.
        QString identifier;
        /// \brief Language key for the scraper, e.g. "en-US", "de-DE", ...
        /// \details Default language is "no locale" to more easily detect issues and to make
        ///          it possible that artist scrapers can check against something to set defaults.
        Locale locale = Locale::NoLocale;
        /// \brief Music details to be loaded using the scraper.
        QSet<MusicScraperInfo> details;
    };

public:
    explicit ArtistScrapeJob(Config config, QObject* parent = nullptr);
    ~ArtistScrapeJob() override = default;

public:
    ELCH_NODISCARD Artist& artist() { return *m_artist; }
    ELCH_NODISCARD const Artist& artist() const { return *m_artist; }

    ELCH_NODISCARD const Config& config() const { return m_config; }
    ELCH_NODISCARD const ScraperError& scraperError() const;

signals:
    /// \brief   Signal emitted when the scrape job has finished.
    /// \details A simple wrapper around finished() to avoid static_asserts
    ///          from Job* to ArtistScrapeJob*.
    ///          Use hasError() and artist() to know whether the request was successful.
    void loadFinished(mediaelch::scraper::ArtistScrapeJob* scrapeJob, QPrivateSignal);

protected:
    void setScraperError(ScraperError error);

protected:
    Artist* m_artist = nullptr;

private:
    const Config m_config;
    ScraperError m_scraperError;
};


/// \brief An album search request resolved by a scraper.
class AlbumSearchJob : public worker::Job
{
    Q_OBJECT

public:
    /// \brief Configuration object for an album search.
    struct Config
    {
        /// \brief Optional artist name
        /// \details If artist name is empty, only the albumQuery is used.
        ///          Otherwise, the artist will be used for the result as well.
        QString artistName;
        /// \brief The search string for the album
        QString albumQuery;
        /// \brief Language key for the scraper, e.g. "en-US", "de-DE", ...
        Locale locale = Locale::English;
    };

    /// \brief Search result of an album search request.
    struct Result
    {
        /// \brief Title shown to the user.
        QString title;
        /// \brief Release date of the album.
        /// \details Date is used for showing the year behind the search result.
        QDate released;
        /// \brief Scraper specific identifier that may be used for scraping an album.
        /// \details The identifier can be passed to a load job.
        QString identifier;
        /// \brief Used for MusicBrainz Release Group ID
        QString groupIdentifier;
    };

public:
    /// \brief Create an artist search.
    explicit AlbumSearchJob(Config config, QObject* parent = nullptr);
    ~AlbumSearchJob() override = default;

public:
    ELCH_NODISCARD const Config& config() const;
    ELCH_NODISCARD const ScraperError& scraperError() const;
    ELCH_NODISCARD const QVector<AlbumSearchJob::Result>& results() const;

signals:
    /// \brief   Signal emitted when the search() request has finished.
    /// \details A simple wrapper around finished() to avoid static_asserts
    ///          from Job* to AlbumSearchJob*.
    ///          Use hasError() and results() to know whether the request was successful.
    void searchFinished(mediaelch::scraper::AlbumSearchJob* searchJob, QPrivateSignal);

protected:
    void setScraperError(ScraperError error);

protected:
    QVector<AlbumSearchJob::Result> m_results;

private:
    ScraperError m_scraperError;
    const Config m_config;
};


class AlbumScrapeJob : public worker::Job
{
    Q_OBJECT

public:
    /// \brief Configuration object for an album scrape job.
    struct Config
    {
        /// \brief A string that can be consumed by the album scraper.
        /// \details It is used to uniquely identify the album. May be an IMDb ID in
        ///          string representation or an URL.
        QString identifier;
        /// \brief Used for MusicBrainz Release Group ID
        QString groupIdentifier;
        /// \brief Language key for the scraper, e.g. "en-US", "de-DE", ...
        /// \details Default language is "no locale" to more easily detect issues and to make
        ///          it possible that album scrapers can check against something to set defaults.
        Locale locale = Locale::NoLocale;
        /// \brief album details to be loaded using the scraper.
        QSet<MusicScraperInfo> details;
    };

public:
    explicit AlbumScrapeJob(Config config, QObject* parent = nullptr);
    ~AlbumScrapeJob() override = default;

public:
    ELCH_NODISCARD Album& album() { return *m_album; }
    ELCH_NODISCARD const Album& album() const { return *m_album; }

    ELCH_NODISCARD const Config& config() const { return m_config; }
    ELCH_NODISCARD const ScraperError& scraperError() const;

signals:
    /// \brief   Signal emitted when the scrape job has finished.
    /// \details A simple wrapper around finished() to avoid static_asserts
    ///          from Job* to AlbumScrapeJob*.
    ///          Use hasError() and album() to know whether the request was successful.
    void loadFinished(mediaelch::scraper::AlbumScrapeJob* scrapeJob, QPrivateSignal);

protected:
    void setScraperError(ScraperError error);

protected:
    Album* m_album = nullptr;

private:
    const Config m_config;
    ScraperError m_scraperError;
};

class MusicScraper : public QObject, public ScraperInterface
{
    Q_OBJECT

public:
    /// \brief   Information object about the music scraper.
    /// \details This object can be used to display details about the scraper.
    ///          For example in the "About" dialog for each scraper or similar.
    struct ScraperMeta
    {
        /// \brief Unique identifier used to store settings and more.
        /// \details The identifier must not be changed once set and is often the
        /// lowercase name of the data provider without spaces or other special characters.
        QString identifier;

        /// \brief Human readable name of the scraper. Often its title.
        QString name;

        /// \brief Short description of the scraper, i.e. a one-liner.
        QString description;

        /// \brief The data provider's website, e.g. https://kodi.tv
        QUrl website;

        /// \brief An URL to the provider's terms of service.
        QUrl termsOfService;

        /// \brief An URL to the data provider's data policy.
        QUrl privacyPolicy;

        /// \brief An URL to the data provider's contact page or forum.
        QUrl help;

        /// \brief A set of music details that the scraper supports.
        QSet<MusicScraperInfo> supportedDetails;

        /// \brief A list of languages that are supported by the scraper.
        /// \see Locale::Locale
        QVector<Locale> supportedLanguages = {Locale::English};

        /// \brief Default locale for this scraper.
        Locale defaultLocale = Locale::English;
    };

public:
    explicit MusicScraper(QObject* parent = nullptr) : QObject(parent) {}
    ~MusicScraper() override = default;
    /// \brief Information about the scraper.
    ELCH_NODISCARD virtual const ScraperMeta& meta() const = 0;

    virtual void initialize() = 0;
    ELCH_NODISCARD virtual bool isInitialized() const = 0;

    /// \brief Search for an artist given a \p query.
    ///
    /// \param config Configuration for the search, e.g. language and search query.
    ELCH_NODISCARD virtual ArtistSearchJob* searchArtist(ArtistSearchJob::Config config) = 0;

    /// \brief Search for an album given a \p query.
    ///
    /// \param config Configuration for the search, e.g. language and search query.
    ELCH_NODISCARD virtual AlbumSearchJob* searchAlbum(AlbumSearchJob::Config config) = 0;

    /// \brief   Load an artist using the given identifier.
    /// \details Only the given details are loaded which may - if only the title
    ///          shall be loaded - results in fewer network requests and faster lookup.
    ///
    /// \param config Configuration for the scrape job, e.g. language and artist ID.
    ELCH_NODISCARD virtual ArtistScrapeJob* loadArtist(ArtistScrapeJob::Config config) = 0;

    /// \brief   Load an artist using the given identifier.
    /// \details Only the given details are loaded which may - if only the title
    ///          shall be loaded - results in fewer network requests and faster lookup.
    ///
    /// \param config Configuration for the scrape job, e.g. language and artist ID.
    ELCH_NODISCARD virtual AlbumScrapeJob* loadAlbum(AlbumScrapeJob::Config config) = 0;

signals:
    void initialized(bool isInitialized); // TODO: Implement in scrapers
};

QVector<ScraperSearchResult> toOldScraperSearchResult(const QVector<ArtistSearchJob::Result>& searchResults);
QVector<ScraperSearchResult> toOldScraperSearchResult(const QVector<AlbumSearchJob::Result>& searchResults);


} // namespace scraper
} // namespace mediaelch
