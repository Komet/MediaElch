#pragma once

#include "scrapers/image/ImageProvider.h"
#include "utils/Meta.h"

#include <QObject>
#include <QString>
#include <QVector>
#include <memory>
#include <vector>

class Settings;

namespace mediaelch {
namespace scraper {

class MusicScraper;
class ConcertScraper;
class TvScraper;
class MovieScraper;
class CustomMovieScraper;

} // namespace scraper

class ScraperConfiguration;

} // namespace mediaelch

namespace mediaelch {

template<class Scraper>
class ManagedScraper
{
public:
    Scraper* scraper() const { return m_scraper.get(); }
    ScraperConfiguration* config() const { return m_config.get(); }
    QWidget* createView() const { return m_viewFactory(); }

private:
    std::unique_ptr<Scraper> m_scraper = nullptr;
    std::unique_ptr<ScraperConfiguration> m_config = nullptr;
    std::function<QWidget*()> m_viewFactory = []() { return nullptr; };

    friend class ScraperManager;
};

using ManagedMovieScraper = ManagedScraper<mediaelch::scraper::MovieScraper>;
using ManagedTvScraper = ManagedScraper<mediaelch::scraper::TvScraper>;
using ManagedConcertScraper = ManagedScraper<mediaelch::scraper::ConcertScraper>;
using ManagedMusicScraper = ManagedScraper<mediaelch::scraper::MusicScraper>;
using ManagedImageProvider = ManagedScraper<mediaelch::scraper::ImageProvider>;

class ScraperManager : public QObject
{
    Q_OBJECT

public:
    explicit ScraperManager(Settings& settings, QObject* parent = nullptr);
    ~ScraperManager() override;

    ELCH_NODISCARD mediaelch::scraper::MovieScraper* movieScraper(const QString& identifier);
    ELCH_NODISCARD mediaelch::ScraperConfiguration* movieScraperConfig(const QString& identifier);
    ELCH_NODISCARD QVector<mediaelch::scraper::MovieScraper*> movieScrapers();
    ELCH_NODISCARD const std::vector<ManagedMovieScraper>& allMovieScrapers();

    ELCH_NODISCARD scraper::CustomMovieScraper& customMovieScraper();

    ELCH_NODISCARD mediaelch::scraper::TvScraper* tvScraper(const QString& identifier);
    ELCH_NODISCARD mediaelch::ScraperConfiguration* tvScraperConfig(const QString& identifier);
    ELCH_NODISCARD QVector<mediaelch::scraper::TvScraper*> tvScrapers();
    ELCH_NODISCARD const std::vector<ManagedTvScraper>& allTvScrapers();

    ELCH_NODISCARD mediaelch::scraper::ConcertScraper* concertScraper(const QString& identifier);
    ELCH_NODISCARD mediaelch::ScraperConfiguration* concertScraperConfig(const QString& identifier);
    ELCH_NODISCARD QVector<mediaelch::scraper::ConcertScraper*> concertScrapers();
    ELCH_NODISCARD const std::vector<ManagedConcertScraper>& allConcertScrapers();

    ELCH_NODISCARD QVector<mediaelch::scraper::MusicScraper*> musicScrapers();
    ELCH_NODISCARD const std::vector<ManagedMusicScraper>& allMusicScrapers();
    ELCH_NODISCARD mediaelch::ScraperConfiguration* musicScraperConfig(const QString& identifier);

    ELCH_NODISCARD mediaelch::scraper::ImageProvider* imageProvider(const QString& identifier);
    ELCH_NODISCARD QVector<mediaelch::scraper::ImageProvider*> imageProviders();
    ELCH_NODISCARD mediaelch::ScraperConfiguration* imageProviderConfig(const QString& identifier);
    /// \brief Returns a list of all image providers available for type
    /// \param type Type of image
    /// \return List of pointers of image providers
    ELCH_NODISCARD QVector<mediaelch::scraper::ImageProvider*> imageProviders(ImageType type);
    ELCH_NODISCARD const std::vector<ManagedImageProvider>& allImageProviders();

private:
    void initMovieScrapers();
    void initTvScrapers();
    void initConcertScrapers();
    void initMusicScrapers();
    void initImageProviders();

private:
    Settings& m_settings;

    std::vector<ManagedMovieScraper> m_scraperMovies;
    std::vector<ManagedTvScraper> m_scraperTv;
    std::vector<ManagedConcertScraper> m_scraperConcert;
    std::vector<ManagedMusicScraper> m_scraperMusic;
    std::vector<ManagedImageProvider> m_imageProviders;
};

} // namespace mediaelch
