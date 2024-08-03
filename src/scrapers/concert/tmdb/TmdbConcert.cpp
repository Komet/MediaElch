#include "scrapers/concert/tmdb/TmdbConcert.h"

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/concert/tmdb/TmdbConcertConfiguration.h"
#include "scrapers/concert/tmdb/TmdbConcertSearchJob.h"
#include "ui/main/MainWindow.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace mediaelch {
namespace scraper {

TmdbConcert::TmdbConcert(QObject* parent) :
    ConcertScraper(parent),
    m_apiKey{"5d832bdf69dcb884922381ab01548d5b"},
    m_locale{"en"},
    m_baseUrl{"http://image.tmdb.org/t/p/"}
{
    m_meta.identifier = TmdbConcert::ID;
    m_meta.name = "TMDB Concerts";
    m_meta.description = tr("The Movie Database (TMDB) is a community built movie and TV database. "
                            "Every piece of data has been added by our amazing community dating back to 2008. "
                            "TMDB's strong international focus and breadth of data is largely unmatched and "
                            "something we're incredibly proud of. Put simply, we live and breathe community "
                            "and that's precisely what makes us different.");
    m_meta.website = "https://www.themoviedb.org/";
    m_meta.termsOfService = "https://www.themoviedb.org/terms-of-use";
    m_meta.privacyPolicy = "https://www.themoviedb.org/privacy-policy";
    m_meta.help = "https://www.themoviedb.org/talk";
    m_meta.supportedDetails = {            //
        ConcertScraperInfo::Title,         //
        ConcertScraperInfo::Tagline,       //
        ConcertScraperInfo::Rating,        //
        ConcertScraperInfo::Released,      //
        ConcertScraperInfo::Runtime,       //
        ConcertScraperInfo::Certification, //
        ConcertScraperInfo::Trailer,       //
        ConcertScraperInfo::Overview,      //
        ConcertScraperInfo::Poster,        //
        ConcertScraperInfo::Backdrop,      //
        ConcertScraperInfo::Genres,        //
        ConcertScraperInfo::ExtraArts};

    m_meta.supportedLanguages = TmdbConcertConfiguration::supportedLanguages();
    m_meta.defaultLocale = TmdbConcertConfiguration::defaultLocale();

    connect(&m_api, &TmdbApi::initialized, this, [this](bool wasSuccessful) { emit initialized(wasSuccessful, this); });

    setup();
}

TmdbConcert::~TmdbConcert() = default;

const ConcertScraper::ScraperMeta& TmdbConcert::meta() const
{
    return m_meta;
}

void TmdbConcert::initialize()
{
    m_api.initialize();
}

bool TmdbConcert::isInitialized() const
{
    return m_api.isInitialized();
}

ConcertSearchJob* TmdbConcert::search(ConcertSearchJob::Config config)
{
    return new TmdbConcertSearchJob(m_api, config, this);
}

mediaelch::network::NetworkManager* TmdbConcert::network()
{
    return &m_network;
}

void TmdbConcert::setup()
{
    QUrl url(QString("https://api.themoviedb.org/3/configuration?api_key=%1").arg(m_apiKey));
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);
    QNetworkReply* reply = network()->getWithWatcher(request);
    connect(reply, &QNetworkReply::finished, this, &TmdbConcert::setupFinished);
}

QString TmdbConcert::localeForTmdb() const
{
    return m_locale.name().replace('_', '-');
}

/**
 * \return Two letter language code (lowercase)
 */
QString TmdbConcert::language() const
{
    return m_locale.name().split('_').first();
}

/**
 * \return Two or three letter country code (uppercase)
 */
QString TmdbConcert::country() const
{
    return m_locale.name().split('_').last();
}

/**
 * \brief Called when setup parameters were got
 *        Parses json and assigns the baseUrl
 */
void TmdbConcert::setupFinished()
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
        qCWarning(generic) << "Error parsing TMDB setup json " << parseError.errorString();
        return;
    }

    const auto imagesObject = parsedJson.value("images").toObject();
    m_baseUrl = imagesObject.value("base_url").toString();
    qCDebug(generic) << "TMDB base url:" << m_baseUrl;
}

/**
 * \brief Starts network requests to download infos from TMDB
 * \param id TMDB movie ID
 * \param concert Concert object
 * \param infos List of infos to load
 * \see TmdbConcert::loadFinished
 * \see TmdbConcert::loadCastsFinished
 * \see TmdbConcert::loadTrailersFinished
 * \see TmdbConcert::loadImagesFinished
 * \see TmdbConcert::loadReleasesFinished
 */
