#include "TMDb.h"

#include <QDebug>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QSettings>
#include <QUrlQuery>

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
    auto layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);

    m_scraperSupports << MovieScraperInfos::Title         //
                      << MovieScraperInfos::Tagline       //
                      << MovieScraperInfos::Rating        //
                      << MovieScraperInfos::Released      //
                      << MovieScraperInfos::Runtime       //
                      << MovieScraperInfos::Certification //
                      << MovieScraperInfos::Trailer       //
                      << MovieScraperInfos::Overview      //
                      << MovieScraperInfos::Poster        //
                      << MovieScraperInfos::Backdrop      //
                      << MovieScraperInfos::Actors        //
                      << MovieScraperInfos::Genres        //
                      << MovieScraperInfos::Studios       //
                      << MovieScraperInfos::Countries     //
                      << MovieScraperInfos::Director      //
                      << MovieScraperInfos::Writer        //
                      << MovieScraperInfos::Logo          //
                      << MovieScraperInfos::Banner        //
                      << MovieScraperInfos::Thumb         //
                      << MovieScraperInfos::CdArt         //
                      << MovieScraperInfos::ClearArt      //
                      << MovieScraperInfos::Set;

    m_scraperNativelySupports << MovieScraperInfos::Title         //
                              << MovieScraperInfos::Tagline       //
                              << MovieScraperInfos::Rating        //
                              << MovieScraperInfos::Released      //
                              << MovieScraperInfos::Runtime       //
                              << MovieScraperInfos::Certification //
                              << MovieScraperInfos::Trailer       //
                              << MovieScraperInfos::Overview      //
                              << MovieScraperInfos::Poster        //
                              << MovieScraperInfos::Backdrop      //
                              << MovieScraperInfos::Actors        //
                              << MovieScraperInfos::Genres        //
                              << MovieScraperInfos::Studios       //
                              << MovieScraperInfos::Countries     //
                              << MovieScraperInfos::Director      //
                              << MovieScraperInfos::Writer        //
                              << MovieScraperInfos::Set;

    m_baseUrl = "http://cf2.imgobject.com/t/p/";
    setup();
}

QString TMDb::apiKey()
{
    return QStringLiteral("5d832bdf69dcb884922381ab01548d5b");
}

/**
 * @brief Returns the name of the scraper
 * @return Name of the Scraper
 */
QString TMDb::name()
{
    return QStringLiteral("The Movie DB");
}

QString TMDb::identifier()
{
    return QStringLiteral("tmdb");
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
    for (int i = 0, n = m_box->count(); i < n; ++i) {
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
    QUrl url(QStringLiteral("https://api.themoviedb.org/3/configuration?api_key=%1").arg(TMDb::apiKey()));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply *const reply = m_qnam.get(request);
    new NetworkReplyWatcher(this, reply);
    connect(reply, &QNetworkReply::finished, this, &TMDb::setupFinished);
}

/**
 * @brief Called when setup parameters were got
 *        Parses json and assigns the baseUrl
 */
void TMDb::setupFinished()
{
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }

    QJsonParseError parseError;
    const auto parsedJson = QJsonDocument::fromJson(reply->readAll(), &parseError).object();
    reply->deleteLater();
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing TMDb setup json " << parseError.errorString();
        return;
    }

    const auto imagesObject = parsedJson.value("images").toObject();
    m_baseUrl = imagesObject.value("base_url").toString();
    qDebug() << "TMDb base url:" << m_baseUrl;
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
    QString searchTitle;
    QString searchYear;
    QUrl url;
    QString includeAdult = (Settings::instance()->showAdultScrapers()) ? "true" : "false";
    QRegExp rx("^tt\\d+$");
    QRegExp rxTmdbId("^id\\d+$");

    if (rx.exactMatch(searchStr)) {
        QUrl newUrl(getMovieUrl(searchStr,
            ApiMovieDetails::INFOS,
            UrlParameterMap{{ApiUrlParameter::INCLUDE_ADULT, includeAdult}, {ApiUrlParameter::LANGUAGE, m_language}}));
        url.swap(newUrl);

    } else if (rxTmdbId.exactMatch(searchStr)) {
        QUrl newUrl(getMovieUrl(searchStr.mid(2),
            ApiMovieDetails::INFOS,
            UrlParameterMap{{ApiUrlParameter::INCLUDE_ADULT, includeAdult}, {ApiUrlParameter::LANGUAGE, m_language}}));
        url.swap(newUrl);

    } else {
        QUrl newUrl(getMovieSearchUrl(searchStr, UrlParameterMap{{ApiUrlParameter::INCLUDE_ADULT, includeAdult}}));
        url.swap(newUrl);
        QList<QRegExp> rxYears;
        rxYears << QRegExp(R"(^(.*) \((\d{4})\)$)") << QRegExp("^(.*) (\\d{4})$") << QRegExp("^(.*) - (\\d{4})$");
        foreach (QRegExp rxYear, rxYears) {
            rxYear.setMinimal(true);
            if (rxYear.exactMatch(searchStr)) {
                searchTitle = rxYear.cap(1);
                searchYear = rxYear.cap(2);
                QUrl newUrl = getMovieSearchUrl(searchTitle,
                    UrlParameterMap{
                        {ApiUrlParameter::INCLUDE_ADULT, includeAdult}, {ApiUrlParameter::YEAR, searchYear}});
                url.swap(newUrl);
                break;
            }
        }
    }
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply *const reply = m_qnam.get(request);
    new NetworkReplyWatcher(this, reply);
    if (!searchTitle.isEmpty() && !searchYear.isEmpty()) {
        reply->setProperty("searchTitle", searchTitle);
        reply->setProperty("searchYear", searchYear);
    }
    reply->setProperty("searchString", searchStr);
    reply->setProperty("results", Storage::toVariant(reply, QList<ScraperSearchResult>()));
    reply->setProperty("page", 1);
    connect(reply, &QNetworkReply::finished, this, &TMDb::searchFinished);
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "searchDone" if there are no more pages in the result set
 * @see TMDb::parseSearch
 */
