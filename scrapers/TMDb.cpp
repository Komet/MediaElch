#include "TMDb.h"

#include <QDebug>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include <QSettings>

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "settings/Settings.h"

/**
 * @brief TMDb::TMDb
 * @param parent
 */
TMDb::TMDb(QObject *parent)
{
    setParent(parent);
    m_apiKey = "5d832bdf69dcb884922381ab01548d5b";
    m_language = "en";

    m_scraperSupports << MovieScraperInfos::Title
                      << MovieScraperInfos::Tagline
                      << MovieScraperInfos::Rating
                      << MovieScraperInfos::Released
                      << MovieScraperInfos::Runtime
                      << MovieScraperInfos::Certification
                      << MovieScraperInfos::Trailer
                      << MovieScraperInfos::Overview
                      << MovieScraperInfos::Poster
                      << MovieScraperInfos::Backdrop
                      << MovieScraperInfos::Actors
                      << MovieScraperInfos::Genres
                      << MovieScraperInfos::Studios
                      << MovieScraperInfos::Countries
                      << MovieScraperInfos::Director
                      << MovieScraperInfos::Writer
                      << MovieScraperInfos::ExtraArts;

    m_baseUrl = "http://cf2.imgobject.com/t/p/";
    setup();
}

TMDb::~TMDb()
{
}

/**
 * @brief languages
 * @return
 */
QMap<QString, QString> TMDb::languages()
{
    QMap<QString, QString> m;

    m.insert(tr("Bulgarian"), "bg");
    m.insert(tr("Chinese"), "zh");
    m.insert(tr("Croatian"), "hr");
    m.insert(tr("Czech"), "cs");
    m.insert(tr("Danish"), "da");
    m.insert(tr("Dutch"), "nl");
    m.insert(tr("English"), "en");
    m.insert(tr("Finnish"), "fi");
    m.insert(tr("French"), "fr");
    m.insert(tr("German"), "de");
    m.insert(tr("Greek"), "el");
    m.insert(tr("Hebrew"), "he");
    m.insert(tr("Hungarian"), "hu");
    m.insert(tr("Italian"), "it");
    m.insert(tr("Japanese"), "ja");
    m.insert(tr("Korean"), "ko");
    m.insert(tr("Norwegian"), "no");
    m.insert(tr("Polish"), "pl");
    m.insert(tr("Portuguese"), "pt");
    m.insert(tr("Russian"), "ru");
    m.insert(tr("Slovene"), "sl");
    m.insert(tr("Spanish"), "es");
    m.insert(tr("Swedish"), "sv");
    m.insert(tr("Turkish"), "tr");

    return m;
}

/**
 * @brief language
 * @return
 */
QString TMDb::language()
{
    return m_language;
}

/**
 * @brief Returns the name of the scraper
 * @return Name of the Scraper
 */
QString TMDb::name()
{
    return QString("The Movie DB");
}

/**
 * @brief Returns if the scraper has settings
 * @return Scraper has settings
 */
bool TMDb::hasSettings()
{
    return true;
}

/**
 * @brief Loads scrapers settings
 */
void TMDb::loadSettings(QSettings &settings)
{
    m_language = settings.value("Scrapers/TMDb/Language", "en").toString();
}

/**
 * @brief Saves scrapers settings
 */
void TMDb::saveSettings(QSettings &settings)
{
    settings.setValue("Scrapers/TMDb/Language", m_language);
}

/**
 * @brief TMDb::setLanguage
 * @param language
 */
void TMDb::setLanguage(QString language)
{
    m_language = language;
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager *TMDb::qnam()
{
    return &m_qnam;
}

/**
 * @brief Returns a list of infos available from the scraper
 * @return List of supported infos
 */
QList<int> TMDb::scraperSupports()
{
    return m_scraperSupports;
}

/**
 * @brief Loads the setup parameters from TMDb
 * @see TMDb::setupFinished
 */
void TMDb::setup()
{
    QUrl url(QString("http://api.themoviedb.org/3/configuration?api_key=%1").arg(m_apiKey));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(setupFinished()));
}

/**
 * @brief Called when setup parameters were got
 *        Parses json and assigns the baseUrl
 */
void TMDb::setupFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    if (reply->error() != QNetworkReply::NoError ) {
        reply->deleteLater();
        return;
    }
    QString msg = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(msg) + ")");

    m_baseUrl = sc.property("images").property("base_url").toString();
}

/**
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @see TMDb::searchFinished
 */
