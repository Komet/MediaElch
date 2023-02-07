#include "TmdbMovie.h"

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "log/Log.h"
#include "scrapers/movie/tmdb/TmdbMovieSearchJob.h"
#include "settings/Settings.h"
#include "ui/main/MainWindow.h"

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
    m_scrapeJob(m_api, {}, nullptr),
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

    m_widget = new QWidget;
    m_box = new QComboBox(m_widget);

    for (const mediaelch::Locale& lang : asConst(m_meta.supportedLanguages)) {
        m_box->addItem(lang.languageTranslated(), lang.toString());
    }

    auto* layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);

    // TODO: Should not be called by the constructor
    initialize();
}

TmdbMovie::~TmdbMovie()
{
    if (!m_widget.isNull() && m_widget->parent() == nullptr) {
        // We set MainWindow::instance() as this Widget's parent.
        // But at construction time, the instance is not setup, yet.
        // See settingsWidget()
        m_widget->deleteLater();
    }
}

const MovieScraper::ScraperMeta& TmdbMovie::meta() const
{
    return m_meta;
}

void TmdbMovie::initialize()
{
    m_api.initialize();
}

bool TmdbMovie::isInitialized() const
{
    return m_api.isInitialized();
}

MovieSearchJob* TmdbMovie::search(MovieSearchJob::Config config)
{
    return new TmdbMovieSearchJob(m_api, std::move(config), this);
}

bool TmdbMovie::hasSettings() const
{
    return true;
}

QWidget* TmdbMovie::settingsWidget()
{
    if (m_widget->parent() == nullptr) {
        m_widget->setParent(MainWindow::instance());
    }
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
        qCInfo(generic) << "[TMDb] Cannot change language because it is not supported:" << locale;
    }
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
void TmdbMovie::loadData(QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
    Movie* movie,
    QSet<MovieScraperInfo> infos)
{
    const QString id = ids.values().first().str();

    if (ImdbId::isValidFormat(id)) {
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

        request.setUrl(m_api.getMovieUrl(id, m_meta.defaultLocale, TmdbApi::ApiMovieDetails::INFOS));
        QNetworkReply* const reply = m_network.getWithWatcher(request);
        reply->setProperty("storage", QVariant::fromValue(movie));
        reply->setProperty("infosToLoad", QVariant::fromValue(infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbMovie::loadFinished);
    }

    // Casts
    if (infos.contains(MovieScraperInfo::Actors) || infos.contains(MovieScraperInfo::Director)
        || infos.contains(MovieScraperInfo::Writer)) {
        loadsLeft.append(ScraperData::Casts);
        request.setUrl(m_api.getMovieUrl(id, m_meta.defaultLocale, TmdbApi::ApiMovieDetails::CASTS));
        QNetworkReply* const reply = m_network.getWithWatcher(request);
        reply->setProperty("storage", QVariant::fromValue(movie));
        reply->setProperty("infosToLoad", QVariant::fromValue(infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbMovie::loadCastsFinished);
    }

    // Trailers
    if (infos.contains(MovieScraperInfo::Trailer)) {
        loadsLeft.append(ScraperData::Trailers);
        request.setUrl(m_api.getMovieUrl(id, m_meta.defaultLocale, TmdbApi::ApiMovieDetails::TRAILERS));
        QNetworkReply* const reply = m_network.getWithWatcher(request);
        reply->setProperty("storage", QVariant::fromValue(movie));
        reply->setProperty("infosToLoad", QVariant::fromValue(infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbMovie::loadTrailersFinished);
    }

    // Images
    if (infos.contains(MovieScraperInfo::Poster) || infos.contains(MovieScraperInfo::Backdrop)) {
        loadsLeft.append(ScraperData::Images);
        request.setUrl(m_api.getMovieUrl(id, m_meta.defaultLocale, TmdbApi::ApiMovieDetails::IMAGES));
        QNetworkReply* const reply = m_network.getWithWatcher(request);
        reply->setProperty("storage", QVariant::fromValue(movie));
        reply->setProperty("infosToLoad", QVariant::fromValue(infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbMovie::loadImagesFinished);
    }

    // Releases
    if (infos.contains(MovieScraperInfo::Certification)) {
        loadsLeft.append(ScraperData::Releases);
        request.setUrl(m_api.getMovieUrl(id, m_meta.defaultLocale, TmdbApi::ApiMovieDetails::RELEASES));
        QNetworkReply* const reply = m_network.getWithWatcher(request);
        reply->setProperty("storage", QVariant::fromValue(movie));
        reply->setProperty("infosToLoad", QVariant::fromValue(infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbMovie::loadReleasesFinished);
    }

    movie->controller()->setLoadsLeft(loadsLeft);
}

/// Called when the movie infos are downloaded
/// \see TMDb::parseAndAssignInfos
void TmdbMovie::loadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* const movie = reply->property("storage").value<Movie*>();
    const auto infos = reply->property("infosToLoad").value<QSet<MovieScraperInfo>>();
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
        qCWarning(generic) << "Network Error (load)" << reply->errorString();
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
    request.setUrl(m_api.getCollectionUrl(collectionTmdbId.toString(), m_meta.defaultLocale));

    QNetworkReply* const reply = m_network.getWithWatcher(request);
    reply->setProperty("storage", QVariant::fromValue(movie));
    connect(reply, &QNetworkReply::finished, this, &TmdbMovie::loadCollectionFinished);
}

void TmdbMovie::loadCollectionFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* const movie = reply->property("storage").value<Movie*>();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    movie->controller()->removeFromLoadsLeft(ScraperData::Infos);

    const QString msg = QString::fromUtf8(reply->readAll());
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(msg.toUtf8(), &parseError).object();
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(generic) << "Error parsing info json " << parseError.errorString();
        return;
    }

    if (parsedJson.keys().contains("success") && !parsedJson.value("success").toBool()) {
        qCWarning(generic) << "[TMDb] Error message from TMDb:" << parsedJson.value("status_message");
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
    Movie* const movie = reply->property("storage").value<Movie*>();
    const auto infos = reply->property("infosToLoad").value<QSet<MovieScraperInfo>>();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        showNetworkError(*reply);
        qCWarning(generic) << "Network Error (casts)" << reply->errorString();
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
    Movie* const movie = reply->property("storage").value<Movie*>();
    const auto infos = reply->property("infosToLoad").value<QSet<MovieScraperInfo>>();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        const QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        showNetworkError(*reply);
        qCDebug(generic) << "Network Error (trailers)" << reply->errorString();
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
    Movie* movie = reply->property("storage").value<Movie*>();
    const auto infos = reply->property("infosToLoad").value<QSet<MovieScraperInfo>>();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        showNetworkError(*reply);
        qCWarning(generic) << "Network Error (images)" << reply->errorString();
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
    Movie* movie = reply->property("storage").value<Movie*>();
    const auto infos = reply->property("infosToLoad").value<QSet<MovieScraperInfo>>();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        showNetworkError(*reply);
        qCWarning(generic) << "Network Error (releases)" << reply->errorString();
    }
    movie->controller()->removeFromLoadsLeft(ScraperData::Releases);
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
    m_scrapeJob.parseAndAssignInfos(json, movie, infos, language(), country());
}

} // namespace scraper
} // namespace mediaelch
