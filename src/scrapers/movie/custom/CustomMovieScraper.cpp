#include "CustomMovieScraper.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "data/Storage.h"
#include "globals/Manager.h"
#include "globals/ScraperManager.h"
#include "scrapers/movie/custom/CustomMovieScrapeJob.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "settings/Settings.h"


namespace mediaelch {
namespace scraper {

CustomMovieScraper::CustomMovieScraper(QObject* parent) : MovieScraper(parent)
{
    m_meta.identifier = ID;
    m_meta.name = tr("Custom Movie Scraper");
    m_meta.description = "";
    m_meta.website = "";
    m_meta.termsOfService = "";
    m_meta.privacyPolicy = "";
    m_meta.help = "";
    m_meta.supportedDetails = allMovieScraperInfos();
    // Note: This scraper is handled in a special way.
    m_meta.supportedLanguages = {mediaelch::Locale::English};
    m_meta.defaultLocale = mediaelch::Locale::English;
    m_meta.isAdult = false;

    m_scrapers = mediaelch::ScraperManager::constructNativeScrapers(this);
}

mediaelch::network::NetworkManager* CustomMovieScraper::network()
{
    return &m_network;
}

CustomMovieScraper* CustomMovieScraper::instance(QObject* parent)
{
    static CustomMovieScraper* m_instance = new CustomMovieScraper(parent);
    return m_instance;
}

const MovieScraper::ScraperMeta& CustomMovieScraper::meta() const
{
    return m_meta;
}

void CustomMovieScraper::initialize()
{
    // TODO
}

bool CustomMovieScraper::isInitialized() const
{
    // TODO
    return true;
}

MovieSearchJob* CustomMovieScraper::search(MovieSearchJob::Config config)
{
    auto* scraper = scraperForInfo(MovieScraperInfo::Title);
    if (scraper == nullptr) {
        // always use TMDb just in case
        scraper = Manager::instance()->scrapers().movieScraper(TmdbMovie::ID);
    }
    Q_ASSERT(scraper != nullptr);
    return scraper->search(std::move(config));
}

MovieScrapeJob* CustomMovieScraper::loadMovie(MovieScrapeJob::Config config)
{
    return new CustomMovieScrapeJob(std::move(config), this);
    //    movie->clear(infos);

    //    TmdbId tmdbId;
    //    ImdbId imdbId;
    //    QHashIterator<MovieScraper*, MovieIdentifier> it(ids);
    //    while (it.hasNext()) {
    //        it.next();
    //        if (it.key()->meta().identifier == TmdbMovie::ID) {
    //            tmdbId = TmdbId(it.value().str());
    //            movie->setTmdbId(tmdbId);

    //        } else if (it.key()->meta().identifier == ImdbMovie::ID) {
    //            imdbId = ImdbId(it.value().str());
    //            movie->setImdbId(imdbId);
    //        }
    //    }

    //    bool needImdbId = false;
    //    for (const auto info : infos) {
    //        MovieScraper* scraper = scraperForInfo(info);
    //        if (scraper == nullptr) {
    //            continue;
    //        }
    //        if (scraper->meta().identifier == ImdbMovie::ID) {
    //            needImdbId = true;
    //            break;
    //        }
    //    }

    //    if (needImdbId && !imdbId.isValid()) {
    //        if (!tmdbId.isValid()) {
    //            qWarning() << "[CustomMovieScraper] Invalid id: can't scrape movie with TMDb id:" << tmdbId;
    //            ScraperError error;
    //            error.error = ScraperError::Type::ConfigError;
    //            error.message = tr("TMDb ID is invalid. Cannot scrape movie.");
    //            error.technical = QStringLiteral("Invalid id: can't scrape movie with TMDb id:
    //            %1").arg(tmdbId.toString()); movie->controller()->scraperLoadDone(this, error); return;
    //        }
    //        QNetworkRequest request;
    //        request.setRawHeader("Accept", "application/json");
    //        QUrl url(
    //            QString("https://api.themoviedb.org/3/movie/%1?api_key=%2").arg(tmdbId.toString()).arg(TmdbApi::apiKey()));
    //        request.setUrl(url);
    //        QNetworkReply* reply = network()->getWithWatcher(request);
    //        reply->setProperty("movie", Storage::toVariant(reply, movie));
    //        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    //        reply->setProperty("ids", Storage::toVariant(reply, ids));
    //        reply->setProperty("tmdbId", tmdbId.toString());
    //        connect(reply, &QNetworkReply::finished, this, &CustomMovieScraper::onLoadTmdbFinished);
    //        return;
    //    }
    //    loadAllData(ids, movie, infos, tmdbId, imdbId);
    return nullptr;
}

QVector<MovieScraper*> CustomMovieScraper::scrapersNeedSearch(QSet<MovieScraperInfo> infos,
    QHash<MovieScraper*, MovieIdentifier> alreadyLoadedIds)
{
    QVector<MovieScraper*> scrapers;
    MovieScraper* titleScraper = scraperForInfo(MovieScraperInfo::Title);
    if (titleScraper == nullptr) {
        return scrapers;
    }

    bool imdbIdAvailable = false;
    QHashIterator<MovieScraper*, MovieIdentifier> it(alreadyLoadedIds);
    while (it.hasNext()) {
        it.next();
        if (it.key()->meta().identifier == ImdbMovie::ID || it.key()->meta().identifier == TmdbMovie::ID) {
            imdbIdAvailable = true;
        }
    }

    for (auto* scraper : scrapersForInfos(infos)) {
        if (scraper == titleScraper) {
            continue;
        }
        if (scrapers.contains(scraper)) {
            continue;
        }
        if (scraper->meta().identifier == TmdbMovie::ID && imdbIdAvailable) {
            continue;
        }
        if (scraper->meta().identifier == ImdbMovie::ID && imdbIdAvailable) {
            continue;
        }
        if (alreadyLoadedIds.contains(scraper)) {
            continue;
        }
        scrapers.append(scraper);
    }

    for (const auto info : infos) {
        QString imageProviderId = Settings::instance()->customMovieScraper().value(info, "notset");
        if (imageProviderId == "images.fanarttv") {
            if (!imdbIdAvailable) {
                // check if imdb id should be loaded
                bool shouldLoad = false;
                for (const auto* scraper : scrapers) {
                    if (scraper->meta().identifier == ImdbMovie::ID || scraper->meta().identifier == TmdbMovie::ID) {
                        shouldLoad = true;
                    }
                }
                if (!shouldLoad) {
                    // add tmdb
                    for (auto* scraper : m_scrapers) {
                        if (scraper->meta().identifier == TmdbMovie::ID) {
                            scrapers.append(scraper);
                            break;
                        }
                    }
                }
            }
            break;
        }
    }

    return scrapers;
}

void CustomMovieScraper::onLoadTmdbFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* movie = reply->property("movie").value<Storage*>()->movie();
    QSet<MovieScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad();
    QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids = reply->property("ids").value<Storage*>()->ids();
    TmdbId tmdbId(reply->property("tmdbId").toString());

