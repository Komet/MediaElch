#include "TMDb.h"

#include <QDebug>
#include <QLabel>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include <QSettings>
#include <QGridLayout>

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/NetworkReplyWatcher.h"
#include "main/MainWindow.h"
#include "settings/Settings.h"

/**
 * @brief TMDb::TMDb
 * @param parent
 */
TMDb::TMDb(QObject *parent)
{
    setParent(parent);
    m_language = "en";

    m_widget = new QWidget(MainWindow::instance());
    m_box = new QComboBox(m_widget);
    m_box->addItem(tr("Bulgarian"), "bg");
    m_box->addItem(tr("Chinese"), "zh");
    m_box->addItem(tr("Croatian"), "hr");
    m_box->addItem(tr("Czech"), "cs");
    m_box->addItem(tr("Danish"), "da");
    m_box->addItem(tr("Dutch"), "nl");
    m_box->addItem(tr("English"), "en");
    m_box->addItem(tr("English (US)"), "en_US");
    m_box->addItem(tr("Finnish"), "fi");
    m_box->addItem(tr("French"), "fr");
    m_box->addItem(tr("German"), "de");
    m_box->addItem(tr("Greek"), "el");
    m_box->addItem(tr("Hebrew"), "he");
    m_box->addItem(tr("Hungarian"), "hu");
    m_box->addItem(tr("Italian"), "it");
    m_box->addItem(tr("Japanese"), "ja");
    m_box->addItem(tr("Korean"), "ko");
    m_box->addItem(tr("Norwegian"), "no");
    m_box->addItem(tr("Polish"), "pl");
    m_box->addItem(tr("Portuguese"), "pt");
    m_box->addItem(tr("Russian"), "ru");
    m_box->addItem(tr("Slovene"), "sl");
    m_box->addItem(tr("Spanish"), "es");
    m_box->addItem(tr("Swedish"), "sv");
    m_box->addItem(tr("Turkish"), "tr");
    QGridLayout *layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);

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
                      << MovieScraperInfos::Logo
                      << MovieScraperInfos::Banner
                      << MovieScraperInfos::Thumb
                      << MovieScraperInfos::CdArt
                      << MovieScraperInfos::ClearArt
                      << MovieScraperInfos::Set;

    m_scraperNativelySupports << MovieScraperInfos::Title
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
                              << MovieScraperInfos::Set;

    m_baseUrl = "http://cf2.imgobject.com/t/p/";
    setup();
}

TMDb::~TMDb()
{
}

QString TMDb::apiKey()
{
    return "5d832bdf69dcb884922381ab01548d5b";
}

/**
 * @brief Returns the name of the scraper
 * @return Name of the Scraper
 */
QString TMDb::name()
{
    return QString("The Movie DB");
}

QString TMDb::identifier()
{
    return QString("tmdb");
}

bool TMDb::isAdult()
{
    return false;
}

/**
 * @brief Returns if the scraper has settings
 * @return Scraper has settings
 */
bool TMDb::hasSettings()
{
    return true;
}

QWidget *TMDb::settingsWidget()
{
    return m_widget;
}

/**
 * @brief Loads scrapers settings
 */
void TMDb::loadSettings(QSettings &settings)
{
    QString lang = settings.value("Scrapers/TMDb/Language", "en").toString();
    m_language = lang;
    m_language2 = "";
    if (m_language.split("_").count() > 1) {
        m_language = lang.split("_").at(0);
        m_language2 = lang.split("_").at(1);
    }

    bool found = false;
    for (int i=0, n=m_box->count() ; i<n ; ++i) {
        if (m_box->itemData(i).toString() == m_language + "_" + m_language2) {
            m_box->setCurrentIndex(i);
            found = true;
        }
        if (m_box->itemData(i).toString() == m_language && !found) {
            m_box->setCurrentIndex(i);
            found = true;
        }
    }
}

/**
 * @brief Saves scrapers settings
 */
