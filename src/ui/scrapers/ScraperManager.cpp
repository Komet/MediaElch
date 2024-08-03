#include "ui/scrapers/ScraperManager.h"

#include "log/Log.h"
#include "scrapers/ScraperConfiguration.h"
#include "scrapers/concert/ConcertScraper.h"
#include "scrapers/concert/tmdb/TmdbConcert.h"
#include "scrapers/concert/tmdb/TmdbConcertConfiguration.h"
#include "scrapers/image/FanartTv.h"
#include "scrapers/image/FanartTvConfiguration.h"
#include "scrapers/image/FanartTvMusic.h"
#include "scrapers/image/FanartTvMusicArtists.h"
#include "scrapers/image/TheTvDbImages.h"
#include "scrapers/image/TmdbImages.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/adultdvdempire/AdultDvdEmpire.h"
#include "scrapers/movie/aebn/AEBN.h"
#include "scrapers/movie/aebn/AebnConfiguration.h"
#include "scrapers/movie/custom/CustomMovieScraper.h"
#include "scrapers/movie/custom/CustomMovieScraperConfiguration.h"
#include "scrapers/movie/hotmovies/HotMovies.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/movie/imdb/ImdbMovieConfiguration.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovieConfiguration.h"
#include "scrapers/movie/videobuster/VideoBuster.h"
#include "scrapers/music/MusicScraper.h"
#include "scrapers/music/UniversalMusicConfiguration.h"
#include "scrapers/music/UniversalMusicScraper.h"
#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/custom/CustomTvScraper.h"
#include "scrapers/tv_show/fernsehserien_de/FernsehserienDe.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvConfiguration.h"
#include "scrapers/tv_show/tvmaze/TvMaze.h"
#include "settings/Settings.h"
#include "ui/scrapers/concert/TmdbConcertConfigurationView.h"
#include "ui/scrapers/image/FanartTvConfigurationView.h"
#include "ui/scrapers/movie/AebnConfigurationView.h"
#include "ui/scrapers/movie/ImdbMovieConfigurationView.h"
#include "ui/scrapers/movie/TmdbMovieConfigurationView.h"
#include "ui/scrapers/music/UniversalMusicConfigurationView.h"
#include "ui/scrapers/tv_show/FernsehserienDeConfigurationView.h"
#include "ui/scrapers/tv_show/ImdbTvConfigurationView.h"
#include "ui/scrapers/tv_show/TheTvDbConfigurationView.h"
#include "ui/scrapers/tv_show/TmdbTvConfigurationView.h"
#include "ui/scrapers/tv_show/TvMazeConfigurationView.h"