    if (reply->error() == QNetworkReply::NoError) {
        ImdbId imdbId;
        QJsonParseError parseError{};
        const auto parsedJson = QJsonDocument::fromJson(reply->readAll(), &parseError).object();
        reply->deleteLater();
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "Error parsing TMDb json " << parseError.errorString();
            return;
        }
        if (!parsedJson.value("imdb_id").toString().isEmpty()) {
            imdbId = ImdbId(parsedJson.value("imdb_id").toString());

        } else {
            qWarning() << "No IMDB id available";
            movie->controller()->scraperLoadDone(this, {}); // silent error
            return;
        }
        loadAllData(ids, movie, infos, tmdbId, imdbId);

    } else {
        // TODO  movie->controller()->scraperLoadDone(this, mediaelch::replyToScraperError(*reply));
        reply->deleteLater();
    }
}

void CustomMovieScraper::loadAllData(QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
    Movie* movie,
    const QSet<MovieScraperInfo>& infos,
    TmdbId tmdbId,
    ImdbId imdbId)
{
    QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> scrapersWithIds;
    for (const auto info : infos) {
        auto* scraper = scraperForInfo(info);
        if (scraper == nullptr) {
            continue;
        }

        if (scrapersWithIds.contains(scraper)) {
            continue;
        }
        QString id;
        if (scraper->meta().identifier == TmdbMovie::ID) {
            id = !tmdbId.isValid() ? imdbId.toString() : tmdbId.toString();
        } else if (scraper->meta().identifier == ImdbMovie::ID) {
            id = imdbId.toString();
        } else {
            id = ids.value(scraper).str();
        }
        scrapersWithIds.insert(scraper, MovieIdentifier(id));
    }

    int loads = scrapersWithIds.count();
    movie->controller()->setProperty("customMovieScraperLoads", loads);
    movie->controller()->setProperty("isCustomScraper", true);

    if (infos.contains(MovieScraperInfo::Backdrop)
        && Settings::instance()->customMovieScraper().value(MovieScraperInfo::Backdrop) == "images.fanarttv") {
        movie->controller()->setForceFanartBackdrop(true);
    }
    if (infos.contains(MovieScraperInfo::Poster)
        && Settings::instance()->customMovieScraper().value(MovieScraperInfo::Poster) == "images.fanarttv") {
        movie->controller()->setForceFanartPoster(true);
    }
    if (infos.contains(MovieScraperInfo::ClearArt)
        && Settings::instance()->customMovieScraper().value(MovieScraperInfo::ClearArt) == "images.fanarttv") {
        movie->controller()->setForceFanartClearArt(true);
    }
    if (infos.contains(MovieScraperInfo::CdArt)
        && Settings::instance()->customMovieScraper().value(MovieScraperInfo::CdArt) == "images.fanarttv") {
        movie->controller()->setForceFanartCdArt(true);
    }
    if (infos.contains(MovieScraperInfo::Logo)
        && Settings::instance()->customMovieScraper().value(MovieScraperInfo::Logo) == "images.fanarttv") {
        movie->controller()->setForceFanartLogo(true);
    }

    QHashIterator<MovieScraper*, MovieIdentifier> itS(scrapersWithIds);
    while (itS.hasNext()) {
        itS.next();
        QSet<MovieScraperInfo> infosToLoad = infosForScraper(itS.key(), infos);
        if (infosToLoad.isEmpty()) {
            continue;
        }
        QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> subIds;
        subIds.insert(nullptr, itS.value());
        // TODO   itS.key()->loadData(subIds, movie, infosToLoad);
    }
}

