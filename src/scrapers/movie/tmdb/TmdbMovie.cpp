#include "TmdbMovie.h"

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "settings/Settings.h"
#include "ui/main/MainWindow.h"

#include <QDebug>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QTextDocument>
#include <QUrlQuery>

namespace mediaelch {
namespace scraper {

TmdbMovie::TmdbMovie(QObject* parent) :
    MovieScraper(parent),
    m_baseUrl{"http://image.tmdb.org/t/p/"},
    m_scraperNativelySupports{MovieScraperInfo::Title,
        MovieScraperInfo::Tagline,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Certification,
        MovieScraperInfo::Trailer,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Backdrop,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Director,
        MovieScraperInfo::Writer,
        MovieScraperInfo::Set}
{
    m_meta.identifier = ID;
    m_meta.name = "The Movie DB";
    m_meta.description = tr("The Movie Database (TMDb) is a community built movie and TV database. "
                            "Every piece of data has been added by our amazing community dating back to 2008. "
                            "TMDb's strong international focus and breadth of data is largely unmatched and "
                            "something we're incredibly proud of. Put simply, we live and breathe community "
                            "and that's precisely what makes us different.");
    m_meta.website = "https://www.themoviedb.org/tv";
    m_meta.termsOfService = "https://www.themoviedb.org/terms-of-use";
    m_meta.privacyPolicy = "https://www.themoviedb.org/privacy-policy";
    m_meta.help = "https://www.themoviedb.org/talk";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Tagline,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Certification,
        MovieScraperInfo::Trailer,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Backdrop,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Director,
        MovieScraperInfo::Writer,
        MovieScraperInfo::Logo,
        MovieScraperInfo::Banner,
        MovieScraperInfo::Thumb,
        MovieScraperInfo::CdArt,
        MovieScraperInfo::ClearArt,
        MovieScraperInfo::Set};
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
    m_meta.defaultLocale = mediaelch::Locale::English;
    m_meta.isAdult = false;

    m_widget = new QWidget(MainWindow::instance());
    m_box = new QComboBox(m_widget);

    for (const mediaelch::Locale& lang : m_meta.supportedLanguages) {
        m_box->addItem(lang.languageTranslated(), lang.toString());
    }

    auto* layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);

    setup();
}

const MovieScraper::ScraperMeta& TmdbMovie::meta() const
{
    return m_meta;
}

void TmdbMovie::initialize()
{
    // TODO
}

bool TmdbMovie::isInitialized() const
{
    // TODO
    return true;
}

QString TmdbMovie::apiKey()
{
    return QStringLiteral("5d832bdf69dcb884922381ab01548d5b");
}

bool TmdbMovie::hasSettings() const
{
    return true;
}

QWidget* TmdbMovie::settingsWidget()
{
    return m_widget;
}

void TmdbMovie::loadSettings(ScraperSettings& settings)
{
    m_meta.defaultLocale = settings.language(m_meta.defaultLocale);
    if (m_meta.defaultLocale.toString() == "C") {
        m_meta.defaultLocale = "en";
    }

    const QString locale = localeForTMDb();
    const QString lang = language();

    for (int i = 0, n = m_box->count(); i < n; ++i) {
        if (m_box->itemData(i).toString() == lang || m_box->itemData(i).toString() == locale) {
            m_box->setCurrentIndex(i);
            break;
        }
    }
}

void TmdbMovie::saveSettings(ScraperSettings& settings)
{
    const QString language = m_box->itemData(m_box->currentIndex()).toString();
    settings.setString("Language", language);
    loadSettings(settings);
}

QSet<MovieScraperInfo> TmdbMovie::scraperNativelySupports()
{
    return m_scraperNativelySupports;
}

void TmdbMovie::changeLanguage(mediaelch::Locale locale)
{
    if (m_meta.supportedLanguages.contains(locale)) {
        m_meta.defaultLocale = locale;
    } else {
        qInfo() << "[TMDb] Cannot change language because it is not supported:" << locale;
    }
}