void TMDb::saveSettings(QSettings &settings)
{
    QString language;
    language = m_box->itemData(m_box->currentIndex()).toString();
    if (language.split("_").count() > 1) {
        m_language = language.split("_").at(0);
        m_language2 = language.split("_").at(1);
    }
    settings.setValue("Scrapers/TMDb/Language", language);
    loadSettings(settings);
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

QList<int> TMDb::scraperNativelySupports()
{
    return m_scraperNativelySupports;
}

/**
 * @brief Loads the setup parameters from TMDb
 * @see TMDb::setupFinished
 */
void TMDb::setup()
{
    QUrl url(QString("http://api.themoviedb.org/3/configuration?api_key=%1").arg(TMDb::apiKey()));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply *reply = qnam()->get(request);
    new NetworkReplyWatcher(this, reply);
    connect(reply, SIGNAL(finished()), this, SLOT(setupFinished()));
}

/**
 * @brief Called when setup parameters were got
 *        Parses json and assigns the baseUrl
 */
void TMDb::setupFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    if (reply->error() != QNetworkReply::NoError) {
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
    QString searchTitle;
    QString searchYear;
    QUrl url;
    QString includeAdult = (Settings::instance()->showAdultScrapers()) ? "true" : "false";
    QRegExp rx("^tt\\d+$");
    QRegExp rxTmdbId("^id\\d+$");
    if (rx.exactMatch(searchStr)) {
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1?api_key=%2&language=%3&include_adult=%4")
                   .arg(searchStr)
                   .arg(TMDb::apiKey())
                   .arg(m_language)
                   .arg(includeAdult));
    } else if (rxTmdbId.exactMatch(searchStr)) {
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1?api_key=%2&language=%3&include_adult=%4")
                   .arg(searchStr.mid(2))
                   .arg(TMDb::apiKey())
                   .arg(m_language)
                   .arg(includeAdult));
    } else {
        url.setUrl(QString("http://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&include_adult=%3&query=%4")
                   .arg(TMDb::apiKey())
                   .arg(m_language)
                   .arg(includeAdult)
                   .arg(encodedSearch));
        QList<QRegExp> rxYears;
        rxYears << QRegExp("^(.*) \\((\\d{4})\\)$") << QRegExp("^(.*) (\\d{4})$") << QRegExp("^(.*) - (\\d{4})$");
        foreach (QRegExp rxYear, rxYears) {
            rxYear.setMinimal(true);
            if (rxYear.exactMatch(searchStr)) {
                searchTitle = rxYear.cap(1);
                searchYear = rxYear.cap(2);
                url.setUrl(QString("http://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&include_adult=%3&year=%4&query=%5")
                           .arg(TMDb::apiKey())
                           .arg(m_language)
                           .arg(includeAdult)
                           .arg(searchYear)
                           .arg(QString(QUrl::toPercentEncoding(searchTitle))));
                break;
            }
        }
    }
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply *reply = qnam()->get(request);
    new NetworkReplyWatcher(this, reply);
    if (!searchTitle.isEmpty() && !searchYear.isEmpty()) {
        reply->setProperty("searchTitle", searchTitle);
        reply->setProperty("searchYear", searchYear);
    }
    reply->setProperty("searchString", searchStr);
    reply->setProperty("results", Storage::toVariant(reply, QList<ScraperSearchResult>()));
    reply->setProperty("page", 1);
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

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        reply->deleteLater();
        emit searchDone(results);
        return;
    }

    QString searchString = reply->property("searchString").toString();
    QString searchTitle = reply->property("searchTitle").toString();
    QString searchYear = reply->property("searchYear").toString();
    int page = reply->property("page").toInt();
    QString msg = QString::fromUtf8(reply->readAll());
    int nextPage = -1;
    results.append(parseSearch(msg, &nextPage, page));
    reply->deleteLater();

    if (nextPage == -1) {
        emit searchDone(results);
    } else {
        QUrl url(QString("http://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&page=%3&query=%4").arg(TMDb::apiKey()).arg(m_language).arg(nextPage).arg(searchString));
        if (!searchTitle.isEmpty() && !searchYear.isEmpty())
            url.setUrl(QString("http://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&page=%3&year=%4&query=%5")
                       .arg(TMDb::apiKey()).arg(m_language).arg(nextPage).arg(searchYear).arg(searchTitle));
        QNetworkRequest request(url);
        request.setRawHeader("Accept", "application/json");
        QNetworkReply *reply = qnam()->get(request);
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("searchString", searchString);
        reply->setProperty("results", Storage::toVariant(reply, results));
        reply->setProperty("page", nextPage);
        connect(reply, SIGNAL(finished()), this, SLOT(searchFinished()));
    }
}

/**
 * @brief Parses the JSON search results
 * @param json JSON string
 * @param nextPage This will hold the next page to get, -1 if there are no more pages
 * @return List of search results
 */