MovieScraper* CustomMovieScraper::scraperForInfo(MovieScraperInfo info)
{
    QString identifier = Settings::instance()->customMovieScraper().value(info, "");
    if (identifier.isEmpty()) {
        return nullptr;
    }
    for (auto* scraper : m_scrapers) {
        if (scraper->meta().identifier == identifier) {
            if (scraper->hasSettings()) {
                ScraperSettingsQt scraperSettings(scraper->meta().identifier, *Settings::instance()->settings());
                scraper->loadSettings(scraperSettings);
            }
            return scraper;
        }
    }
    return nullptr;
}

QVector<MovieScraper*> CustomMovieScraper::scrapersForInfos(QSet<MovieScraperInfo> infos)
{
    QVector<MovieScraper*> scrapers;
    for (const auto info : infos) {
        MovieScraper* scraper = scraperForInfo(info);
        if ((scraper != nullptr) && !scrapers.contains(scraper)) {
            scrapers.append(scraper);
        }
    }

    for (MovieScraper* scraper : scrapers) {
        if (scraper->hasSettings()) {
            ScraperSettingsQt scraperSettings(scraper->meta().identifier, *Settings::instance()->settings());
            scraper->loadSettings(scraperSettings);
        }
    }

    return scrapers;
}

QSet<MovieScraperInfo> CustomMovieScraper::infosForScraper(MovieScraper* scraper, QSet<MovieScraperInfo> selectedInfos)
{
    QSet<MovieScraperInfo> infosForScraper;
    for (const auto info : selectedInfos) {
        if (scraper == scraperForInfo(info)) {
            infosForScraper.insert(info);
        }
    }
    return infosForScraper;
}

MovieScraper* CustomMovieScraper::titleScraper()
{
    return scraperForInfo(MovieScraperInfo::Title);
}

QSet<MovieScraperInfo> CustomMovieScraper::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void CustomMovieScraper::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op
}

bool CustomMovieScraper::hasSettings() const
{
    return false;
}

void CustomMovieScraper::loadSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

void CustomMovieScraper::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QWidget* CustomMovieScraper::settingsWidget()
{
    return nullptr;
}

} // namespace scraper
} // namespace mediaelch
