#include "CustomMovieScraper.h"

#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#include "globals/Manager.h"
#include "globals/NetworkReplyWatcher.h"
#include "settings/Settings.h"
#include "data/Storage.h"

CustomMovieScraper::CustomMovieScraper(QObject *parent)
{
    setParent(parent);
    m_scrapers = Manager::constructNativeScrapers(this);
    foreach (ScraperInterface *scraper, m_scrapers)
        connect(scraper, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(onTitleSearchDone(QList<ScraperSearchResult>)));
}

QNetworkAccessManager *CustomMovieScraper::qnam()
{
    return &m_qnam;
}

CustomMovieScraper *CustomMovieScraper::instance(QObject *parent)
{
    static CustomMovieScraper *m_instance = 0;
    if (!m_instance)
        m_instance = new CustomMovieScraper(parent);
    return m_instance;
}

QString CustomMovieScraper::name()
{
    return tr("Custom Movie Scraper");
}

QString CustomMovieScraper::identifier()
{
    return QString("custom-movie");
}

bool CustomMovieScraper::isAdult()
{
    return false;
}

void CustomMovieScraper::search(QString searchStr)
{
    ScraperInterface *scraper = scraperForInfo(MovieScraperInfos::Title);
    if (!scraper)
        return;
    scraper->search(searchStr);
}

void CustomMovieScraper::onTitleSearchDone(QList<ScraperSearchResult> results)
{
    ScraperInterface *scraper = static_cast<ScraperInterface*>(QObject::sender());
    if (!scraper)
        return;

    if (scraper == scraperForInfo(MovieScraperInfos::Title))
        emit searchDone(results);
}

