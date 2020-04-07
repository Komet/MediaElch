#pragma once

#include "globals/ScraperInfos.h"
#include "scrapers/Locale.h"
#include "scrapers/ScraperInterface.h"

#include <QMap>
#include <QString>
#include <QVector>
#include <memory>

class Movie;

namespace mediaelch {
namespace scraper {

// This file contains all interfaces for a movie scraper:
class MovieSearchJob;
class MovieScrapeJob;
class MovieScraper;

/// \brief A movie search request resolved by a scraper.
class MovieSearchJob : public QObject
{
    Q_OBJECT

public:
    /// \brief Configuration object for a movie search.
    struct Config
    {
        Config(QString _query, Locale _locale) : query{std::move(_query)}, locale{std::move(_locale)} {}
        /// \brief The search string
        QString query;
        /// \brief Language key for the scraper, e.g. "en-US", "de-DE", ...
        Locale locale = Locale("en-US");
        /// \brief Whether the search results should also include entries that are NSFW.
        bool includeAdtulResults = false;
    };

    /// \brief Search result of a movie search request.
    struct Result
    {
        /// \brief Title shown to the user.
        QString title;
        /// \brief Release date of the movie.
        /// \details Date is used for showing the year behind the search result.
        QDate released;
        /// \brief Scraper specific identifier that may be used for scraping a movie.
        /// \details The identifier can be passed to scrape().
        QString identifier;
    };

public:
    /// \brief Extract the title and year from a search query.
    /// \details This function checks for common patterns and extract the title
    ///          and year if a pattern matches.
    /// \returns Title/Year pair if a pattern matched, empty string pair otherwise.
    static QPair<QString, QString> extractTitleAndYear(const QString& query);

public:
    /// \brief Create a movie search.
    explicit MovieSearchJob(Config config, QObject* parent = nullptr) : QObject(parent), m_config{std::move(config)} {}
    virtual ~MovieSearchJob() = default;

    virtual void execute() = 0;

    /// \brief Get a reference to the search's configuration.
    const Config& config() const { return m_config; }
    /// \brief Get the search string of this search.
    const QString& query() const { return m_config.query; }

signals:
    /// \brief Signal emitted when the search() request returns successfully.
    ///
    /// \param results Search results
    void sigSearchSuccess(QVector<MovieSearchJob::Result> results);

    /// \brief Signal emitted when the search() request returns erroneous.
    ///        For example for a network error.
    ///
    /// \param error Object describing the error in textual form.
    void sigSearchError(ScraperSearchError error);

private:
    const Config m_config;
};


class MovieScrapeJob : public QObject
{
    Q_OBJECT

public:
    /// \brief Configuration object for a movie scrape job.
    struct Config
    {
        Config(QString _identifier, Locale _locale, QVector<MovieScraperInfos> _details) :
            identifier{std::move(_identifier)}, locale{std::move(_locale)}, details{std::move(_details)}
        {
        }

        /// \brief A string that can be consumed by the movie scraper.
        /// \details It is used to uniquely identify the movie. May be an IMDb ID in
        ///          string representation or an URL.
        QString identifier;

        /// \brief Language key for the scraper, e.g. "en-US", "de-DE", ...
        Locale locale = Locale("en-US");

        /// \brief Details to be loaded using the scraper.
        QVector<MovieScraperInfos> details;

        /// \brief Whether to set the movie's outline based on the plot.
        /// \see Settings::usePlotForOutline()
        bool usePlotForOutline = false;
    };

public:
    MovieScrapeJob(MovieScraper& scraper, Movie& movie, Config config, QObject* parent = nullptr) :
        QObject(parent), m_scraper{scraper}, m_movie{movie}, m_config{config}
    {
    }

    virtual void execute() = 0;

    MovieScraper& scraper() { return m_scraper; }
    Movie& movie() { return m_movie; }

    const Locale& locale() { return m_config.locale; }
    const QString& identifier() { return m_config.identifier; }
    const QVector<MovieScraperInfos>& details() { return m_config.details; }
    bool usePlotForOutline() const { return m_config.usePlotForOutline; }

signals:
    /// \brief Signal emitted when the scrape() request returns successfully.
    void sigScrapeSuccess();

    /// \brief Signal emitted when the scrape() request returns erroneous.
    ///        For example for a network error.
    ///
    /// \param error Object describing the error in textual form.
    void sigScrapeError(ScraperLoadError error);

    /// \brief Signals a download progress. Useful if a scraper has to load
    ///        data from multiple sites or sends multiple requests.
    void sigProgress(int progress, int max);

private:
    MovieScraper& m_scraper;
    Movie& m_movie;
    const Config m_config;
};


/// \brief A scraper for movies that allows searching for and loading of
///        movie details.
class MovieScraper : public QObject
{
    Q_OBJECT

public:
    using LoadDetails = QVector<MovieScraperInfos>;

    /// \brief   Information object about the scraper.
    /// \details This object can be used to display details about the scraper.
    ///          For example in the "About" dialog for each scraper or similar.
    struct ScraperInfo
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
        QString website;

        /// \brief A URL to the data provider's data policy.
        QUrl privacyPolicy;

        /// \brief A URL to the data provider's contact page or forum.
        QUrl help;

        /// \brief Returns true if the scraper is NSFW (i.e. only for people older than 18
        ///        in most countries).
        bool isAdultScraper = false;

        /// \brief Returns a list of details that the scraper supports.
        LoadDetails scraperSupports;

        /// \brief TODO
        QVector<Locale> supportedLanguages = {Locale("en-US")};

        /// \brief TODO
        Locale defaultLocale = Locale("en-US");
    };

public:
    explicit MovieScraper(QObject* parent = nullptr);
    virtual ~MovieScraper() = default;

    /// \brief Information about the scraper.
    virtual const ScraperInfo& info() const = 0;

    virtual void initialize() = 0;
    virtual bool isInitialized() const = 0;

    /// \brief Search for the given \p query.
    ///
    /// \param config  Configuration for the search, e.g. language and search query.
    virtual MovieSearchJob* search(MovieSearchJob::Config config) = 0;

    /// \brief   Load a movie using the given identifier.
    /// \details Only the given details are loaded which - if only the title
    ///          shall be loaded - results in fewer network requests and faster lookup.
    ///
    /// \param movie  Movie to scrape the details into. Due to the Qt signal/slot
    ///               handling, we can't pass a unique_ptr as an argument to the slot.
    ///               Returning a movie from this method would also result in ownership
    ///               issues.
    virtual MovieScrapeJob* scrape(Movie& movie, MovieScrapeJob::Config config) = 0;

signals:
    void initialized();
};

} // namespace scraper
} // namespace mediaelch