void TmdbConcert::loadData(TmdbId id, Concert* concert, QSet<ConcertScraperInfo> infos)
{
    qCDebug(generic) << "Entered, id=" << id << "concert=" << concert->title();
    concert->setTmdbId(id);
    concert->clear(infos);

    QUrl url;
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(QUrl{});

    QVector<ScraperData> loadsLeft;

    // Infos
    {
        loadsLeft.append(ScraperData::Infos);
        url.setUrl(QStringLiteral("https://api.themoviedb.org/3/movie/%1?api_key=%2&language=%3")
                       .arg(id.toString(), m_apiKey, localeForTmdb()));
        request.setUrl(url);
        QNetworkReply* reply = network()->getWithWatcher(request);
        reply->setProperty("storage", QVariant::fromValue(concert));
        reply->setProperty("infosToLoad", QVariant::fromValue(infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbConcert::loadFinished);
    }

    // Trailers
    if (infos.contains(ConcertScraperInfo::Trailer)) {
        loadsLeft.append(ScraperData::Trailers);
        url.setUrl(QString("https://api.themoviedb.org/3/movie/%1/trailers?api_key=%2").arg(id.toString(), m_apiKey));
        request.setUrl(url);
        QNetworkReply* reply = network()->getWithWatcher(request);
        reply->setProperty("storage", QVariant::fromValue(concert));
        reply->setProperty("infosToLoad", QVariant::fromValue(infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbConcert::loadTrailersFinished);
    }

    // Images
    if (infos.contains(ConcertScraperInfo::Poster) || infos.contains(ConcertScraperInfo::Backdrop)) {
        loadsLeft.append(ScraperData::Images);
        url.setUrl(QString("https://api.themoviedb.org/3/movie/%1/images?api_key=%2").arg(id.toString(), m_apiKey));
        request.setUrl(url);
        QNetworkReply* reply = network()->getWithWatcher(request);
        reply->setProperty("storage", QVariant::fromValue(concert));
        reply->setProperty("infosToLoad", QVariant::fromValue(infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbConcert::loadImagesFinished);
    }

    // Releases
    if (infos.contains(ConcertScraperInfo::Certification)) {
        loadsLeft.append(ScraperData::Releases);
        url.setUrl(QString("https://api.themoviedb.org/3/movie/%1/releases?api_key=%2").arg(id.toString(), m_apiKey));
        request.setUrl(url);
        QNetworkReply* reply = network()->getWithWatcher(request);
        reply->setProperty("storage", QVariant::fromValue(concert));
        reply->setProperty("infosToLoad", QVariant::fromValue(infos));
        connect(reply, &QNetworkReply::finished, this, &TmdbConcert::loadReleasesFinished);
    }
    concert->controller()->setLoadsLeft(loadsLeft);
}

/**
 * \brief Called when the concert infos are downloaded
 * \see TmdbConcert::parseAndAssignInfos
 */
void TmdbConcert::loadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Concert* concert = reply->property("storage").value<Concert*>();
    QSet<ConcertScraperInfo> infos = reply->property("infosToLoad").value<QSet<ConcertScraperInfo>>();
    reply->deleteLater();
    if (concert == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, concert, infos);
    } else {
        qCWarning(generic) << "Network Error (load)" << reply->errorString();
    }
    concert->controller()->removeFromLoadsLeft(ScraperData::Infos);
}

/**
 * \brief Called when the concert trailers are downloaded
 * \see TmdbConcert::parseAndAssignInfos
 */
void TmdbConcert::loadTrailersFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Concert* concert = reply->property("storage").value<Concert*>();
    QSet<ConcertScraperInfo> infos = reply->property("infosToLoad").value<QSet<ConcertScraperInfo>>();
    reply->deleteLater();
    if (concert == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, concert, infos);
    } else {
        qCDebug(generic) << "Network Error (trailers)" << reply->errorString();
    }
    concert->controller()->removeFromLoadsLeft(ScraperData::Trailers);
}

/**
 * \brief Called when the concert images are downloaded
 * \see TmdbConcert::parseAndAssignInfos
 */
void TmdbConcert::loadImagesFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Concert* concert = reply->property("storage").value<Concert*>();
    QSet<ConcertScraperInfo> infos = reply->property("infosToLoad").value<QSet<ConcertScraperInfo>>();
    reply->deleteLater();
    if (concert == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, concert, infos);
    } else {
        qCWarning(generic) << "Network Error (images)" << reply->errorString();
    }
    concert->controller()->removeFromLoadsLeft(ScraperData::Images);
}

/**
 * \brief Called when the concert releases are downloaded
 * \see TmdbConcert::parseAndAssignInfos
 */