/**
 * \brief Loads the setup parameters from TMDb
 * \see TMDb::setupFinished
 */
void TmdbMovie::setup()
{
    qDebug() << "[TMDb] Request setup from server";
    QUrl url(QStringLiteral("https://api.themoviedb.org/3/configuration?api_key=%1").arg(TmdbMovie::apiKey()));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply* const reply = m_network.getWithWatcher(request);
    connect(reply, &QNetworkReply::finished, this, &TmdbMovie::setupFinished);
}

QString TmdbMovie::localeForTMDb() const
{
    return m_meta.defaultLocale.toString('-');
}

/**
 * \return Two letter language code (lowercase)
 */
QString TmdbMovie::language() const
{
    return m_meta.defaultLocale.language();
}

/**
 * \return Two or three letter country code (uppercase)
 */
QString TmdbMovie::country() const
{
    return m_meta.defaultLocale.country();
}

/**
 * \brief Called when setup parameters were got
 *        Parses json and assigns the baseUrl
 */
void TmdbMovie::setupFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }

    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(reply->readAll(), &parseError).object();
    reply->deleteLater();
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[TMDb] Error parsing setup json:" << parseError.errorString();
        return;
    }

    const auto imagesObject = parsedJson.value("images").toObject();
    m_baseUrl = imagesObject.value("base_url").toString();
    qDebug() << "[TMDb] Base url:" << m_baseUrl;
}

/**
 * \brief Searches for a movie
 * \param searchStr The Movie name/search string
 * \see TMDb::searchFinished
 */
void TmdbMovie::search(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    searchStr = searchStr.replace("-", " ");

    if (searchStr.isEmpty()) {
        // searching without a query results in a network error
        QTimer::singleShot(0, [this]() { emit searchDone({}, {}); });
        return;
    }

    QString searchTitle;
    QString searchYear;
    QUrl url;
    QString includeAdult = (Settings::instance()->showAdultScrapers()) ? "true" : "false";

    const bool isSearchByImdbId = QRegExp("^tt\\d+$").exactMatch(searchStr);
    const bool isSearchByTmdbId = QRegExp("^id\\d+$").exactMatch(searchStr);

    if (isSearchByImdbId) {
        QUrl newUrl(getMovieUrl(
            searchStr, ApiMovieDetails::INFOS, UrlParameterMap{{ApiUrlParameter::INCLUDE_ADULT, includeAdult}}));
        url.swap(newUrl);

    } else if (isSearchByTmdbId) {
        QUrl newUrl(getMovieUrl(
            searchStr.mid(2), ApiMovieDetails::INFOS, UrlParameterMap{{ApiUrlParameter::INCLUDE_ADULT, includeAdult}}));
        url.swap(newUrl);

    } else {
        QUrl newUrl(getMovieSearchUrl(searchStr, UrlParameterMap{{ApiUrlParameter::INCLUDE_ADULT, includeAdult}}));
        url.swap(newUrl);
        QVector<QRegExp> rxYears;
        rxYears << QRegExp(R"(^(.*) \((\d{4})\)$)") << QRegExp("^(.*) (\\d{4})$") << QRegExp("^(.*) - (\\d{4})$");
        for (QRegExp rxYear : rxYears) {
            rxYear.setMinimal(true);
            if (rxYear.exactMatch(searchStr)) {
                searchTitle = rxYear.cap(1);
                searchYear = rxYear.cap(2);
                QUrl newSearchUrl = getMovieSearchUrl(searchTitle,
                    UrlParameterMap{
                        {ApiUrlParameter::INCLUDE_ADULT, includeAdult}, {ApiUrlParameter::YEAR, searchYear}});
                url.swap(newSearchUrl);
                break;
            }
        }
    }
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply* const reply = m_network.getWithWatcher(request);
    if (!searchTitle.isEmpty() && !searchYear.isEmpty()) {
        reply->setProperty("searchTitle", searchTitle);
        reply->setProperty("searchYear", searchYear);
    }
    reply->setProperty("searchString", searchStr);
    reply->setProperty("results", Storage::toVariant(reply, QVector<ScraperSearchResult>()));
    reply->setProperty("page", 1);
    connect(reply, &QNetworkReply::finished, this, &TmdbMovie::searchFinished);
}

