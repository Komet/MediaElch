#include "CustomMovieScraper.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "data/Storage.h"
#include "globals/Manager.h"
#include "globals/NetworkReplyWatcher.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/movie/TMDb.h"
#include "settings/Settings.h"

CustomMovieScraper::CustomMovieScraper(QObject* parent)
{
    setParent(parent);
    m_scrapers = Manager::constructNativeScrapers(this);
    for (MovieScraperInterface* scraper : m_scrapers) {
        connect(scraper, &MovieScraperInterface::searchDone, this, &CustomMovieScraper::onTitleSearchDone);
    }
}

QNetworkAccessManager* CustomMovieScraper::qnam()
{
    return &m_qnam;
}

CustomMovieScraper* CustomMovieScraper::instance(QObject* parent)
{
    static CustomMovieScraper* m_instance = nullptr;
    if (m_instance == nullptr) {
        m_instance = new CustomMovieScraper(parent);
    }
    return m_instance;
}

QString CustomMovieScraper::name() const
{
    return tr("Custom Movie Scraper");
}

QString CustomMovieScraper::identifier() const
{
    return scraperIdentifier;
}

bool CustomMovieScraper::isAdult() const
{
    return false;
}

void CustomMovieScraper::search(QString searchStr)
{
    auto* scraper = scraperForInfo(MovieScraperInfos::Title);
    if (scraper == nullptr) {
        // @todo Better error handling. Currently there is no way to tell the scraper window that something has failed
        qWarning() << "[CustomMovieScraper] Abort search: no valid scraper found for title information";
        ScraperSearchResult errorResult;
        errorResult.name =
            tr("The custom movie scraper is not configured correctly. Please go to settings and reconfigure it.");
        emit searchDone({errorResult}, {});
        return;
    }
    scraper->search(searchStr);
}

void CustomMovieScraper::onTitleSearchDone(QVector<ScraperSearchResult> results, ScraperSearchError error)
{
    if (error.hasError()) {
        emit searchDone({}, error);
        return;
    }

    auto* scraper = dynamic_cast<MovieScraperInterface*>(QObject::sender());
    if (scraper == nullptr) {
        qCritical() << "[CustomMovieScraper] onTitleSearchDone: dynamic_cast failed";
        emit searchDone({}, {ScraperSearchError::ErrorType::InternalError, tr("Internal Error: Please report!")});
        return;
    }

    if (scraper == scraperForInfo(MovieScraperInfos::Title)) {
        emit searchDone(results, error);
    }
}