void TMDb::searchFinished()
{
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    QList<ScraperSearchResult> results = reply->property("results").value<Storage *>()->results();

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
        QString nextPageStr{QString::number(nextPage)};
        const QUrl url = [&]() {
            if (searchTitle.isEmpty() || searchYear.isEmpty()) {
                return getMovieSearchUrl(searchString, UrlParameterMap{{ApiUrlParameter::PAGE, nextPageStr}});
            }
            return getMovieSearchUrl(searchTitle,
                UrlParameterMap{{ApiUrlParameter::PAGE, nextPageStr}, {ApiUrlParameter::YEAR, searchYear}});
        }();

        QNetworkRequest request(url);
        request.setRawHeader("Accept", "application/json");
        QNetworkReply *const reply = m_qnam.get(request);
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("searchString", searchString);
        reply->setProperty("results", Storage::toVariant(reply, results));
        reply->setProperty("page", nextPage);
        connect(reply, &QNetworkReply::finished, this, &TMDb::searchFinished);
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
    QList<ScraperSearchResult> results;

    QJsonParseError parseError;
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing search json " << parseError.errorString();
        return results;
    }

    // only get the first 3 pages
    if (page < parsedJson.value("total_pages").toInt() && page < 3) {
        *nextPage = page + 1;
    }

    if (parsedJson.value("results").isArray()) {
        const auto jsonResults = parsedJson.value("results").toArray();
        for (const auto &it : jsonResults) {
            const auto resultObj = it.toObject();
            if (resultObj.value("id").toInt() == 0) {
                continue;
            }
            ScraperSearchResult result;
            result.name = resultObj.value("title").toString();
            if (result.name.isEmpty()) {
                result.name = resultObj.value("original_title").toString();
            }
            result.id = QString::number(resultObj.value("id").toInt());
            result.released = QDate::fromString(resultObj.value("release_date").toString(), "yyyy-MM-dd");
            results.append(result);
        }

    } else if (parsedJson.value("id").toInt() > 0) {
        ScraperSearchResult result;
        result.name = parsedJson.value("title").toString();
        if (result.name.isEmpty()) {
            result.name = parsedJson.value("original_title").toString();
        }
        result.id = QString::number(parsedJson.value("id").toInt());
        result.released = QDate::fromString(parsedJson.value("release_date").toString(), "yyyy-MM-dd");
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
void TMDb::loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<int> infos)
{
    if (!ids.values().first().startsWith("tt")) {
        movie->setTmdbId(ids.values().first());
    }
    movie->clear(infos);

    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");

    QList<ScraperData> loadsLeft;

    // Infos
    loadsLeft.append(DataInfos);

    request.setUrl(getMovieUrl(
        ids.values().first(), ApiMovieDetails::INFOS, UrlParameterMap{{ApiUrlParameter::LANGUAGE, m_language}}));
    QNetworkReply *const reply = m_qnam.get(request);
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, &QNetworkReply::finished, this, &TMDb::loadFinished);

    // Casts
    if (infos.contains(MovieScraperInfos::Actors) || infos.contains(MovieScraperInfos::Director)
        || infos.contains(MovieScraperInfos::Writer)) {
        loadsLeft.append(DataCasts);
        request.setUrl(getMovieUrl(ids.values().first(), ApiMovieDetails::CASTS));
        QNetworkReply *const reply = m_qnam.get(request);
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TMDb::loadCastsFinished);
    }

    // Trailers
    if (infos.contains(MovieScraperInfos::Trailer)) {
        loadsLeft.append(DataTrailers);
        request.setUrl(getMovieUrl(
            ids.values().first(), ApiMovieDetails::TRAILERS, UrlParameterMap{{ApiUrlParameter::LANGUAGE, m_language}}));
        QNetworkReply *const reply = m_qnam.get(request);
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TMDb::loadTrailersFinished);
    }

    // Images
    if (infos.contains(MovieScraperInfos::Poster) || infos.contains(MovieScraperInfos::Backdrop)) {
        loadsLeft.append(DataImages);
        request.setUrl(getMovieUrl(ids.values().first(), ApiMovieDetails::IMAGES));
        QNetworkReply *const reply = m_qnam.get(request);
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TMDb::loadImagesFinished);
    }

    // Releases
    if (infos.contains(MovieScraperInfos::Certification)) {
        loadsLeft.append(DataReleases);
        request.setUrl(getMovieUrl(ids.values().first(), ApiMovieDetails::RELEASES));
        QNetworkReply *const reply = m_qnam.get(request);
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TMDb::loadReleasesFinished);
    }
    movie->controller()->setLoadsLeft(loadsLeft);
}