/**
 * \brief Called when the search result was downloaded
 *        Emits "searchDone" if there are no more pages in the result set
 * \see TMDb::parseSearch
 */
void TmdbMovie::searchFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[TMDb] onSearchFinished: nullptr reply | Please report this issue!";
        emit searchDone(
            {}, {ScraperError::Type::InternalError, tr("Internal Error: Please report!"), "nullptr dereference"});
        return;
    }
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[TMDb] Search: Network Error" << reply->errorString();
        emit searchDone({}, mediaelch::replyToScraperError(*reply));
        return;
    }

    QVector<ScraperSearchResult> results = reply->property("results").value<Storage*>()->results();
    QString searchString = reply->property("searchString").toString();
    QString searchTitle = reply->property("searchTitle").toString();
    QString searchYear = reply->property("searchYear").toString();
    int page = reply->property("page").toInt();
    QString msg = QString::fromUtf8(reply->readAll());
    int nextPage = -1;
    results.append(parseSearch(msg, &nextPage, page));
    reply->deleteLater();

    if (nextPage == -1) {
        emit searchDone(results, {});
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
        QNetworkReply* const searchReply = m_network.getWithWatcher(request);
        searchReply->setProperty("searchString", searchString);
        searchReply->setProperty("results", Storage::toVariant(searchReply, results));
        searchReply->setProperty("page", nextPage);
        connect(searchReply, &QNetworkReply::finished, this, &TmdbMovie::searchFinished);
    }
}

/**
 * \brief Parses the JSON search results
 * \param json JSON string
 * \param nextPage This will hold the next page to get, -1 if there are no more pages
 * \return List of search results
 */
QVector<ScraperSearchResult> TmdbMovie::parseSearch(QString json, int* nextPage, int page)
{
    QVector<ScraperSearchResult> results;

    QJsonParseError parseError{};
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
        for (const auto& it : jsonResults) {
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
 * \brief Starts network requests to download infos from TMDb
 * \param ids TMDb movie ID
 * \param movie Movie object
 * \param infos List of infos to load
 * \see TMDb::loadFinished
 * \see TMDb::loadCastsFinished
 * \see TMDb::loadTrailersFinished
 * \see TMDb::loadImagesFinished
 * \see TMDb::loadReleasesFinished
 */
void TmdbMovie::loadData(QHash<MovieScraper*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos)
{
    const QString id = ids.values().first();
    const bool isImdbId = id.startsWith("tt");

    if (isImdbId) {
        movie->setImdbId(ImdbId(id));
    } else {
        movie->setTmdbId(TmdbId(id));
    }

    movie->clear(infos);

    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");

    QVector<ScraperData> loadsLeft;

    // Infos
    {
        loadsLeft.append(ScraperData::Infos);

        request.setUrl(getMovieUrl(id, ApiMovieDetails::INFOS));
        QNetworkReply* const reply = m_network.getWithWatcher(request);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbMovie::loadFinished);
    }

    // Casts
    if (infos.contains(MovieScraperInfo::Actors) || infos.contains(MovieScraperInfo::Director)
        || infos.contains(MovieScraperInfo::Writer)) {
        loadsLeft.append(ScraperData::Casts);
        request.setUrl(getMovieUrl(id, ApiMovieDetails::CASTS));
        QNetworkReply* const reply = m_network.getWithWatcher(request);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbMovie::loadCastsFinished);
    }

    // Trailers
    if (infos.contains(MovieScraperInfo::Trailer)) {
        loadsLeft.append(ScraperData::Trailers);
        request.setUrl(getMovieUrl(id, ApiMovieDetails::TRAILERS));
        QNetworkReply* const reply = m_network.getWithWatcher(request);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbMovie::loadTrailersFinished);
    }

    // Images
    if (infos.contains(MovieScraperInfo::Poster) || infos.contains(MovieScraperInfo::Backdrop)) {
        loadsLeft.append(ScraperData::Images);
        request.setUrl(getMovieUrl(id, ApiMovieDetails::IMAGES));
        QNetworkReply* const reply = m_network.getWithWatcher(request);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbMovie::loadImagesFinished);
    }

    // Releases
    if (infos.contains(MovieScraperInfo::Certification)) {
        loadsLeft.append(ScraperData::Releases);
        request.setUrl(getMovieUrl(id, ApiMovieDetails::RELEASES));
        QNetworkReply* const reply = m_network.getWithWatcher(request);
        reply->setProperty("storage", Storage::toVariant(reply, movie));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbMovie::loadReleasesFinished);
    }

    movie->controller()->setLoadsLeft(loadsLeft);
}