void TMDb::search(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    searchStr = searchStr.replace("-", " ");
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrl url(QString("http://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&query=%3").arg(m_apiKey).arg(m_language).arg(encodedSearch));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply *reply = qnam()->get(request);
    reply->setProperty("searchString", searchStr);
    reply->setProperty("results", Storage::toVariant(reply, QList<ScraperSearchResult>()));
    connect(reply, SIGNAL(finished()), this, SLOT(searchFinished()));
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "searchDone" if there are no more pages in the result set
 * @see TMDb::parseSearch
 */
void TMDb::searchFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    QList<ScraperSearchResult> results = reply->property("results").value<Storage*>()->results();

    if (reply->error() != QNetworkReply::NoError ) {
        qWarning() << "Network Error" << reply->errorString();
        reply->deleteLater();
        emit searchDone(results);
        return;
    }

    QString searchString = reply->property("searchString").toString();
    QString msg = QString::fromUtf8(reply->readAll());
    int nextPage = -1;
    results.append(parseSearch(msg, &nextPage));
    reply->deleteLater();

    if (nextPage == -1) {
        emit searchDone(results);
    } else {
        QUrl url(QString("http://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&page=%3&query=%4").arg(m_apiKey).arg(m_language).arg(nextPage).arg(searchString));
        QNetworkRequest request(url);
        request.setRawHeader("Accept", "application/json");
        QNetworkReply *reply = qnam()->get(request);
        reply->setProperty("searchString", searchString);
        reply->setProperty("results", Storage::toVariant(reply, results));
        connect(reply, SIGNAL(finished()), this, SLOT(searchFinished()));
    }
}

/**
 * @brief Parses the JSON search results
 * @param json JSON string
 * @param nextPage This will hold the next page to get, -1 if there are no more pages
 * @return List of search results
 */
QList<ScraperSearchResult> TMDb::parseSearch(QString json, int *nextPage)
{
    qDebug() << "Entered";
    QList<ScraperSearchResult> results;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    // only get the first 3 pages
    if (sc.property("page").toInteger() < sc.property("total_pages").toInteger() && sc.property("page").toInteger() < 3)
        *nextPage = sc.property("page").toInteger()+1;

    if (sc.property("results").isArray() ) {
        QScriptValueIterator it(sc.property("results"));
        while (it.hasNext() ) {
            it.next();
            if (it.value().property("id").toString().isEmpty()) {
                continue;
            }
            ScraperSearchResult result;
            result.name     = it.value().property("title").toString();
            if (result.name.isEmpty())
                it.value().property("original_title").toString();
            result.id       = it.value().property("id").toString();
            result.released = QDate::fromString(it.value().property("release_date").toString(), "yyyy-MM-dd");
            results.append(result);
        }
    }

    return results;
}

/**
 * @brief Starts network requests to download infos from TMDb
 * @param id TMDb movie ID
 * @param movie Movie object
 * @param infos List of infos to load
 * @see TMDb::loadFinished
 * @see TMDb::loadCastsFinished
 * @see TMDb::loadTrailersFinished
 * @see TMDb::loadImagesFinished
 * @see TMDb::loadReleasesFinished
 */
void TMDb::loadData(QString id, Movie *movie, QList<int> infos)
{
    qDebug() << "Entered, id=" << id << "movie=" << movie->name();
    movie->setTmdbId(id);
    movie->clear(infos);

    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");

    QList<ScraperData> loadsLeft;

    // Infos
    loadsLeft.append(DataInfos);
    url.setUrl(QString("http://api.themoviedb.org/3/movie/%1?api_key=%2&language=%3").arg(id).arg(m_apiKey).arg(m_language));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(loadFinished()));

    // Casts
    if (infos.contains(MovieScraperInfos::Actors) ||
        infos.contains(MovieScraperInfos::Director) ||
        infos.contains(MovieScraperInfos::Writer)) {
        loadsLeft.append(DataCasts);
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/casts?api_key=%2").arg(id).arg(m_apiKey));
        request.setUrl(url);
        QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, SIGNAL(finished()), this, SLOT(loadCastsFinished()));
    }

    // Trailers
    if (infos.contains(MovieScraperInfos::Trailer)) {
        loadsLeft.append(DataTrailers);
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/trailers?api_key=%2&language=%3").arg(id).arg(m_apiKey).arg(m_language));
        request.setUrl(url);
        QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, SIGNAL(finished()), this, SLOT(loadTrailersFinished()));
    }

    // Images
    if (infos.contains(MovieScraperInfos::Poster) || infos.contains(MovieScraperInfos::Backdrop)) {
        loadsLeft.append(DataImages);
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/images?api_key=%2").arg(id).arg(m_apiKey));
        request.setUrl(url);
        QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, SIGNAL(finished()), this, SLOT(loadImagesFinished()));
    }

    // Releases
    if (infos.contains(MovieScraperInfos::Certification)) {
        loadsLeft.append(DataReleases);
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/releases?api_key=%2").arg(id).arg(m_apiKey));
        request.setUrl(url);
        QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, SIGNAL(finished()), this, SLOT(loadReleasesFinished()));
    }
    movie->controller()->setLoadsLeft(loadsLeft);
}