QVector<MovieScraperInterface*> CustomMovieScraper::scrapersNeedSearch(QVector<MovieScraperInfos> infos,
    QMap<MovieScraperInterface*, QString> alreadyLoadedIds)
{
    QVector<MovieScraperInterface*> scrapers;
    MovieScraperInterface* titleScraper = scraperForInfo(MovieScraperInfos::Title);
    if (titleScraper == nullptr) {
        return scrapers;
    }

    bool imdbIdAvailable = false;
    QMapIterator<MovieScraperInterface*, QString> it(alreadyLoadedIds);
    while (it.hasNext()) {
        it.next();
        if (it.key()->identifier() == IMDB::scraperIdentifier || it.key()->identifier() == TMDb::scraperIdentifier) {
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
        if (scraper->identifier() == TMDb::scraperIdentifier && imdbIdAvailable) {
            continue;
        }
        if (scraper->identifier() == IMDB::scraperIdentifier && imdbIdAvailable) {
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
                    if (scraper->identifier() == IMDB::scraperIdentifier
                        || scraper->identifier() == TMDb::scraperIdentifier) {
                        shouldLoad = true;
                    }
                }
                if (!shouldLoad) {
                    // add tmdb
                    for (auto* scraper : m_scrapers) {
                        if (scraper->identifier() == TMDb::scraperIdentifier) {
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

void CustomMovieScraper::loadData(QMap<MovieScraperInterface*, QString> ids,
    Movie* movie,
    QVector<MovieScraperInfos> infos)
{
    movie->clear(infos);

    QString tmdbId;
    QString imdbId;
    QMapIterator<MovieScraperInterface*, QString> it(ids);
    while (it.hasNext()) {
        it.next();
        if (it.key()->identifier() == TMDb::scraperIdentifier) {
            movie->setTmdbId(TmdbId(it.value()));
            tmdbId = it.value();
        } else if (it.key()->identifier() == IMDB::scraperIdentifier) {
            movie->setId(ImdbId(it.value()));
            imdbId = it.value();
        }
    }

    bool needImdbId = false;
    for (const auto info : infos) {
        MovieScraperInterface* scraper = scraperForInfo(info);
        if (scraper == nullptr) {
            continue;
        }
        if (scraper->identifier() == IMDB::scraperIdentifier) {
            needImdbId = true;
            break;
        }
    }

    if (needImdbId && imdbId.isEmpty()) {
        if (!TmdbId(tmdbId).isValid()) {
            qWarning() << "[CustomMovieScraper] Invalid id: can't scrape movie with tmdb id:" << tmdbId;
            movie->controller()->scraperLoadDone(this);
            return;
        }
        QNetworkRequest request;
        request.setRawHeader("Accept", "application/json");
        QUrl url(QString("https://api.themoviedb.org/3/movie/%1?api_key=%2").arg(tmdbId).arg(TMDb::apiKey()));
        request.setUrl(url);
        QNetworkReply* reply = qnam()->get(request);
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("movie", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        reply->setProperty("ids", Storage::toVariant(reply, ids));
        reply->setProperty("tmdbId", tmdbId);
        connect(reply, &QNetworkReply::finished, this, &CustomMovieScraper::onLoadTmdbFinished);
        return;
    }
    loadAllData(ids, movie, infos, tmdbId, imdbId);
}

void CustomMovieScraper::onLoadTmdbFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* movie = reply->property("movie").value<Storage*>()->movie();
    QVector<MovieScraperInfos> infos = reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad();
    QMap<MovieScraperInterface*, QString> ids = reply->property("ids").value<Storage*>()->ids();
    QString tmdbId = reply->property("tmdbId").toString();

    if (reply->error() == QNetworkReply::NoError) {
        QString imdbId;
        QJsonParseError parseError{};
        const auto parsedJson = QJsonDocument::fromJson(reply->readAll(), &parseError).object();
        reply->deleteLater();
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "Error parsing TMDb json " << parseError.errorString();
            return;
        }
        if (!parsedJson.value("imdb_id").toString().isEmpty()) {
            imdbId = parsedJson.value("imdb_id").toString();

        } else {
            qWarning() << "No IMDB id available";
            movie->controller()->scraperLoadDone(this);
            return;
        }
        loadAllData(ids, movie, infos, tmdbId, imdbId);

    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error" << reply->errorString();
        movie->controller()->scraperLoadDone(this);
        reply->deleteLater();
    }
}

void CustomMovieScraper::loadAllData(QMap<MovieScraperInterface*, QString> ids,
    Movie* movie,
    QVector<MovieScraperInfos> infos,
    QString tmdbId,
    QString imdbId)
{
    QMap<MovieScraperInterface*, QString> scrapersWithIds;
    for (const auto info : infos) {
        auto* scraper = scraperForInfo(info);
        if (scraper == nullptr) {
            continue;
        }

        if (scrapersWithIds.contains(scraper)) {
            continue;
        }
        if (scraper->identifier() == TMDb::scraperIdentifier) {
            scrapersWithIds.insert(scraper, tmdbId.isEmpty() ? imdbId : tmdbId);
        } else if (scraper->identifier() == IMDB::scraperIdentifier) {
            scrapersWithIds.insert(scraper, imdbId);
        } else {
            scrapersWithIds.insert(scraper, ids.value(scraper));
        }
    }

    int loads = scrapersWithIds.count();
    movie->controller()->setProperty("customMovieScraperLoads", loads);
    movie->controller()->setProperty("isCustomScraper", true);

    if (infos.contains(MovieScraperInfos::Backdrop)
        && Settings::instance()->customMovieScraper().value(MovieScraperInfos::Backdrop) == "images.fanarttv") {
        movie->controller()->setForceFanartBackdrop(true);
    }
    if (infos.contains(MovieScraperInfos::Poster)
        && Settings::instance()->customMovieScraper().value(MovieScraperInfos::Poster) == "images.fanarttv") {
        movie->controller()->setForceFanartPoster(true);
    }
    if (infos.contains(MovieScraperInfos::ClearArt)
        && Settings::instance()->customMovieScraper().value(MovieScraperInfos::ClearArt) == "images.fanarttv") {
        movie->controller()->setForceFanartClearArt(true);
    }
    if (infos.contains(MovieScraperInfos::CdArt)
        && Settings::instance()->customMovieScraper().value(MovieScraperInfos::CdArt) == "images.fanarttv") {
        movie->controller()->setForceFanartCdArt(true);
    }
    if (infos.contains(MovieScraperInfos::Logo)
        && Settings::instance()->customMovieScraper().value(MovieScraperInfos::Logo) == "images.fanarttv") {
        movie->controller()->setForceFanartLogo(true);
    }

    QMapIterator<MovieScraperInterface*, QString> itS(scrapersWithIds);
    while (itS.hasNext()) {
        itS.next();
        QVector<MovieScraperInfos> infosToLoad = infosForScraper(itS.key(), infos);
        if (infosToLoad.isEmpty()) {
            continue;
        }
        QMap<MovieScraperInterface*, QString> subIds;
        subIds.insert(nullptr, itS.value());
        itS.key()->loadData(subIds, movie, infosToLoad);
    }
}

MovieScraperInterface* CustomMovieScraper::scraperForInfo(MovieScraperInfos info)
{
    QString identifier = Settings::instance()->customMovieScraper().value(info, "");
    if (identifier.isEmpty()) {
        return nullptr;
    }
    for (auto* scraper : m_scrapers) {
        if (scraper->identifier() == identifier) {
            if (scraper->hasSettings()) {
                ScraperSettingsQt scraperSettings(*scraper, *Settings::instance()->settings());
                scraper->loadSettings(scraperSettings);
            }
            return scraper;
        }
    }
    return nullptr;
}

QVector<MovieScraperInterface*> CustomMovieScraper::scrapersForInfos(QVector<MovieScraperInfos> infos)
{
    QVector<MovieScraperInterface*> scrapers;
    for (const auto info : infos) {
        MovieScraperInterface* scraper = scraperForInfo(info);
        if ((scraper != nullptr) && !scrapers.contains(scraper)) {
            scrapers.append(scraper);
        }
    }

    for (MovieScraperInterface* scraper : scrapers) {
        if (scraper->hasSettings()) {
            ScraperSettingsQt scraperSettings(*scraper, *Settings::instance()->settings());
            scraper->loadSettings(scraperSettings);
        }
    }

    return scrapers;
}

QVector<MovieScraperInfos> CustomMovieScraper::infosForScraper(MovieScraperInterface* scraper,
    QVector<MovieScraperInfos> selectedInfos)
{
    QVector<MovieScraperInfos> infosForScraper;
    for (const auto info : selectedInfos) {
        if (scraper == scraperForInfo(info)) {
            infosForScraper.append(info);
        }
    }
    return infosForScraper;
}

QVector<MovieScraperInfos> CustomMovieScraper::scraperSupports()
{
    QVector<MovieScraperInfos> supports;
    QMapIterator<MovieScraperInfos, QString> it(Settings::instance()->customMovieScraper());
    while (it.hasNext()) {
        it.next();
        if (!it.value().isEmpty()) {
            supports << it.key();
        }
    }
    return supports;
}

MovieScraperInterface* CustomMovieScraper::titleScraper()
{
    return scraperForInfo(MovieScraperInfos::Title);
}

QVector<MovieScraperInfos> CustomMovieScraper::scraperNativelySupports()
{
    return scraperSupports();
}

std::vector<ScraperLanguage> CustomMovieScraper::supportedLanguages()
{
    // Note: This scraper is handled in a special way.
    return {{tr("Unknown"), "en"}};
}

void CustomMovieScraper::changeLanguage(QString /*languageKey*/)
{
    // no-op
}

QString CustomMovieScraper::defaultLanguageKey()
{
    return QStringLiteral("");
}

bool CustomMovieScraper::hasSettings() const
{
    return false;
}

void CustomMovieScraper::loadSettings(const ScraperSettings& settings)
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