/// Called when the movie infos are downloaded
/// \see TMDb::parseAndAssignInfos
void TmdbMovie::loadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* const movie = reply->property("storage").value<Storage*>()->movie();
    const QSet<MovieScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        const QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);

        // if the movie is part of a collection then download the collection data
        // and delay the call to removeFromLoadsLeft(ScraperData::Infos)
        // to loadCollectionFinished()
        if (infos.contains(MovieScraperInfo::Set)) {
            loadCollection(movie, movie->set().tmdbId);
            return;
        }

    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error (load)" << reply->errorString();
    }

    movie->controller()->removeFromLoadsLeft(ScraperData::Infos);
}

void TmdbMovie::loadCollection(Movie* movie, const TmdbId& collectionTmdbId)
{
    if (!collectionTmdbId.isValid()) {
        movie->controller()->removeFromLoadsLeft(ScraperData::Infos);
        return;
    }

    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    request.setUrl(getCollectionUrl(collectionTmdbId.toString()));

    QNetworkReply* const reply = m_network.getWithWatcher(request);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    connect(reply, &QNetworkReply::finished, this, &TmdbMovie::loadCollectionFinished);
}

void TmdbMovie::loadCollectionFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* const movie = reply->property("storage").value<Storage*>()->movie();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    const QString msg = QString::fromUtf8(reply->readAll());
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(msg.toUtf8(), &parseError).object();
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing info json " << parseError.errorString();
        return;
    }

    movie->controller()->removeFromLoadsLeft(ScraperData::Infos);

    if (parsedJson.keys().contains("success") && !parsedJson.value("success").toBool()) {
        qWarning() << "[TMDb] Error message from TMDb:" << parsedJson.value("status_message");
        return;
    }

    MovieSet set;
    set.tmdbId = TmdbId(parsedJson.value("id").toInt());
    set.name = parsedJson.value("name").toString();
    set.overview = parsedJson.value("overview").toString();
    movie->setSet(set);
}

/**
 * \brief Called when the movie casts are downloaded
 * \see TMDb::parseAndAssignInfos
 */
void TmdbMovie::loadCastsFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* const movie = reply->property("storage").value<Storage*>()->movie();
    const QSet<MovieScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error (casts)" << reply->errorString();
    }
    movie->controller()->removeFromLoadsLeft(ScraperData::Casts);
}

/**
 * \brief Called when the movie trailers are downloaded
 * \see TMDb::parseAndAssignInfos
 */
void TmdbMovie::loadTrailersFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* const movie = reply->property("storage").value<Storage*>()->movie();
    const QSet<MovieScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        const QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        showNetworkError(*reply);
        qDebug() << "Network Error (trailers)" << reply->errorString();
    }
    movie->controller()->removeFromLoadsLeft(ScraperData::Trailers);
}