/**
 * @brief Called when the movie infos are downloaded
 * @see TMDb::parseAndAssignInfos
 */
void TMDb::loadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }
    movie->controller()->removeFromLoadsLeft(DataInfos);
}

/**
 * @brief Called when the movie casts are downloaded
 * @see TMDb::parseAndAssignInfos
 */
void TMDb::loadCastsFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        qWarning() << "Network Error (casts)" << reply->errorString();
    }
    movie->controller()->removeFromLoadsLeft(DataCasts);
}

/**
 * @brief Called when the movie trailers are downloaded
 * @see TMDb::parseAndAssignInfos
 */
void TMDb::loadTrailersFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        qDebug() << "Network Error (trailers)" << reply->errorString();
    }
    movie->controller()->removeFromLoadsLeft(DataTrailers);
}

/**
 * @brief Called when the movie images are downloaded
 * @see TMDb::parseAndAssignInfos
 */
void TMDb::loadImagesFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        qWarning() << "Network Error (images)" << reply->errorString();
    }
    movie->controller()->removeFromLoadsLeft(DataImages);
}

/**
 * @brief Called when the movie releases are downloaded
 * @see TMDb::parseAndAssignInfos
 */
void TMDb::loadReleasesFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        qWarning() << "Network Error (releases)" << reply->errorString();
    }
    movie->controller()->removeFromLoadsLeft(DataReleases);
}

/**
 * @brief Parses JSON data and assigns it to the given movie object
 *        Handles all types of data from TMDb (info, releases, trailers, casts, images)
 * @param json JSON data
 * @param movie Movie object
 * @param infos List of infos to load
 */