void TmdbConcert::loadReleasesFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Concert* concert = reply->property("storage").value<Concert*>();
    QSet<ConcertScraperInfo> infos = reply->property("infosToLoad").value<QSet<ConcertScraperInfo>>();
    reply->deleteLater();
    if (concert == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, concert, infos);
    } else {
        qCWarning(generic) << "Network Error (releases)" << reply->errorString();
    }
    concert->controller()->removeFromLoadsLeft(ScraperData::Releases);
}

/**
 * \brief Parses JSON data and assigns it to the given concert object
 *        Handles all types of data from TMDB (info, releases, trailers, images)
 * \param json JSON data
 * \param concert Concert object
 * \param infos List of infos to load
 */
void TmdbConcert::parseAndAssignInfos(QString json, Concert* concert, QSet<ConcertScraperInfo> infos)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(generic) << "Error parsing concert info json " << parseError.errorString();
        return;
    }

    // Infos
    if (!parsedJson.value("imdb_id").toString().isEmpty()) {
        concert->setImdbId(ImdbId(parsedJson.value("imdb_id").toString()));
    }
    if (infos.contains(ConcertScraperInfo::Title)) {
        if (!parsedJson.value("title").toString().isEmpty()) {
            concert->setTitle(parsedJson.value("title").toString());
        }
        if (!parsedJson.value("original_title").toString().isEmpty()) {
            concert->setOriginalTitle(parsedJson.value("original_title").toString());
        }
    }
    if (infos.contains(ConcertScraperInfo::Overview)) {
        const auto overviewStr = parsedJson.value("overview").toString();
        if (!overviewStr.isEmpty()) {
            concert->setOverview(overviewStr);
        }
    }
    if (infos.contains(ConcertScraperInfo::Rating) && parsedJson.value("vote_average").toDouble(-1) >= 0) {
        Rating rating;
        rating.source = "themoviedb";
        rating.rating = parsedJson.value("vote_average").toDouble();
        rating.voteCount = parsedJson.value("vote_count").toInt();
        concert->ratings().setOrAddRating(rating);
        concert->setChanged(true);
    }
    if (infos.contains(ConcertScraperInfo::Tagline) && !parsedJson.value("tagline").toString().isEmpty()) {
        concert->setTagline(parsedJson.value("tagline").toString());
    }
    if (infos.contains(ConcertScraperInfo::Released) && !parsedJson.value("release_date").toString().isEmpty()) {
        concert->setReleased(QDate::fromString(parsedJson.value("release_date").toString(), "yyyy-MM-dd"));
    }
    if (infos.contains(ConcertScraperInfo::Runtime) && parsedJson.value("runtime").toInt(-1) >= 0) {
        concert->setRuntime(std::chrono::minutes(parsedJson.value("runtime").toInt()));
    }
    if (infos.contains(ConcertScraperInfo::Genres) && parsedJson.value("genres").isArray()) {
        const auto genres = parsedJson.value("genres").toArray();
        for (const auto& it : genres) {
            const auto genre = it.toObject();
            if (genre.value("id").toInt(-1) == -1) {
                continue;
            }
            concert->addGenre(helper::mapGenre(genre.value("name").toString()));
        }
    }

    // Trailers
    if (infos.contains(ConcertScraperInfo::Trailer) && parsedJson.value("youtube").isArray()) {
        // The trailer listed first is most likely also the best.
        const auto firstTrailer = parsedJson.value("youtube").toArray().first().toObject();
        if (!firstTrailer.value("source").toString().isEmpty()) {
            const QString youtubeSrc = firstTrailer.value("source").toString();
            concert->setTrailer(
                QUrl(helper::formatTrailerUrl(QStringLiteral("https://www.youtube.com/watch?v=%1").arg(youtubeSrc))));
        }
    }

    // Images
    if (infos.contains(ConcertScraperInfo::Backdrop) && parsedJson.value("backdrops").isArray()) {
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
            concert->addBackdrop(b);
        }
    }

    if (infos.contains(ConcertScraperInfo::Poster) && parsedJson.value("posters").isArray()) {
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
            concert->addPoster(b);
        }
    }

    // Releases
    if (infos.contains(ConcertScraperInfo::Certification) && parsedJson.value("countries").isArray()) {
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

        if (us.isValid()) {
            concert->setCertification(helper::mapCertification(us));

        } else if (gb.isValid()) {
            concert->setCertification(helper::mapCertification(gb));

        } else if (locale.isValid()) {
            concert->setCertification(helper::mapCertification(locale));
        }
    }
}

} // namespace scraper
} // namespace mediaelch