/**
 * \brief Called when the movie images are downloaded
 * \see TMDb::parseAndAssignInfos
 */
void TmdbMovie::loadImagesFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* movie = reply->property("storage").value<Storage*>()->movie();
    QSet<MovieScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error (images)" << reply->errorString();
    }
    movie->controller()->removeFromLoadsLeft(ScraperData::Images);
}

/**
 * \brief Called when the movie releases are downloaded
 * \see TMDb::parseAndAssignInfos
 */
void TmdbMovie::loadReleasesFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* movie = reply->property("storage").value<Storage*>()->movie();
    QSet<MovieScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error (releases)" << reply->errorString();
    }
    movie->controller()->removeFromLoadsLeft(ScraperData::Releases);
}

/**
 * \brief Get a string representation of ApiUrlParameter
 */
QString TmdbMovie::apiUrlParameterString(ApiUrlParameter parameter) const
{
    switch (parameter) {
    case ApiUrlParameter::YEAR: return QStringLiteral("year");
    case ApiUrlParameter::PAGE: return QStringLiteral("page");
    case ApiUrlParameter::INCLUDE_ADULT: return QStringLiteral("include_adult");
    }
    qCritical() << "[TMDb] ApiUrlParameter: Unhandled enum case.";
    return QStringLiteral("unknown");
}

/**
 * \brief Get the movie search URL for TMDb. Adds the API key and language.
 * \param searchStr Search string. Will be percent encoded.
 * \param parameters A QMap of URL parameters. The values will be percent encoded.
 */
QUrl TmdbMovie::getMovieSearchUrl(const QString& searchStr, const UrlParameterMap& parameters) const
{
    auto url = QStringLiteral("https://api.themoviedb.org/3/search/movie?");

    QUrlQuery queries;
    queries.addQueryItem("api_key", TmdbMovie::apiKey());
    queries.addQueryItem("language", localeForTMDb());
    queries.addQueryItem("query", searchStr);

    for (const auto& key : parameters.keys()) {
        queries.addQueryItem(apiUrlParameterString(key), parameters.value(key));
    }

    return QUrl{url.append(queries.toString())};
}

/// \brief Get the movie URL for TMDb. Adds the API key.
QUrl TmdbMovie::getMovieUrl(QString movieId, ApiMovieDetails type, const UrlParameterMap& parameters) const
{
    const auto typeStr = [type]() {
        switch (type) {
        case ApiMovieDetails::INFOS: return QString{};
        case ApiMovieDetails::IMAGES: return QStringLiteral("/images");
        case ApiMovieDetails::CASTS: return QStringLiteral("/casts");
        case ApiMovieDetails::TRAILERS: return QStringLiteral("/trailers");
        case ApiMovieDetails::RELEASES: return QStringLiteral("/releases");
        }
        return QString{};
    }();

    auto url =
        QStringLiteral("https://api.themoviedb.org/3/movie/%1%2?").arg(QUrl::toPercentEncoding(movieId), typeStr);
    QUrlQuery queries;
    queries.addQueryItem("api_key", TmdbMovie::apiKey());
    queries.addQueryItem("language", localeForTMDb());

    if (type == ApiMovieDetails::IMAGES) {
        queries.addQueryItem("include_image_language", "en,null," + language());
    }

    for (const auto& key : parameters.keys()) {
        queries.addQueryItem(apiUrlParameterString(key), parameters.value(key));
    }

    return QUrl{url.append(queries.toString())};
}

/// \brief Get the collection URL for TMDb. Adds the API key.
QUrl TmdbMovie::getCollectionUrl(QString collectionId) const
{
    auto url = QStringLiteral("https://api.themoviedb.org/3/collection/%1?").arg(collectionId);

    QUrlQuery queries;
    queries.addQueryItem("api_key", TmdbMovie::apiKey());
    queries.addQueryItem("language", localeForTMDb());

    return QUrl{url.append(queries.toString())};
}