void TMDb::parseAndAssignInfos(QString json, Movie *movie, QList<int> infos)
{
    qDebug() << "Entered";
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    // Infos
    if (sc.property("imdb_id").isValid() && !sc.property("imdb_id").toString().isEmpty())
        movie->setId(sc.property("imdb_id").toString());
    if (infos.contains(MovieScraperInfos::Title) && sc.property("title").isValid())
        movie->setName(sc.property("title").toString());
    if (infos.contains(MovieScraperInfos::Title) && sc.property("original_title").isValid())
        movie->setOriginalName(sc.property("original_title").toString());
    if (infos.contains(MovieScraperInfos::Overview) && sc.property("overview").isValid() && !sc.property("overview").isNull()) {
        movie->setOverview(sc.property("overview").toString());
        if (Settings::instance()->usePlotForOutline())
            movie->setOutline(sc.property("overview").toString());
    }
    if (infos.contains(MovieScraperInfos::Rating) && sc.property("vote_average").isValid())
        movie->setRating(sc.property("vote_average").toNumber());
    if (infos.contains(MovieScraperInfos::Rating) && sc.property("vote_count").isValid())
        movie->setVotes(sc.property("vote_count").toInteger());
    if (infos.contains(MovieScraperInfos::Tagline) && sc.property("tagline").isValid() && !sc.property("tagline").isNull())
       movie->setTagline(sc.property("tagline").toString());
    if (infos.contains(MovieScraperInfos::Released) && sc.property("release_date").isValid())
        movie->setReleased(QDate::fromString(sc.property("release_date").toString(), "yyyy-MM-dd"));
    if (infos.contains(MovieScraperInfos::Runtime) && sc.property("runtime").isValid())
        movie->setRuntime(sc.property("runtime").toInteger());
    if (infos.contains(MovieScraperInfos::Genres) && sc.property("genres").isArray()) {
        QScriptValueIterator itC(sc.property("genres"));
        while (itC.hasNext()) {
            itC.next();
            QScriptValue vC = itC.value();
            if (vC.property("id").toString().isEmpty())
                continue;
            movie->addGenre(vC.property("name").toString());
        }
    }
    if (infos.contains(MovieScraperInfos::Studios) && sc.property("production_companies").isArray()) {
        QScriptValueIterator itS(sc.property("production_companies"));
        while (itS.hasNext()) {
            itS.next();
            QScriptValue vS = itS.value();
            if (vS.property("id").toString().isEmpty())
                continue;
            movie->addStudio(vS.property("name").toString());
        }
    }
    if (infos.contains(MovieScraperInfos::Countries) && sc.property("production_countries").isArray()) {
        QScriptValueIterator itC(sc.property("production_countries"));
        while (itC.hasNext()) {
            itC.next();
            QScriptValue vC = itC.value();
            if (vC.property("name").toString().isEmpty())
                continue;
            movie->addCountry(vC.property("name").toString());
        }
    }
    if (infos.contains(MovieScraperInfos::Title) && sc.property("belongs_to_collection").isValid()) {
        if (!sc.property("belongs_to_collection").property("name").toString().isEmpty())
            movie->setSet(sc.property("belongs_to_collection").property("name").toString());
    }

    // Casts
    if (infos.contains(MovieScraperInfos::Actors) && sc.property("cast").isArray()) {
        QScriptValueIterator itC(sc.property("cast"));
        while (itC.hasNext()) {
            itC.next();
            QScriptValue vC = itC.value();
            if (vC.property("name").toString().isEmpty())
                continue;
            Actor a;
            a.name = vC.property("name").toString();
            a.role = vC.property("character").toString();
            if (!vC.property("profile_path").isNull())
                a.thumb = m_baseUrl + "original" + vC.property("profile_path").toString();
            movie->addActor(a);
        }
    }

    // Crew
    if ((infos.contains(MovieScraperInfos::Director) || infos.contains(MovieScraperInfos::Writer)) && sc.property("crew").isArray()) {
        QScriptValueIterator itC(sc.property("crew"));
        while (itC.hasNext()) {
            itC.next();
            QScriptValue vC = itC.value();
            if (vC.property("name").toString().isEmpty())
                continue;
            if (infos.contains(MovieScraperInfos::Writer) && vC.property("department").toString() == "Writing") {
                QString writer = movie->writer();
                if (writer.contains(vC.property("name").toString()))
                    continue;
                if (!writer.isEmpty())
                    writer.append(", ");
                writer.append(vC.property("name").toString());
                movie->setWriter(writer);
            }
            if (infos.contains(MovieScraperInfos::Director) && vC.property("job").toString() == "Director" && vC.property("department").toString() == "Directing")
                movie->setDirector(vC.property("name").toString());
        }
    }

    // Trailers
    if (infos.contains(MovieScraperInfos::Trailer) && sc.property("youtube").isArray()) {
        QScriptValueIterator itC(sc.property("youtube"));
        while (itC.hasNext()) {
            itC.next();
            QScriptValue vC = itC.value();
            if (vC.property("source").toString().isEmpty())
                continue;
            movie->setTrailer(QUrl(Helper::formatTrailerUrl(QString("http://www.youtube.com/watch?v=%1").arg(vC.property("source").toString()))));
            break;
        }
    }

    // Images
    if (infos.contains(MovieScraperInfos::Backdrop) && sc.property("backdrops").isArray()) {
        QScriptValueIterator itB(sc.property("backdrops"));
        while (itB.hasNext()) {
            itB.next();
            QScriptValue vB = itB.value();
            if (vB.property("file_path").toString().isEmpty())
                continue;
            Poster b;
            b.thumbUrl = m_baseUrl + "w780" + vB.property("file_path").toString();
            b.originalUrl = m_baseUrl + "original" + vB.property("file_path").toString();
            b.originalSize.setWidth(vB.property("width").toString().toInt());
            b.originalSize.setHeight(vB.property("height").toString().toInt());
            movie->addBackdrop(b);
        }
    }

    if (infos.contains(MovieScraperInfos::Poster) && sc.property("posters").isArray()) {
        QScriptValueIterator itB(sc.property("posters"));
        while (itB.hasNext()) {
            itB.next();
            QScriptValue vB = itB.value();
            if (vB.property("file_path").toString().isEmpty())
                continue;
            Poster b;
            b.thumbUrl = m_baseUrl + "w342" + vB.property("file_path").toString();
            b.originalUrl = m_baseUrl + "original" + vB.property("file_path").toString();
            b.originalSize.setWidth(vB.property("width").toString().toInt());
            b.originalSize.setHeight(vB.property("height").toString().toInt());
            b.language = vB.property("iso_639_1").toString();
            bool primaryLang = (b.language==m_language);
            movie->addPoster(b,primaryLang);
        }
    }

    // Releases
    if (infos.contains(MovieScraperInfos::Certification) && sc.property("countries").isArray()) {
        QString locale;
        QString us;
        QString gb;
        QScriptValueIterator itB(sc.property("countries"));
        while (itB.hasNext()) {
            itB.next();
            QScriptValue vB = itB.value();
            if (vB.property("iso_3166_1").toString() == "US")
                us = vB.property("certification").toString();
            if (vB.property("iso_3166_1").toString() == "GB")
                gb = vB.property("certification").toString();
            if (vB.property("iso_3166_1").toString().toLower() == m_language)
                locale = vB.property("certification").toString();
        }
        if (!locale.isEmpty())
            movie->setCertification(locale);
        else if (!us.isEmpty())
            movie->setCertification(us);
        else if (!gb.isEmpty())
            movie->setCertification(gb);
    }

}