/**
 * @brief Called when the movie infos are downloaded
 * @see TMDb::parseAndAssignInfos
 */
void TMDb::loadFinished()
{
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    Movie *const movie = reply->property("storage").value<Storage *>()->movie();
    const QList<int> infos = reply->property("infosToLoad").value<Storage *>()->infosToLoad();
    reply->deleteLater();
    if (!movie) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        const QString msg = QString::fromUtf8(reply->readAll());
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
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    Movie *const movie = reply->property("storage").value<Storage *>()->movie();
    const QList<int> infos = reply->property("infosToLoad").value<Storage *>()->infosToLoad();
    reply->deleteLater();
    if (!movie) {
        return;
    }

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
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    Movie *const movie = reply->property("storage").value<Storage *>()->movie();
    const QList<int> infos = reply->property("infosToLoad").value<Storage *>()->infosToLoad();
    reply->deleteLater();
    if (!movie) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        const QString msg = QString::fromUtf8(reply->readAll());
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
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage *>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage *>()->infosToLoad();
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
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage *>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage *>()->infosToLoad();
    reply->deleteLater();
    if (!movie) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        qWarning() << "Network Error (releases)" << reply->errorString();
    }
    movie->controller()->removeFromLoadsLeft(DataReleases);
}

/**
 * @brief Get a string representation of ApiUrlParameter
 */
QString TMDb::apiUrlParameterString(ApiUrlParameter parameter) const
{
    switch (parameter) {
    case ApiUrlParameter::LANGUAGE: return QStringLiteral("language");
    case ApiUrlParameter::YEAR: return QStringLiteral("year");
    case ApiUrlParameter::PAGE: return QStringLiteral("page");
    case ApiUrlParameter::INCLUDE_ADULT: return QStringLiteral("include_adult");
    default: return QStringLiteral("unknown");
    }
}

/**
 * @brief Get the movie search URL for TMDb. Adds the API key and language.
 * @param search Search string. Will be percent encoded.
 * @param arguments A QMap of URL parameters. The values will be percent encoded.
 */
QUrl TMDb::getMovieSearchUrl(const QString &searchStr, const UrlParameterMap &parameters) const
{
    auto url = QStringLiteral("https://api.themoviedb.org/3/search/movie?");

    QUrlQuery queries;
    queries.addQueryItem("api_key", TMDb::apiKey());
    queries.addQueryItem("language", m_language);
    queries.addQueryItem("query", searchStr);

    for (const auto &key : parameters.keys()) {
        queries.addQueryItem(apiUrlParameterString(key), parameters.value(key));
    }

    return QUrl{url.append(queries.toString())};
}

/**
 * @brief Get the movie URL for TMDb. Adds the API key.
 * @param search Search string. Will be percent encoded.
 * @param arguments A QMap of URL parameters. The values will be percent encoded.
 */