namespace mediaelch {

ScraperManager::ScraperManager(Settings& settings, QObject* parent) : QObject(parent), m_settings{settings}
{
    initMovieScrapers();
    initTvScrapers();
    initConcertScrapers();
    initMusicScrapers();
    initImageProviders();
}

ScraperManager::~ScraperManager() = default;

QVector<mediaelch::scraper::MovieScraper*> ScraperManager::movieScrapers()
{
    QVector<mediaelch::scraper::MovieScraper*> movieScrapers;
    for (auto& entry : m_scraperMovies) {
        if (entry.scraper() != nullptr) {
            movieScrapers << entry.scraper();
        }
    }
    return movieScrapers;
}

mediaelch::scraper::MovieScraper* ScraperManager::movieScraper(const QString& identifier)
{
    MediaElch_Debug_Assert(identifier != "");
    for (auto& scraper : asConst(m_scraperMovies)) {
        if (scraper.scraper()->meta().identifier == identifier) {
            return scraper.scraper();
        }
    }
    if (identifier != "images.fanarttv") { // TODO
        qCDebug(generic) << "[ScraperManager] No scraper with ID:" << identifier;
    }
    return nullptr;
}

mediaelch::ScraperConfiguration* ScraperManager::movieScraperConfig(const QString& identifier)
{
    MediaElch_Debug_Assert(identifier != "");
    for (auto& scraper : asConst(m_scraperMovies)) {
        if (scraper.scraper()->meta().identifier == identifier) {
            return scraper.config();
        }
    }
    if (identifier != "images.fanarttv") { // TODO
        qCDebug(generic) << "[ScraperManager] No scraper with ID:" << identifier;
    }
    return nullptr;
}

scraper::ConcertScraper* ScraperManager::concertScraper(const QString& identifier)
{
    for (auto& scraper : asConst(m_scraperConcert)) {
        if (scraper.scraper()->meta().identifier == identifier) {
            return scraper.scraper();
        }
    }

    return nullptr;
}

QVector<mediaelch::scraper::TvScraper*> ScraperManager::tvScrapers()
{
    QVector<mediaelch::scraper::TvScraper*> tvScrapers;
    for (auto& entry : m_scraperTv) {
        if (entry.scraper() != nullptr) {
            tvScrapers << entry.scraper();
        }
    }
    return tvScrapers;
}

mediaelch::scraper::TvScraper* ScraperManager::tvScraper(const QString& identifier)
{
    for (auto& scraper : asConst(m_scraperTv)) {
        if (scraper.scraper()->meta().identifier == identifier) {
            return scraper.scraper();
        }
    }
    return nullptr;
}

mediaelch::ScraperConfiguration* ScraperManager::tvScraperConfig(const QString& identifier)
{
    for (auto& scraper : asConst(m_scraperTv)) {
        if (scraper.scraper()->meta().identifier == identifier) {
            return scraper.config();
        }
    }
    return nullptr;
}

QVector<mediaelch::scraper::ConcertScraper*> ScraperManager::concertScrapers()
{
    QVector<mediaelch::scraper::ConcertScraper*> concertScrapers;
    for (auto& entry : m_scraperConcert) {
        if (entry.scraper() != nullptr) {
            concertScrapers << entry.scraper();
        }
    }
    return concertScrapers;
}

mediaelch::ScraperConfiguration* ScraperManager::concertScraperConfig(const QString& identifier)
{
    for (auto& scraper : asConst(m_scraperConcert)) {
        if (scraper.scraper()->meta().identifier == identifier) {
            return scraper.config();
        }
    }
    return nullptr;
}

QVector<mediaelch::scraper::MusicScraper*> ScraperManager::musicScrapers()
{
    QVector<mediaelch::scraper::MusicScraper*> musicScrapers;
    for (auto& entry : m_scraperMusic) {
        if (entry.scraper() != nullptr) {
            musicScrapers << entry.scraper();
        }
    }
    return musicScrapers;
}

mediaelch::ScraperConfiguration* ScraperManager::musicScraperConfig(const QString& identifier)
{
    for (auto& scraper : asConst(m_scraperMusic)) {
        if (scraper.scraper()->meta().identifier == identifier) {
            return scraper.config();
        }
    }
    return nullptr;
}

void ScraperManager::initMovieScrapers()
{
    using namespace mediaelch::scraper;

    {
        ManagedMovieScraper tmdb;
        auto config = std::make_unique<TmdbMovieConfiguration>(m_settings);
        config->init();
        tmdb.m_scraper = std::make_unique<TmdbMovie>(*config, nullptr);
        tmdb.m_viewFactory = [tmdbConfig = config.get()]() { return new TmdbMovieConfigurationView(*tmdbConfig); };
        tmdb.m_config = std::move(config);

        m_scraperMovies.push_back(std::move(tmdb));
    }

    {
        ManagedMovieScraper imdb;
        auto config = std::make_unique<ImdbMovieConfiguration>(m_settings);
        config->init();
        imdb.m_scraper = std::make_unique<ImdbMovie>(*config, nullptr);
        imdb.m_viewFactory = [imdbConfig = config.get()]() { return new ImdbMovieConfigurationView(*imdbConfig); };
        imdb.m_config = std::move(config);

        m_scraperMovies.push_back(std::move(imdb));
    }

    {
        ManagedMovieScraper videoBuster;
        auto config = std::make_unique<ScraperConfigurationStub>(VideoBuster::ID, m_settings);
        config->init();
        videoBuster.m_scraper = std::make_unique<VideoBuster>(nullptr);
        videoBuster.m_config = std::move(config);

        m_scraperMovies.push_back(std::move(videoBuster));
    }

    // Adult Movie Scrapers
    {
        ManagedMovieScraper hotMovies;
        auto config = std::make_unique<ScraperConfigurationStub>(HotMovies::ID, m_settings);
        config->init();
        hotMovies.m_scraper = std::make_unique<HotMovies>(nullptr);
        hotMovies.m_config = std::move(config);

        m_scraperMovies.push_back(std::move(hotMovies));
    }
    {
        ManagedMovieScraper ade;
        auto config = std::make_unique<ScraperConfigurationStub>(AdultDvdEmpire::ID, m_settings);
        config->init();
        ade.m_scraper = std::make_unique<AdultDvdEmpire>(nullptr);
        ade.m_config = std::move(config);

        m_scraperMovies.push_back(std::move(ade));
    }
    {
        ManagedMovieScraper aebn;
        auto aebnConfig = std::make_unique<AebnConfiguration>(m_settings);
        aebnConfig->init();
        aebn.m_scraper = std::make_unique<AEBN>(*aebnConfig, nullptr);
        aebn.m_viewFactory = [aebnConfig = aebnConfig.get()]() { return new AebnConfigurationView(*aebnConfig); };
        aebn.m_config = std::move(aebnConfig);

        m_scraperMovies.push_back(std::move(aebn));
    }

    { // Custom Movie Scraper
        ManagedMovieScraper custom;
        auto config = std::make_unique<CustomMovieScraperConfiguration>(m_settings);
        config->init();
        custom.m_scraper = std::make_unique<CustomMovieScraper>(*config, m_settings, nullptr);
        custom.m_config = std::move(config);

        m_scraperMovies.push_back(std::move(custom));
    }
}

void ScraperManager::initTvScrapers()
{
    using namespace mediaelch::scraper;

    TmdbTv* tmdbPtr = nullptr;
    ImdbTv* imdbPtr = nullptr;

    {
        ManagedTvScraper tmdb;
        auto tmdbConfig = std::make_unique<TmdbTvConfiguration>(m_settings);
        tmdbConfig->init();
        tmdb.m_scraper = std::make_unique<TmdbTv>(*tmdbConfig, nullptr);
        tmdb.m_viewFactory = [tmdbConfig = tmdbConfig.get()]() { //
            return new TmdbTvConfigurationView(*tmdbConfig);
        };
        tmdb.m_config = std::move(tmdbConfig);

        tmdbPtr = dynamic_cast<TmdbTv*>(tmdb.scraper());
        MediaElch_Assert(tmdbPtr != nullptr);

        m_scraperTv.push_back(std::move(tmdb));
    }
    {
        ManagedTvScraper theTvDb;
        auto theTvDbConfig = std::make_unique<TheTvDbConfiguration>(m_settings);
        theTvDbConfig->init();
        theTvDb.m_scraper = std::make_unique<TheTvDb>(*theTvDbConfig, nullptr);
        theTvDb.m_viewFactory = [config = theTvDbConfig.get()]() { //
            return new TheTvDbConfigurationView(*config);
        };
        theTvDb.m_config = std::move(theTvDbConfig);

        m_scraperTv.push_back(std::move(theTvDb));
    }
    {
        ManagedTvScraper imdbTv;
        auto imdbTvConfig = std::make_unique<ImdbTvConfiguration>(m_settings);
        imdbTvConfig->init();
        imdbTv.m_scraper = std::make_unique<ImdbTv>(*imdbTvConfig, nullptr);
        imdbTv.m_viewFactory = [config = imdbTvConfig.get()]() { //
            return new ImdbTvConfigurationView(*config);
        };
        imdbTv.m_config = std::move(imdbTvConfig);

        imdbPtr = dynamic_cast<ImdbTv*>(imdbTv.scraper());
        MediaElch_Assert(imdbPtr != nullptr);

        m_scraperTv.push_back(std::move(imdbTv));
    }
    {
        ManagedTvScraper tvMaze;
        auto tvMazeConfig = std::make_unique<TvMazeConfiguration>(m_settings);
        tvMazeConfig->init();
        tvMaze.m_scraper = std::make_unique<TvMaze>(*tvMazeConfig, nullptr);
        tvMaze.m_viewFactory = [config = tvMazeConfig.get()]() { //
            return new TvMazeConfigurationView(*config);
        };
        tvMaze.m_config = std::move(tvMazeConfig);

        m_scraperTv.push_back(std::move(tvMaze));
    }
    {
        ManagedTvScraper fernsehserienDe;
        auto fernsehserienDeConfig = std::make_unique<FernsehserienDeConfiguration>(m_settings);
        fernsehserienDeConfig->init();
        fernsehserienDe.m_scraper = std::make_unique<FernsehserienDe>(*fernsehserienDeConfig, nullptr);
        fernsehserienDe.m_viewFactory = [config = fernsehserienDeConfig.get()]() {
            return new FernsehserienDeConfigurationView(*config);
        };
        fernsehserienDe.m_config = std::move(fernsehserienDeConfig);

        m_scraperTv.push_back(std::move(fernsehserienDe));
    }


    for (auto& managed : asConst(m_scraperTv)) {
        auto* scraper = managed.scraper();

        qCInfo(generic) << "[TvScraper] Initializing" << scraper->meta().name;
        connect(scraper, &scraper::TvScraper::initialized, this, [](bool wasSuccessful, scraper::TvScraper* tv) {
            if (wasSuccessful) {
                qCInfo(generic) << "[TvScraper] Initialized:" << tv->meta().name;
            } else {
                qCWarning(generic) << "[TvScraper] Initialization failed:" << tv->meta().name;
            }
        });
        scraper->initialize();
    }

    {
        // Only add the Custom TV scraper after the previous ones were added
        // since the constructor explicitly requires them.
        // TODO: Use detail->scraper maps
        ManagedTvScraper custom;
        auto customConfig = std::make_unique<CustomTvScraperConfiguration>(m_settings,
            *tmdbPtr,
            *imdbPtr,
            CustomTvScraperConfiguration::ScraperForShowDetails{},
            CustomTvScraperConfiguration::ScraperForEpisodeDetails{});
        customConfig->init();
        custom.m_scraper = std::make_unique<CustomTvScraper>(*customConfig, nullptr);
        custom.m_config = std::move(customConfig);

        m_scraperTv.push_back(std::move(custom));
    }
}

void ScraperManager::initConcertScrapers()
{
    using namespace mediaelch::scraper;

    {
        ManagedConcertScraper tmdbConcert;
        auto config = std::make_unique<TmdbConcertConfiguration>(m_settings);
        config->init();
        tmdbConcert.m_scraper = std::make_unique<TmdbConcert>(nullptr);
        tmdbConcert.m_viewFactory = [tmdbConfig = config.get()]() {
            return new TmdbConcertConfigurationView(*tmdbConfig);
        };
        tmdbConcert.m_config = std::move(config);

        m_scraperConcert.push_back(std::move(tmdbConcert));
    }
}

void ScraperManager::initMusicScrapers()
{
    using namespace mediaelch::scraper;
    {
        ManagedMusicScraper universal;
        auto config = std::make_unique<UniversalMusicConfiguration>(m_settings);
        config->init();
        universal.m_scraper = std::make_unique<UniversalMusicScraper>(*config, nullptr);
        universal.m_viewFactory = [universalConfig = config.get()]() {
            return new UniversalMusicConfigurationView(*universalConfig);
        };
        universal.m_config = std::move(config);

        m_scraperMusic.push_back(std::move(universal));
    }
    {
        ManagedMusicScraper discogs;
        auto config = std::make_unique<ScraperConfigurationStub>(Discogs::ID, m_settings);
        config->init();
        discogs.m_scraper = std::make_unique<Discogs>(nullptr);
        discogs.m_config = std::move(config);

        m_scraperMusic.push_back(std::move(discogs));
    }
}

void ScraperManager::initImageProviders()
{
    using namespace mediaelch::scraper;

    {
        ManagedImageProvider provider;
        auto providerConfig = std::make_unique<FanartTvConfiguration>(m_settings);
        providerConfig->init();
        provider.m_scraper = std::make_unique<FanartTv>(*providerConfig, nullptr);
        provider.m_viewFactory = [config = providerConfig.get()]() { return new FanartTvConfigurationView(*config); };
        provider.m_config = std::move(providerConfig);

        m_imageProviders.push_back(std::move(provider));
    }
    {
        ManagedImageProvider provider;
        auto providerConfig = std::make_unique<FanartTvConfiguration>(m_settings);
        providerConfig->init();
        provider.m_scraper = std::make_unique<FanartTvMusic>(*providerConfig, nullptr);
        provider.m_viewFactory = [config = providerConfig.get()]() { return new FanartTvConfigurationView(*config); };
        provider.m_config = std::move(providerConfig);

        m_imageProviders.push_back(std::move(provider));
    }
    {
        ManagedImageProvider provider;
        auto providerConfig = std::make_unique<FanartTvConfiguration>(m_settings);
        providerConfig->init();
        provider.m_scraper = std::make_unique<FanartTvMusicArtists>(*providerConfig, nullptr);
        provider.m_viewFactory = [config = providerConfig.get()]() { return new FanartTvConfigurationView(*config); };
        provider.m_config = std::move(providerConfig);

        m_imageProviders.push_back(std::move(provider));
    }
    {
        ManagedImageProvider provider;
        auto providerConfig = std::make_unique<ScraperConfigurationStub>(TmdbImages::ID, m_settings);
        providerConfig->init();
        provider.m_scraper = std::make_unique<TmdbImages>(nullptr);
        provider.m_config = std::move(providerConfig);

        m_imageProviders.push_back(std::move(provider));
    }
    {
        ManagedImageProvider provider;
        auto providerConfig = std::make_unique<ScraperConfigurationStub>(TheTvDbImages::ID, m_settings);
        providerConfig->init();
        provider.m_scraper = std::make_unique<TheTvDbImages>(nullptr);
        provider.m_config = std::move(providerConfig);

        m_imageProviders.push_back(std::move(provider));
    }
}

const std::vector<ManagedMovieScraper>& ScraperManager::allMovieScrapers()
{
    return m_scraperMovies;
}

const std::vector<ManagedTvScraper>& ScraperManager::allTvScrapers()
{
    return m_scraperTv;
}

const std::vector<ManagedConcertScraper>& ScraperManager::allConcertScrapers()
{
    return m_scraperConcert;
}

const std::vector<ManagedMusicScraper>& ScraperManager::allMusicScrapers()
{
    return m_scraperMusic;
}

mediaelch::scraper::ImageProvider* ScraperManager::imageProvider(const QString& identifier)
{
    MediaElch_Debug_Assert(identifier != "");
    for (auto& scraper : asConst(m_imageProviders)) {
        if (scraper.scraper()->meta().identifier == identifier) {
            return scraper.scraper();
        }
    }
    return nullptr;
}

QVector<mediaelch::scraper::ImageProvider*> ScraperManager::imageProviders()
{
    QVector<mediaelch::scraper::ImageProvider*> imageProviders;
    for (auto& entry : m_imageProviders) {
        if (entry.scraper() != nullptr) {
            imageProviders << entry.scraper();
        }
    }
    return imageProviders;
}

QVector<mediaelch::scraper::ImageProvider*> ScraperManager::imageProviders(ImageType type)
{
    QVector<mediaelch::scraper::ImageProvider*> providers;
    for (auto& provider : asConst(m_imageProviders)) {
        if (provider.scraper()->meta().supportedImageTypes.contains(type)) {
            providers.append(provider.scraper());
        }
    }
    return providers;
}

const std::vector<ManagedImageProvider>& ScraperManager::allImageProviders()
{
    return m_imageProviders;
}

mediaelch::ScraperConfiguration* ScraperManager::imageProviderConfig(const QString& identifier)
{
    for (auto& scraper : asConst(m_imageProviders)) {
        if (scraper.scraper()->meta().identifier == identifier) {
            return scraper.config();
        }
    }
    return nullptr;
}

scraper::CustomMovieScraper& ScraperManager::customMovieScraper()
{
    auto* scraper = movieScraper(mediaelch::scraper::CustomMovieScraper::ID);
    auto* custom = dynamic_cast<scraper::CustomMovieScraper*>(scraper);
    MediaElch_Assert(custom != nullptr);
    return *custom;
}

} // namespace mediaelch