QList<ScraperSearchResult> TMDb::parseSearch(QString json, int *nextPage, int page)
{
    qDebug() << "Entered";
    QList<ScraperSearchResult> results;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    // only get the first 3 pages
    if (page < sc.property("total_pages").toInteger() && page < 3)
        *nextPage = page+1;

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
    } else if (!sc.property("id").toString().isEmpty()) {
        ScraperSearchResult result;
        result.name     = sc.property("title").toString();
        if (result.name.isEmpty())
            sc.property("original_title").toString();
        result.id       = sc.property("id").toString();
        result.released = QDate::fromString(sc.property("release_date").toString(), "yyyy-MM-dd");
        results.append(result);
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
void TMDb::loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos)
{
    if (!ids.values().first().startsWith("tt"))
        movie->setTmdbId(ids.values().first());
    movie->clear(infos);

    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");

    QList<ScraperData> loadsLeft;

    // Infos
    loadsLeft.append(DataInfos);
    url.setUrl(QString("http://api.themoviedb.org/3/movie/%1?api_key=%2&language=%3").arg(ids.values().first()).arg(TMDb::apiKey()).arg(m_language));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(loadFinished()));

    // Casts
    if (infos.contains(MovieScraperInfos::Actors) ||
        infos.contains(MovieScraperInfos::Director) ||
        infos.contains(MovieScraperInfos::Writer)) {
        loadsLeft.append(DataCasts);
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/casts?api_key=%2").arg(ids.values().first()).arg(TMDb::apiKey()));
        request.setUrl(url);
        QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, SIGNAL(finished()), this, SLOT(loadCastsFinished()));
    }

    // Trailers
    if (infos.contains(MovieScraperInfos::Trailer)) {
        loadsLeft.append(DataTrailers);
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/trailers?api_key=%2&language=%3").arg(ids.values().first()).arg(TMDb::apiKey()).arg(m_language));
        request.setUrl(url);
        QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, SIGNAL(finished()), this, SLOT(loadTrailersFinished()));
    }

    // Images
    if (infos.contains(MovieScraperInfos::Poster) || infos.contains(MovieScraperInfos::Backdrop)) {
        loadsLeft.append(DataImages);
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/images?api_key=%2").arg(ids.values().first()).arg(TMDb::apiKey()));
        request.setUrl(url);
        QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, SIGNAL(finished()), this, SLOT(loadImagesFinished()));
    }

    // Releases
    if (infos.contains(MovieScraperInfos::Certification)) {
        loadsLeft.append(DataReleases);
        url.setUrl(QString("http://api.themoviedb.org/3/movie/%1/releases?api_key=%2").arg(ids.values().first()).arg(TMDb::apiKey()));
        request.setUrl(url);
        QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
        new NetworkReplyWatcher(this, reply);
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

    if (reply->error() == QNetworkReply::NoError) {
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

    if (reply->error() == QNetworkReply::NoError) {
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

    if (reply->error() == QNetworkReply::NoError) {
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

    if (reply->error() == QNetworkReply::NoError) {
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

    if (reply->error() == QNetworkReply::NoError) {
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
    if (infos.contains(MovieScraperInfos::Set) && !sc.property("belongs_to_collection").toString().isEmpty())
        movie->setSet(sc.property("belongs_to_collection").property("name").toString());
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
            movie->addGenre(Helper::instance()->mapGenre(vC.property("name").toString()));
        }
    }
    if (infos.contains(MovieScraperInfos::Studios) && sc.property("production_companies").isArray()) {
        QScriptValueIterator itS(sc.property("production_companies"));
        while (itS.hasNext()) {
            itS.next();
            QScriptValue vS = itS.value();
            if (vS.property("id").toString().isEmpty())
                continue;
            movie->addStudio(Helper::instance()->mapStudio(vS.property("name").toString()));
        }
    }
    if (infos.contains(MovieScraperInfos::Countries) && sc.property("production_countries").isArray()) {
        QScriptValueIterator itC(sc.property("production_countries"));
        while (itC.hasNext()) {
            itC.next();
            QScriptValue vC = itC.value();
            if (vC.property("name").toString().isEmpty())
                continue;
            movie->addCountry(Helper::instance()->mapCountry(vC.property("name").toString()));
        }
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
            movie->setTrailer(QUrl(Helper::instance()->formatTrailerUrl(QString("http://www.youtube.com/watch?v=%1").arg(vC.property("source").toString()))));
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

        if (m_language2 == "US" && !us.isEmpty())
            movie->setCertification(Helper::instance()->mapCertification(us));
        else if (m_language == "en" && m_language2 == "" && !gb.isEmpty())
            movie->setCertification(Helper::instance()->mapCertification(gb));
        else if (!locale.isEmpty())
            movie->setCertification(Helper::instance()->mapCertification(locale));
        else if (!us.isEmpty())
            movie->setCertification(Helper::instance()->mapCertification(us));
        else if (!gb.isEmpty())
            movie->setCertification(Helper::instance()->mapCertification(gb));
    }

}
