#pragma once

#include "concerts/Concert.h"
#include "globals/Globals.h"
#include "globals/ScraperResult.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/concert/ConcertSearchJob.h"

class Concert;

namespace mediaelch {
namespace scraper {

/// \brief The ConcertScraper class
/// This class is the base for every concert Scraper.
class ConcertScraper : public QObject, public ScraperInterface
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

        /// \brief A set of concert details that the scraper supports.
        QSet<ConcertScraperInfo> supportedDetails;

        /// \brief A list of languages that are supported by the scraper.
        /// \see Locale::Locale
        QVector<Locale> supportedLanguages = {Locale::English};

        /// \brief Default locale for this scraper.
        Locale defaultLocale = Locale::English;
    };

public:
    explicit ConcertScraper(QObject* parent = nullptr) : QObject(parent) {}

    virtual const ScraperMeta& meta() const = 0;

    virtual void initialize() = 0;
    virtual bool isInitialized() const = 0;

    /// \brief Search for the given \p query.
    ///
    /// \param config Configuration for the search, e.g. language and search query.
    ELCH_NODISCARD virtual ConcertSearchJob* search(ConcertSearchJob::Config config) = 0;

    virtual void loadData(TmdbId id, Concert* concert, QSet<ConcertScraperInfo> infos) = 0;
    virtual QSet<ConcertScraperInfo> scraperSupports() = 0;
    virtual QWidget* settingsWidget() = 0;

signals:
    void initialized(bool wasSuccessful, ConcertScraper* scraper);
};

} // namespace scraper
} // namespace mediaelch