QUrl TMDb::getMovieUrl(const QString &title, ApiMovieDetails type, const UrlParameterMap &parameters) const
{
    const auto typeStr = [type]() {
        switch (type) {
        case ApiMovieDetails::INFOS: return QString{};
        case ApiMovieDetails::IMAGES: return QStringLiteral("/images");
        case ApiMovieDetails::CASTS: return QStringLiteral("/casts");
        case ApiMovieDetails::TRAILERS: return QStringLiteral("/trailers");
        case ApiMovieDetails::RELEASES: return QStringLiteral("/releases");
        default: return QString{};
        }
    }();

    auto url = QStringLiteral("https://api.themoviedb.org/3/movie/%1%2?").arg(QUrl::toPercentEncoding(title), typeStr);
    QUrlQuery queries;
    queries.addQueryItem("api_key", TMDb::apiKey());

    for (const auto &key : parameters.keys()) {
        queries.addQueryItem(apiUrlParameterString(key), parameters.value(key));
    }

    return QUrl{url.append(queries.toString())};
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
    QJsonParseError parseError;
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing info json " << parseError.errorString();
        return;
    }

    // Infos
    if (!parsedJson.value("imdb_id").toString().isEmpty()) {
        movie->setId(parsedJson.value("imdb_id").toString());
    }
    if (infos.contains(MovieScraperInfos::Title)) {
        if (!parsedJson.value("title").toString().isEmpty()) {
            movie->setName(parsedJson.value("title").toString());
        }
        if (!parsedJson.value("original_title").toString().isEmpty()) {
            movie->setOriginalName(parsedJson.value("original_title").toString());
        }
    }
    if (infos.contains(MovieScraperInfos::Set) && parsedJson.value("belongs_to_collection").isObject()) {
        const auto collection = parsedJson.value("belongs_to_collection").toObject();
        movie->setSet(collection.value("name").toString());
    }
    if (infos.contains(MovieScraperInfos::Overview)) {
        const auto overviewStr = parsedJson.value("overview").toString();
        if (!overviewStr.isEmpty()) {
            movie->setOverview(overviewStr);
            if (Settings::instance()->usePlotForOutline()) {
                movie->setOutline(overviewStr);
            }
        }
    }
    // Either set both vote_average and vote_count or neither one.
    if (infos.contains(MovieScraperInfos::Rating) && parsedJson.value("vote_average").toDouble(-1) >= 0) {
        movie->setRating(parsedJson.value("vote_average").toDouble());
        movie->setVotes(parsedJson.value("vote_count").toInt());
    }
    if (infos.contains(MovieScraperInfos::Tagline) && !parsedJson.value("tagline").toString().isEmpty()) {
        movie->setTagline(parsedJson.value("tagline").toString());
    }
    if (infos.contains(MovieScraperInfos::Released) && !parsedJson.value("release_date").toString().isEmpty()) {
        movie->setReleased(QDate::fromString(parsedJson.value("release_date").toString(), "yyyy-MM-dd"));
    }
    if (infos.contains(MovieScraperInfos::Runtime) && parsedJson.value("runtime").toInt(-1) >= 0) {
        movie->setRuntime(parsedJson.value("runtime").toInt());
    }
    if (infos.contains(MovieScraperInfos::Genres) && parsedJson.value("genres").isArray()) {
        const auto genres = parsedJson.value("genres").toArray();
        for (const auto &it : genres) {
            const auto genre = it.toObject();
            if (genre.value("id").toInt(-1) == -1) {
                continue;
            }
            movie->addGenre(Helper::mapGenre(genre.value("name").toString()));
        }
    }
    if (infos.contains(MovieScraperInfos::Studios) && parsedJson.value("production_companies").isArray()) {
        const auto companies = parsedJson.value("production_companies").toArray();
        for (const auto &it : companies) {
            const auto company = it.toObject();
            if (company.value("id").toInt(-1) == -1) {
                continue;
            }
            movie->addStudio(Helper::mapStudio(company.value("name").toString()));
        }
    }
    if (infos.contains(MovieScraperInfos::Countries) && parsedJson.value("production_countries").isArray()) {
        const auto countries = parsedJson.value("production_countries").toArray();
        for (const auto &it : countries) {
            const auto country = it.toObject();
            if (country.value("name").toString().isEmpty()) {
                continue;
            }
            movie->addCountry(Helper::mapStudio(country.value("name").toString()));
        }
    }

    // Casts
    if (infos.contains(MovieScraperInfos::Actors) && parsedJson.value("cast").isArray()) {
        const auto cast = parsedJson.value("cast").toArray();
        for (const auto &it : cast) {
            const auto actor = it.toObject();
            if (actor.value("name").toString().isEmpty()) {
                continue;
            }
            Actor a;
            a.name = actor.value("name").toString();
            a.role = actor.value("character").toString();
            if (!actor.value("profile_path").toString().isEmpty()) {
                a.thumb = m_baseUrl + "original" + actor.value("profile_path").toString();
            }
            movie->addActor(a);
        }
    }

    // Crew
    if ((infos.contains(MovieScraperInfos::Director) || infos.contains(MovieScraperInfos::Writer))
        && parsedJson.value("crew").isArray()) {
        const auto crew = parsedJson.value("crew").toArray();
        for (const auto &it : crew) {
            const auto member = it.toObject();
            if (member.value("name").toString().isEmpty()) {
                continue;
            }
            if (infos.contains(MovieScraperInfos::Writer) && member.value("department").toString() == "Writing") {
                QString writer = movie->writer();
                if (writer.contains(member.value("name").toString())) {
                    continue;
                }
                if (!writer.isEmpty()) {
                    writer.append(", ");
                }
                writer.append(member.value("name").toString());
                movie->setWriter(writer);
            }
            if (infos.contains(MovieScraperInfos::Director) && member.value("job").toString() == "Director"
                && member.value("department").toString() == "Directing") {
                movie->setDirector(member.value("name").toString());
            }
        }
    }

    // Trailers
    if (infos.contains(MovieScraperInfos::Trailer) && parsedJson.value("youtube").isArray()) {
        // The trailer listed first is most likely also the best.
        const auto firstTrailer = parsedJson.value("youtube").toArray().first().toObject();
        if (!firstTrailer.value("source").toString().isEmpty()) {
            const QString youtubeSrc = firstTrailer.value("source").toString();
            movie->setTrailer(
                QUrl(Helper::formatTrailerUrl(QStringLiteral("https://www.youtube.com/watch?v=%1").arg(youtubeSrc))));
        }
    }

    // Images
    if (infos.contains(MovieScraperInfos::Backdrop) && parsedJson.value("backdrops").isArray()) {
        const auto backdrops = parsedJson.value("backdrops").toArray();
        for (const auto &it : backdrops) {
            const auto backdrop = it.toObject();
            const QString filePath = backdrop.value("file_path").toString();
            if (filePath.isEmpty()) {
                continue;
            }
            Poster b;
            b.thumbUrl = m_baseUrl + "w780" + filePath;
            b.originalUrl = m_baseUrl + "original" + filePath;
            b.originalSize.setWidth(backdrop.value("width").toInt());
            b.originalSize.setHeight(backdrop.value("height").toInt());
            movie->addBackdrop(b);
        }
    }

    if (infos.contains(MovieScraperInfos::Poster) && parsedJson.value("posters").isArray()) {
        const auto posters = parsedJson.value("posters").toArray();
        for (const auto &it : posters) {
            const auto poster = it.toObject();
            const QString filePath = poster.value("file_path").toString();
            if (filePath.isEmpty()) {
                continue;
            }
            Poster b;
            b.thumbUrl = m_baseUrl + "w342" + filePath;
            b.originalUrl = m_baseUrl + "original" + filePath;
            b.originalSize.setWidth(poster.value("width").toInt());
            b.originalSize.setHeight(poster.value("height").toInt());
            b.language = poster.value("iso_639_1").toString();
            bool primaryLang = (b.language == m_language);
            movie->addPoster(b, primaryLang);
        }
    }

    // Releases
    if (infos.contains(MovieScraperInfos::Certification) && parsedJson.value("countries").isArray()) {
        QString locale;
        QString us;
        QString gb;
        const auto countries = parsedJson.value("countries").toArray();
        for (const auto &it : countries) {
            const auto country = it.toObject();
            const QString iso3166 = country.value("iso_3166_1").toString();
            const QString certification = country.value("certification").toString();

            if (iso3166 == "US") {
                us = certification;
            }
            if (iso3166 == "GB") {
                gb = certification;
            }
            if (iso3166.toLower() == m_language) {
                locale = certification;
            }
        }

        if (m_language2 == "US" && !us.isEmpty()) {
            movie->setCertification(Helper::mapCertification(us));
        } else if (m_language == "en" && m_language2 == "" && !gb.isEmpty()) {
            movie->setCertification(Helper::mapCertification(gb));
        } else if (!locale.isEmpty()) {
            movie->setCertification(Helper::mapCertification(locale));
        } else if (!us.isEmpty()) {
            movie->setCertification(Helper::mapCertification(us));
        } else if (!gb.isEmpty()) {
            movie->setCertification(Helper::mapCertification(gb));
        }
    }
}
