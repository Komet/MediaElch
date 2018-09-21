#include "CustomMovieScraper.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "data/Storage.h"
#include "globals/Manager.h"
#include "globals/NetworkReplyWatcher.h"
#include "settings/Settings.h"

CustomMovieScraper::CustomMovieScraper(QObject *parent)
{
    setParent(parent);
    m_scrapers = Manager::constructNativeScrapers(this);
    for (ScraperInterface *scraper : m_scrapers) {
        connect(scraper,
            SIGNAL(searchDone(QList<ScraperSearchResult>)),
            this,
            SLOT(onTitleSearchDone(QList<ScraperSearchResult>)));
    }
}

QNetworkAccessManager *CustomMovieScraper::qnam()
{
    return &m_qnam;
}

CustomMovieScraper *CustomMovieScraper::instance(QObject *parent)
{
    static CustomMovieScraper *m_instance = nullptr;
    if (!m_instance) {
        m_instance = new CustomMovieScraper(parent);
    }
    return m_instance;
}

QString CustomMovieScraper::name()
{
    return tr("Custom Movie Scraper");
}

QString CustomMovieScraper::identifier()
{
    return QStringLiteral("custom-movie");
}

bool CustomMovieScraper::isAdult()
{
    return false;
}

void CustomMovieScraper::search(QString searchStr)
{
    ScraperInterface *scraper = scraperForInfo(MovieScraperInfos::Title);
    if (!scraper) {
        return;
    }
    scraper->search(searchStr);
}

void CustomMovieScraper::onTitleSearchDone(QList<ScraperSearchResult> results)
{
    auto scraper = dynamic_cast<ScraperInterface *>(QObject::sender());
    if (!scraper) {
        return;
    }

    if (scraper == scraperForInfo(MovieScraperInfos::Title)) {
        emit searchDone(results);
    }
}