/**
 * \brief Parses JSON data and assigns it to the given movie object
 *        Handles all types of data from TMDb (info, releases, trailers, casts, images)
 * \param json JSON data
 * \param movie Movie object
 * \param infos List of infos to load
 */
void TmdbMovie::parseAndAssignInfos(QString json, Movie* movie, QSet<MovieScraperInfo> infos)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing info json " << parseError.errorString();
        return;
    }

    // Infos
    int tmdbId = parsedJson.value("id").toInt(-1);
    if (tmdbId > -1) {
        movie->setTmdbId(TmdbId(tmdbId));
    }
    if (!parsedJson.value("imdb_id").toString().isEmpty()) {
        movie->setImdbId(ImdbId(parsedJson.value("imdb_id").toString()));
    }
    if (infos.contains(MovieScraperInfo::Title)) {
        if (!parsedJson.value("title").toString().isEmpty()) {
            movie->setName(parsedJson.value("title").toString());
        }
        if (!parsedJson.value("original_title").toString().isEmpty()) {
            movie->setOriginalName(parsedJson.value("original_title").toString());
        }
    }
    if (infos.contains(MovieScraperInfo::Set) && parsedJson.value("belongs_to_collection").isObject()) {
        const auto collection = parsedJson.value("belongs_to_collection").toObject();
        MovieSet set;
        set.tmdbId = TmdbId(collection.value("id").toInt());
        set.name = collection.value("name").toString();
        movie->setSet(set);
    }
    if (infos.contains(MovieScraperInfo::Overview)) {
        QTextDocument doc;
        doc.setHtml(parsedJson.value("overview").toString());
        const auto overviewStr = doc.toPlainText();
        if (!overviewStr.isEmpty()) {
            movie->setOverview(overviewStr);
            if (Settings::instance()->usePlotForOutline()) {
                movie->setOutline(overviewStr);
            }
        }
    }
    // Either set both vote_average and vote_count or neither one.
    if (infos.contains(MovieScraperInfo::Rating) && parsedJson.value("vote_average").toDouble(-1) >= 0) {
        Rating rating;
        rating.source = "themoviedb";
        rating.maxRating = 10;
        rating.rating = parsedJson.value("vote_average").toDouble();
        rating.voteCount = parsedJson.value("vote_count").toInt();
        movie->ratings().push_back(rating);
    }
    if (infos.contains(MovieScraperInfo::Tagline) && !parsedJson.value("tagline").toString().isEmpty()) {
        movie->setTagline(parsedJson.value("tagline").toString());
    }
    if (infos.contains(MovieScraperInfo::Released) && !parsedJson.value("release_date").toString().isEmpty()) {
        movie->setReleased(QDate::fromString(parsedJson.value("release_date").toString(), "yyyy-MM-dd"));
    }
    if (infos.contains(MovieScraperInfo::Runtime) && parsedJson.value("runtime").toInt(-1) >= 0) {
        movie->setRuntime(std::chrono::minutes(parsedJson.value("runtime").toInt()));
    }
    if (infos.contains(MovieScraperInfo::Genres) && parsedJson.value("genres").isArray()) {
        const auto genres = parsedJson.value("genres").toArray();
        for (const auto& it : genres) {
            const auto genre = it.toObject();
            if (genre.value("id").toInt(-1) == -1) {
                continue;
            }
            movie->addGenre(helper::mapGenre(genre.value("name").toString()));
        }
    }
    if (infos.contains(MovieScraperInfo::Studios) && parsedJson.value("production_companies").isArray()) {
        const auto companies = parsedJson.value("production_companies").toArray();
        for (const auto& it : companies) {
            const auto company = it.toObject();
            if (company.value("id").toInt(-1) == -1) {
                continue;
            }
            movie->addStudio(helper::mapStudio(company.value("name").toString()));
        }
    }
    if (infos.contains(MovieScraperInfo::Countries) && parsedJson.value("production_countries").isArray()) {
        const auto countries = parsedJson.value("production_countries").toArray();
        for (const auto& it : countries) {
            const auto country = it.toObject();
            if (country.value("name").toString().isEmpty()) {
                continue;
            }
            movie->addCountry(helper::mapCountry(country.value("name").toString()));
        }
    }

    // Casts
    if (infos.contains(MovieScraperInfo::Actors) && parsedJson.value("cast").isArray()) {
        // clear actors
        movie->setActors({});

        const auto cast = parsedJson.value("cast").toArray();
        for (const auto& it : cast) {
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
    if ((infos.contains(MovieScraperInfo::Director) || infos.contains(MovieScraperInfo::Writer))
        && parsedJson.value("crew").isArray()) {
        const auto crew = parsedJson.value("crew").toArray();
        for (const auto& it : crew) {
            const auto member = it.toObject();
            if (member.value("name").toString().isEmpty()) {
                continue;
            }
            if (infos.contains(MovieScraperInfo::Writer) && member.value("department").toString() == "Writing") {
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
            if (infos.contains(MovieScraperInfo::Director) && member.value("job").toString() == "Director"
                && member.value("department").toString() == "Directing") {
                movie->setDirector(member.value("name").toString());
            }
        }
    }

    // Trailers
    if (infos.contains(MovieScraperInfo::Trailer) && parsedJson.value("youtube").isArray()) {
        // Look for "type" key in each element and look for the first instance of "Trailer" as value
        const auto videos = parsedJson.value("youtube").toArray();
        for (const auto& it : videos) {
            const auto videoObj = it.toObject();
            const QString videoType = videoObj.value("type").toString();
            if (videoType.toLower() == "trailer") {
                const QString youtubeSrc = videoObj.value("source").toString();
                movie->setTrailer(QUrl(
                    helper::formatTrailerUrl(QStringLiteral("https://www.youtube.com/watch?v=%1").arg(youtubeSrc))));
                break;
            }
        }
    }

    // Images
    if (infos.contains(MovieScraperInfo::Backdrop) && parsedJson.value("backdrops").isArray()) {
        const auto backdrops = parsedJson.value("backdrops").toArray();
        for (const auto& it : backdrops) {
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
            movie->images().addBackdrop(b);
        }
    }

    if (infos.contains(MovieScraperInfo::Poster) && parsedJson.value("posters").isArray()) {
        const auto posters = parsedJson.value("posters").toArray();
        for (const auto& it : posters) {
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
            bool primaryLang = (b.language == language());
            movie->images().addPoster(b, primaryLang);
        }
    }

    // Releases
    if (infos.contains(MovieScraperInfo::Certification) && parsedJson.value("countries").isArray()) {
        Certification locale;
        Certification us;
        Certification gb;
        const auto countries = parsedJson.value("countries").toArray();
        for (const auto& it : countries) {
            const auto countryObj = it.toObject();
            const QString iso3166 = countryObj.value("iso_3166_1").toString();
            const Certification certification = Certification(countryObj.value("certification").toString());
            if (iso3166 == "US") {
                us = certification;
            }
            if (iso3166 == "GB") {
                gb = certification;
            }
            if (iso3166.toUpper() == country()) {
                locale = certification;
            }
        }

        if (m_meta.defaultLocale.country() == "US" && us.isValid()) {
            movie->setCertification(helper::mapCertification(us));

        } else if (m_meta.defaultLocale.language() == "en" && gb.isValid()) {
            movie->setCertification(helper::mapCertification(gb));

        } else if (locale.isValid()) {
            movie->setCertification(helper::mapCertification(locale));

        } else if (us.isValid()) {
            movie->setCertification(helper::mapCertification(us));

        } else if (gb.isValid()) {
            movie->setCertification(helper::mapCertification(gb));
        }
    }
}

} // namespace scraper
} // namespace mediaelch
