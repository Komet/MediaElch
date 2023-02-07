#pragma once

#include "network/NetworkManager.h"
#include "scrapers/image/ImageProvider.h"
#include "scrapers/movie/MovieScraper.h"

#include <QObject>


namespace mediaelch {
namespace scraper {

class CustomMovieScraper : public MovieScraper
{
    Q_OBJECT
public:
    explicit CustomMovieScraper(QObject* parent = nullptr);
    static constexpr const char* ID = "custom-movie";
    static CustomMovieScraper* instance(QObject* parent = nullptr);

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD MovieSearchJob* search(MovieSearchJob::Config config) override;
    /// \brief   Load a movie using the given configuration. Requires setScraperMovieIds() beforehand.
    /// \details setScraperMovieIds() needs to be called beforehand to set up a
    ///          MovieScraper<->Identifier map.
    /// \see     setScraperMovieIds()
    ELCH_NODISCARD MovieScrapeJob* loadMovie(MovieScrapeJob::Config config) override;
    /// \brief Set up a MovieScraper<->Identifier map to be used by loadMovie().
    void setScraperMovieIds(QHash<MovieScraper*, MovieIdentifier> ids);

public:
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;

    QSet<MovieScraperInfo> scraperNativelySupports() override;

    void changeLanguage(mediaelch::Locale locale) override;

    MovieScraper* titleScraper();
    MovieScraper* scraperForInfo(MovieScraperInfo info);
    QMap<MovieScraperInfo, MovieScraper*> detailsToScrapers();
    // TODO: Maybe use some custom loadMovie() function? This seems hacky.
    QVector<MovieScraper*> scrapersNeedSearch(const QSet<MovieScraperInfo>& infos);

    QWidget* settingsWidget() override;

private:
    ImageProvider* imageProviderForInfo(int info);
    QVector<ImageProvider*> imageProvidersForInfos(QSet<MovieScraperInfo> infos);
    network::NetworkManager* network();
    void updateSupportedDetails();

private:
    ScraperMeta m_meta;
    network::NetworkManager m_network;
    QHash<MovieScraper*, MovieIdentifier> m_scraperMovieIds;
};

} // namespace scraper
} // namespace mediaelch