QList<ScraperInterface*> CustomMovieScraper::scrapersNeedSearch(QList<int> infos, QMap<ScraperInterface*, QString> alreadyLoadedIds)
{
    QList<ScraperInterface*> scrapers;
    ScraperInterface *titleScraper = scraperForInfo(MovieScraperInfos::Title);
    if (!titleScraper)
        return scrapers;

    bool imdbIdAvailable = false;
    QMapIterator<ScraperInterface*, QString> it(alreadyLoadedIds);
    while (it.hasNext()) {
        it.next();
        if (it.key()->identifier() == "imdb" || it.key()->identifier() == "tmdb")
            imdbIdAvailable = true;
    }

    foreach (ScraperInterface *scraper, scrapersForInfos(infos)) {
        if (scraper == titleScraper)
            continue;
        if (scrapers.contains(scraper))
            continue;
        if (scraper->identifier() == "tmdb" && imdbIdAvailable)
            continue;
        if (scraper->identifier() == "imdb" && imdbIdAvailable)
            continue;
        if (alreadyLoadedIds.contains(scraper))
            continue;
        scrapers.append(scraper);
    }

    foreach (const int &info, infos) {
        QString imageProviderId = Settings::instance()->customMovieScraper().value(info, "notset");
        if (imageProviderId == "images.fanarttv") {
            if (!imdbIdAvailable) {
                // check if imdb id should be loaded
                bool shouldLoad = false;
                foreach (ScraperInterface *scraper, scrapers) {
                    if (scraper->identifier() == "imdb" || scraper->identifier() == "tmdb")
                        shouldLoad = true;
                }
                if (!shouldLoad) {
                    // add tmdb
                    foreach (ScraperInterface *scraper, m_scrapers) {
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

void CustomMovieScraper::loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos)
{
    movie->clear(infos);

    QString tmdbId;
    QString imdbId;
    QMapIterator<ScraperInterface*, QString> it(ids);
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
    foreach (const int &info, infos) {
        ScraperInterface *scraper = scraperForInfo(info);
        if (!scraper)
            continue;
        if (scraper->identifier() == "imdb") {
            needImdbId = true;
            break;
        }
    }

    if (needImdbId && imdbId.isEmpty()) {
        QNetworkRequest request;
        request.setRawHeader("Accept", "application/json");
        QUrl url(QString("http://api.themoviedb.org/3/movie/%1?api_key=%2").arg(tmdbId).arg(TMDb::apiKey()));
        request.setUrl(url);
        QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("movie", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        reply->setProperty("ids", Storage::toVariant(reply, ids));
        reply->setProperty("tmdbId", tmdbId);
        connect(reply, SIGNAL(finished()), this, SLOT(onLoadTmdbFinished()));
    } else {
        loadAllData(ids, movie, infos, tmdbId, imdbId);
    }
}

void CustomMovieScraper::onLoadTmdbFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("movie").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    QMap<ScraperInterface*, QString> ids = reply->property("ids").value<Storage*>()->ids();
    QString tmdbId = reply->property("tmdbId").toString();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QString imdbId;
        QScriptValue sc;
        QScriptEngine engine;
        sc = engine.evaluate("(" + QString::fromUtf8(reply->readAll()) + ")");
        if (sc.property("imdb_id").isValid() && !sc.property("imdb_id").toString().isEmpty()) {
            imdbId = sc.property("imdb_id").toString();
        } else {
            qWarning() << "No IMDB id available";
            movie->controller()->scraperLoadDone(this);
            return;
        }
        loadAllData(ids, movie, infos, tmdbId, imdbId);
    } else {
        qWarning() << "Network Error" << reply->errorString();
        movie->controller()->scraperLoadDone(this);
    }
}

void CustomMovieScraper::loadAllData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<int> infos, QString tmdbId, QString imdbId)
{
    QMap<ScraperInterface*, QString> scrapersWithIds;
    foreach (const int &info, infos) {
        ScraperInterface *scraper = scraperForInfo(info);
        if (!scraper)
            continue;

        if (scrapersWithIds.contains(scraper))
            continue;
        if (scraper->identifier() == "tmdb")
            scrapersWithIds.insert(scraper, tmdbId.isEmpty() ? imdbId : tmdbId);
        else if (scraper->identifier() == "imdb")
            scrapersWithIds.insert(scraper, imdbId);
        else
            scrapersWithIds.insert(scraper, ids.value(scraper));
    }

    int loads = scrapersWithIds.count();
    movie->controller()->setProperty("customMovieScraperLoads", loads);
    movie->controller()->setProperty("isCustomScraper", true);

    if (infos.contains(MovieScraperInfos::Backdrop) && Settings::instance()->customMovieScraper().value(MovieScraperInfos::Backdrop) == "images.fanarttv")
        movie->controller()->setForceFanartBackdrop(true);
    if (infos.contains(MovieScraperInfos::Poster) && Settings::instance()->customMovieScraper().value(MovieScraperInfos::Poster) == "images.fanarttv")
        movie->controller()->setForceFanartPoster(true);
    if (infos.contains(MovieScraperInfos::ClearArt) && Settings::instance()->customMovieScraper().value(MovieScraperInfos::ClearArt) == "images.fanarttv")
        movie->controller()->setForceFanartClearArt(true);
    if (infos.contains(MovieScraperInfos::CdArt) && Settings::instance()->customMovieScraper().value(MovieScraperInfos::CdArt) == "images.fanarttv")
        movie->controller()->setForceFanartCdArt(true);
    if (infos.contains(MovieScraperInfos::Logo) && Settings::instance()->customMovieScraper().value(MovieScraperInfos::Logo) == "images.fanarttv")
        movie->controller()->setForceFanartLogo(true);

    QMapIterator<ScraperInterface*, QString> itS(scrapersWithIds);
    while (itS.hasNext()) {
        itS.next();
        QList<int> infosToLoad = infosForScraper(itS.key(), infos);
        if (infosToLoad.isEmpty())
            continue;
        QMap<ScraperInterface*, QString> subIds;
        subIds.insert(0, itS.value());
        itS.key()->loadData(subIds, movie, infosToLoad);
    }
}

ScraperInterface *CustomMovieScraper::scraperForInfo(int info)
{
    QString identifier = Settings::instance()->customMovieScraper().value(info, "");
    if (identifier.isEmpty())
        return 0;
    foreach (ScraperInterface *scraper, m_scrapers) {
        if (scraper->identifier() == identifier) {
            if (scraper->hasSettings())
                scraper->loadSettings(*Settings::instance()->settings());
            return scraper;
        }
    }
    return 0;
}

QList<ScraperInterface*> CustomMovieScraper::scrapersForInfos(QList<int> infos)
{
    QList<ScraperInterface*> scrapers;
    foreach (const int &info, infos) {
        ScraperInterface *scraper = scraperForInfo(info);
        if (scraper && !scrapers.contains(scraper))
            scrapers.append(scraper);
    }

    foreach (ScraperInterface *scraper, scrapers) {
        if (scraper->hasSettings())
            scraper->loadSettings(*Settings::instance()->settings());
    }

    return scrapers;
}

QList<int> CustomMovieScraper::infosForScraper(ScraperInterface *scraper, QList<int> selectedInfos)
{
    QList<int> infosForScraper;
    foreach (const int &info, selectedInfos) {
        if (scraper == scraperForInfo(info))
            infosForScraper.append(info);
    }
    return infosForScraper;
}

QList<int> CustomMovieScraper::scraperSupports()
{
    QList<int> supports;
    QMapIterator<int, QString> it(Settings::instance()->customMovieScraper());
    while (it.hasNext()) {
        it.next();
        if (!it.value().isEmpty())
            supports << it.key();
    }
    return supports;
}

ScraperInterface *CustomMovieScraper::titleScraper()
{
    return scraperForInfo(MovieScraperInfos::Title);
}

QList<int> CustomMovieScraper::scraperNativelySupports()
{
    return scraperSupports();
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
    return 0;
}