QList<ScraperInterface *> CustomMovieScraper::scrapersNeedSearch(QList<MovieScraperInfos> infos,
    QMap<ScraperInterface *, QString> alreadyLoadedIds)
{
    QList<ScraperInterface *> scrapers;
    ScraperInterface *titleScraper = scraperForInfo(MovieScraperInfos::Title);
    if (!titleScraper) {
        return scrapers;
    }

    bool imdbIdAvailable = false;
    QMapIterator<ScraperInterface *, QString> it(alreadyLoadedIds);
    while (it.hasNext()) {
        it.next();
        if (it.key()->identifier() == "imdb" || it.key()->identifier() == "tmdb") {
            imdbIdAvailable = true;
        }
    }

    for (ScraperInterface *scraper : scrapersForInfos(infos)) {
        if (scraper == titleScraper) {
            continue;
        }
        if (scrapers.contains(scraper)) {
            continue;
        }
        if (scraper->identifier() == "tmdb" && imdbIdAvailable) {
            continue;
        }
        if (scraper->identifier() == "imdb" && imdbIdAvailable) {
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
                for (ScraperInterface *scraper : scrapers) {
                    if (scraper->identifier() == "imdb" || scraper->identifier() == "tmdb") {
                        shouldLoad = true;
                    }
                }
                if (!shouldLoad) {
                    // add tmdb
                    for (ScraperInterface *scraper : m_scrapers) {
                        if (scraper->identifier() == "tmdb") {
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

void CustomMovieScraper::loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<MovieScraperInfos> infos)
{
    movie->clear(infos);

    QString tmdbId;
    QString imdbId;
    QMapIterator<ScraperInterface *, QString> it(ids);
    while (it.hasNext()) {
        it.next();
        if (it.key()->identifier() == "tmdb") {
            movie->setTmdbId(it.value());
            tmdbId = it.value();
        } else if (it.key()->identifier() == "imdb") {
            movie->setId(it.value());
            imdbId = it.value();
        }
    }

    bool needImdbId = false;
    for (const auto info : infos) {
        ScraperInterface *scraper = scraperForInfo(info);
        if (!scraper) {
            continue;
        }
        if (scraper->identifier() == "imdb") {
            needImdbId = true;
            break;
        }
    }

    if (needImdbId && imdbId.isEmpty()) {
        QNetworkRequest request;
        request.setRawHeader("Accept", "application/json");
        QUrl url(QString("https://api.themoviedb.org/3/movie/%1?api_key=%2").arg(tmdbId).arg(TMDb::apiKey()));
        request.setUrl(url);
        QNetworkReply *reply = qnam()->get(request);
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("movie", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        reply->setProperty("ids", Storage::toVariant(reply, ids));
        reply->setProperty("tmdbId", tmdbId);
        connect(reply, &QNetworkReply::finished, this, &CustomMovieScraper::onLoadTmdbFinished);
    } else {
        loadAllData(ids, movie, infos, tmdbId, imdbId);
    }
}

void CustomMovieScraper::onLoadTmdbFinished()
{
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    Movie *movie = reply->property("movie").value<Storage *>()->movie();
    QList<MovieScraperInfos> infos = reply->property("infosToLoad").value<Storage *>()->movieInfosToLoad();
    QMap<ScraperInterface *, QString> ids = reply->property("ids").value<Storage *>()->ids();
    QString tmdbId = reply->property("tmdbId").toString();

    if (reply->error() == QNetworkReply::NoError) {
        QString imdbId;
        QJsonParseError parseError;
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
        qWarning() << "Network Error" << reply->errorString();
        movie->controller()->scraperLoadDone(this);
        reply->deleteLater();
    }
}

void CustomMovieScraper::loadAllData(QMap<ScraperInterface *, QString> ids,
    Movie *movie,
    QList<MovieScraperInfos> infos,
    QString tmdbId,
    QString imdbId)
{
    QMap<ScraperInterface *, QString> scrapersWithIds;
    for (const auto info : infos) {
        ScraperInterface *scraper = scraperForInfo(info);
        if (!scraper) {
            continue;
        }

        if (scrapersWithIds.contains(scraper)) {
            continue;
        }
        if (scraper->identifier() == "tmdb") {
            scrapersWithIds.insert(scraper, tmdbId.isEmpty() ? imdbId : tmdbId);
        } else if (scraper->identifier() == "imdb") {
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

    QMapIterator<ScraperInterface *, QString> itS(scrapersWithIds);
    while (itS.hasNext()) {
        itS.next();
        QList<MovieScraperInfos> infosToLoad = infosForScraper(itS.key(), infos);
        if (infosToLoad.isEmpty()) {
            continue;
        }
        QMap<ScraperInterface *, QString> subIds;
        subIds.insert(nullptr, itS.value());
        itS.key()->loadData(subIds, movie, infosToLoad);
    }
}

ScraperInterface *CustomMovieScraper::scraperForInfo(MovieScraperInfos info)
{
    QString identifier = Settings::instance()->customMovieScraper().value(info, "");
    if (identifier.isEmpty()) {
        return nullptr;
    }
    for (ScraperInterface *scraper : m_scrapers) {
        if (scraper->identifier() == identifier) {
            if (scraper->hasSettings()) {
                scraper->loadSettings(*Settings::instance()->settings());
            }
            return scraper;
        }
    }
    return nullptr;
}

QList<ScraperInterface *> CustomMovieScraper::scrapersForInfos(QList<MovieScraperInfos> infos)
{
    QList<ScraperInterface *> scrapers;
    for (const auto info : infos) {
        ScraperInterface *scraper = scraperForInfo(info);
        if (scraper && !scrapers.contains(scraper)) {
            scrapers.append(scraper);
        }
    }

    for (ScraperInterface *scraper : scrapers) {
        if (scraper->hasSettings()) {
            scraper->loadSettings(*Settings::instance()->settings());
        }
    }

    return scrapers;
}

QList<MovieScraperInfos> CustomMovieScraper::infosForScraper(ScraperInterface *scraper,
    QList<MovieScraperInfos> selectedInfos)
{
    QList<MovieScraperInfos> infosForScraper;
    for (const auto info : selectedInfos) {
        if (scraper == scraperForInfo(info)) {
            infosForScraper.append(info);
        }
    }
    return infosForScraper;
}

QList<MovieScraperInfos> CustomMovieScraper::scraperSupports()
{
    QList<MovieScraperInfos> supports;
    QMapIterator<MovieScraperInfos, QString> it(Settings::instance()->customMovieScraper());
    while (it.hasNext()) {
        it.next();
        if (!it.value().isEmpty()) {
            supports << it.key();
        }
    }
    return supports;
}

ScraperInterface *CustomMovieScraper::titleScraper()
{
    return scraperForInfo(MovieScraperInfos::Title);
}

QList<MovieScraperInfos> CustomMovieScraper::scraperNativelySupports()
{
    return scraperSupports();
}

std::vector<ScraperLanguage> CustomMovieScraper::supportedLanguages()
{
    // Note: This scraper is handled in a special way.
    return {{tr("Unknown"), "en"}};
}

bool CustomMovieScraper::hasSettings()
{
    return false;
}

void CustomMovieScraper::loadSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

void CustomMovieScraper::saveSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

QWidget *CustomMovieScraper::settingsWidget()
{
    return nullptr;
}
