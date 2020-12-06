#pragma once

#include "data/Locale.h"
#include "globals/Meta.h"
#include "globals/ScraperInfos.h"
#include "scrapers/tv_show/EpisodeIdentifier.h"
#include "scrapers/tv_show/EpisodeScrapeJob.h"
#include "scrapers/tv_show/SeasonScrapeJob.h"
#include "scrapers/tv_show/ShowIdentifier.h"
#include "scrapers/tv_show/ShowScrapeJob.h"
#include "scrapers/tv_show/ShowSearchJob.h"
#include "tv_shows/SeasonOrder.h"

#include <QObject>
#include <QString>
#include <QUrl>

class TvShow;
class TvShowEpisode;

namespace mediaelch {
namespace scraper {

/// \brief A scraper for tvShows that allows searching for and loading of
///        TV show details.
class TvScraper : public QObject
{
    Q_OBJECT

public:
    /// \brief   Information object about the scraper.
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

        /// \brief A set of show details that the scraper supports.
        QSet<ShowScraperInfo> supportedShowDetails;

        /// \brief A set of  episodedetails that the scraper supports.
        QSet<EpisodeScraperInfo> supportedEpisodeDetails;

        /// \brief A set of season orders that the scraper supports.
        QSet<SeasonOrder> supportedSeasonOrders = {SeasonOrder::Aired};

        /// \brief A list of languages that are supported by the scraper.
        /// \see Locale::Locale
        QVector<Locale> supportedLanguages = {Locale::English};

        /// \brief Default locale for this scraper.
        Locale defaultLocale = Locale::English;
    };

public:
    explicit TvScraper(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~TvScraper() = default;

    /// \brief Information about the scraper.
    virtual const ScraperMeta& meta() const = 0;

    virtual void initialize() = 0;
    virtual bool isInitialized() const = 0;

signals:
    void initialized(bool wasSuccessful, TvScraper* scraper);

public:
    /// \brief Search for the given \p query.
    ///
    /// \param config Configuration for the search, e.g. language and search query.
    ELCH_NODISCARD virtual ShowSearchJob* search(ShowSearchJob::Config config) = 0;

    /// \brief   Load a TV show using the given identifier.
    /// \details Only the given details are loaded which - if only the title
    ///          shall be loaded - results in fewer network requests and faster lookup.
    ///
    /// \param config Configuration for the scrape job, e.g. language and show ID.
    ELCH_NODISCARD virtual ShowScrapeJob* loadShow(ShowScrapeJob::Config config) = 0;

    /// \brief   Load episodes of the given seasons.
    /// \details Only episodes of the configured seasons are loaded.
    ///
    /// \param config Configuration for the scrape job, e.g. language and show ID.
    ELCH_NODISCARD virtual SeasonScrapeJob* loadSeasons(SeasonScrapeJob::Config config) = 0;

    /// \brief   Load a TV episode using the given identifier.
    /// \details Only the given details are loaded which - if only the title
    ///          shall be loaded - results in fewer network requests and faster lookup.
    ///
    /// \param config Configuration for the scrape job, e.g. language and episode ID.
    ELCH_NODISCARD virtual EpisodeScrapeJob* loadEpisode(EpisodeScrapeJob::Config config) = 0;
};

} // namespace scraper
} // namespace mediaelch
