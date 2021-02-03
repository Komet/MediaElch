#pragma once

#include "data/Locale.h"
#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "globals/ScraperResult.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/movie/MovieScrapeJob.h"
#include "scrapers/movie/MovieSearchJob.h"
#include "settings/ScraperSettings.h"

#include <QMap>
#include <QString>
#include <QVector>
#include <vector>

class Movie;

namespace mediaelch {
namespace scraper {

/// \brief The MovieScraper class
/// This class is the base for every movie Scraper.
class MovieScraper : public QObject, public ScraperInterface
{
    Q_OBJECT
public:
    /// \brief   Information object about the movie scraper.
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

        /// \brief A set of movie details that the scraper supports.
        QSet<MovieScraperInfo> supportedDetails;

        /// \brief A list of languages that are supported by the scraper.
        /// \see Locale::Locale
        QVector<Locale> supportedLanguages = {Locale::English};

        /// \brief Default locale for this scraper.
        Locale defaultLocale = Locale::English;

        /// \brief Whether the scraper loads adult-only content.
        bool isAdult = false;
    };

public:
    explicit MovieScraper(QObject* parent = nullptr) : QObject(parent) {}
    /// \brief Information about the scraper.
    virtual const ScraperMeta& meta() const = 0;

    virtual void initialize() = 0;
    virtual bool isInitialized() const = 0;

    /// \brief Search for the given \p query.
    ///
    /// \param config Configuration for the search, e.g. language and search query.
    ELCH_NODISCARD virtual MovieSearchJob* search(MovieSearchJob::Config config) = 0;

    /// \brief   Load a movie using the given identifier.
    /// \details Only the given details are loaded which - if only the title
    ///          shall be loaded - results in fewer network requests and faster lookup.
    ///
    /// \param config Configuration for the scrape job, e.g. language and movie ID.
    ELCH_NODISCARD virtual MovieScrapeJob* loadMovie(MovieScrapeJob::Config config) = 0;

public:
    /// \todo Remove
    virtual QSet<MovieScraperInfo> scraperNativelySupports() = 0;
    /// \todo Remove
    virtual void changeLanguage(mediaelch::Locale locale) = 0;
    /// \todo Remove and move into own settings classes.
    virtual QWidget* settingsWidget() = 0;
};

} // namespace scraper
} // namespace mediaelch
