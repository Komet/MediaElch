#pragma once

#include "data/Locale.h"
#include "globals/Globals.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/ScraperResult.h"
#include "scrapers/movie/MovieIdentifier.h"
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

public:
    virtual void loadData(QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
        Movie* movie,
        QSet<MovieScraperInfo> infos) = 0;

    virtual QSet<MovieScraperInfo> scraperNativelySupports() = 0;
    virtual void changeLanguage(mediaelch::Locale locale) = 0;
    virtual QWidget* settingsWidget() = 0;
};

} // namespace scraper
} // namespace mediaelch


Q_DECLARE_METATYPE(mediaelch::scraper::MovieScraper*)
