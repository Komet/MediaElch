#include "scrapers/tv_show/tmdb/TmdbTv.h"

#include "scrapers/tv_show/tmdb/TmdbTvEpisodeScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTvSeasonScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowSearchJob.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

QString TmdbTv::ID = "tmdbtv";

TmdbTv::TmdbTv(QObject* parent) : TvScraper(parent)
{
    m_meta.identifier = TmdbTv::ID;
    m_meta.name = "TMDb TV";
    m_meta.description = tr("The Movie Database (TMDb) is a community built movie and TV database. "
                            "Every piece of data has been added by our amazing community dating back to 2008. "
                            "TMDb's strong international focus and breadth of data is largely unmatched and "
                            "something we're incredibly proud of. Put simply, we live and breathe community "
                            "and that's precisely what makes us different.");
    m_meta.website = "https://www.themoviedb.org/tv";
    m_meta.termsOfService = "https://www.themoviedb.org/terms-of-use";
    m_meta.privacyPolicy = "https://www.themoviedb.org/privacy-policy";
    m_meta.help = "https://www.themoviedb.org/talk";
    m_meta.supportedShowDetails = {ShowScraperInfo::Title,
        ShowScraperInfo::Overview,
        ShowScraperInfo::Certification,
        ShowScraperInfo::FirstAired,
        ShowScraperInfo::Status,
        ShowScraperInfo::Runtime,
        ShowScraperInfo::Rating,
        ShowScraperInfo::Tags,
        ShowScraperInfo::Poster,
        ShowScraperInfo::Fanart,
        ShowScraperInfo::Genres,
        ShowScraperInfo::Actors,
        ShowScraperInfo::Network,
        ShowScraperInfo::SeasonPoster};
    m_meta.supportedEpisodeDetails = {EpisodeScraperInfo::Actors,
        // EpisodeScraperInfo::Certification,
        EpisodeScraperInfo::Director,
        EpisodeScraperInfo::FirstAired,
        // EpisodeScraperInfo::Network,
        EpisodeScraperInfo::Overview,
        EpisodeScraperInfo::Rating,
        EpisodeScraperInfo::Thumbnail,
        EpisodeScraperInfo::Title,
        EpisodeScraperInfo::Writer};
    m_meta.supportedSeasonOrders = {SeasonOrder::Aired};
    // For officially supported languages, see:
    // https://developers.themoviedb.org/3/configuration/get-primary-translations
    m_meta.supportedLanguages = {"ar-AE",
        "ar-SA",
        "be-BY",
        "bg-BG",
        "bn-BD",
        "ca-ES",
        "ch-GU",
        "cn-CN",
        "cs-CZ",
        "da-DK",
        "de-DE",
        "de-AT",
        "de-CH",
        "el-GR",
        "en-AU",
        "en-CA",
        "en-GB",
        "en-NZ",
        "en-US",
        "eo-EO",
        "es-ES",
        "es-MX",
        "et-EE",
        "eu-ES",
        "fa-IR",
        "fi-FI",
        "fr-CA",
        "fr-FR",
        "gl-ES",
        "he-IL",
        "hi-IN",
        "hu-HU",
        "hr-HR",
        "id-ID",
        "it-IT",
        "ja-JP",
        "ka-GE",
        "kk-KZ",
        "kn-IN",
        "ko-KR",
        "lt-LT",
        "lv-LV",
        "ml-IN",
        "ms-MY",
        "ms-SG",
        "nb-NO",
        "nl-NL",
        "no-NO",
        "pl-PL",
        "pt-BR",
        "pt-PT",
        "ro-RO",
        "ru-RU",
        "si-LK",
        "sk-SK",
        "sl-SI",
        "sq-AL",
        "sr-RS",
        "sv-SE",
        "ta-IN",
        "te-IN",
        "th-TH",
        "tl-PH",
        "tr-TR",
        "uk-UA",
        "vi-VN",
        "zh-CN",
        "zh-HK",
        "zh-TW",
        "zu-ZA"};
    m_meta.defaultLocale = Locale::English;

    connect(&m_api, &TmdbApi::initialized, this, [this](bool wasSuccessful) { emit initialized(wasSuccessful, this); });
}

const TvScraper::ScraperMeta& TmdbTv::meta() const
{
    return m_meta;
}

void TmdbTv::initialize()
{
    m_api.initialize();
}

bool TmdbTv::isInitialized() const
{
    return m_api.isInitialized();
}

ShowSearchJob* TmdbTv::search(ShowSearchJob::Config config)
{
    qInfo() << "[TmdbTv] Search for:" << config.query;
    return new TmdbTvShowSearchJob(m_api, config, this);
}

ShowScrapeJob* TmdbTv::loadShow(ShowScrapeJob::Config config)
{
    qInfo() << "[TmdbTv] Load TV show with id:" << config.identifier;
    return new TmdbTvShowScrapeJob(m_api, config, this);
}

SeasonScrapeJob* TmdbTv::loadSeasons(SeasonScrapeJob::Config config)
{
    qInfo() << "[TmdbTv] Load season with show id:" << config.showIdentifier;
    return new TmdbTvSeasonScrapeJob(m_api, config, this);
}

EpisodeScrapeJob* TmdbTv::loadEpisode(EpisodeScrapeJob::Config config)
{
    qDebug() << "[TmdbTv] Load single episode of TV show with id:" << config.identifier;
    return new TmdbTvEpisodeScrapeJob(m_api, config, this);
}

} // namespace scraper
} // namespace mediaelch
