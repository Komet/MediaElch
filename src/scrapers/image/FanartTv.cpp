#include "FanartTv.h"

#include <QDebug>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>

#include "data/Storage.h"
#include "globals/Manager.h"
#include "network/NetworkRequest.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "ui/main/MainWindow.h"

namespace mediaelch {
namespace scraper {

QString FanartTv::ID = "images.fanarttv";

FanartTv::FanartTv(QObject* parent) : ImageProvider(parent)
{
    m_meta.identifier = ID;
    m_meta.name = "Fanart.tv";
    m_meta.description = tr("FanartTV is a community-driven image provider.");
    m_meta.website = "https://fanart.tv";
    m_meta.termsOfService = "https://fanart.tv/terms-and-conditions/";
    m_meta.privacyPolicy = "https://fanart.tv/privacy-policy/";
    m_meta.help = "https://forum.fanart.tv/";
    m_meta.supportedImageTypes = {ImageType::MovieBackdrop,
        ImageType::MovieLogo,
        ImageType::MovieClearArt,
        ImageType::MovieCdArt,
        ImageType::MovieBanner,
        ImageType::MovieThumb,
        ImageType::MoviePoster,
        ImageType::TvShowClearArt,
        ImageType::TvShowBackdrop,
        ImageType::TvShowBanner,
        ImageType::TvShowThumb,
        ImageType::TvShowSeasonThumb,
        ImageType::TvShowSeasonPoster,
        ImageType::TvShowLogos,
        ImageType::TvShowCharacterArt,
        ImageType::TvShowPoster,
        ImageType::ConcertBackdrop,
        ImageType::ConcertLogo,
        ImageType::ConcertClearArt,
        ImageType::ConcertCdArt};
    // Multiple languages, but no way to query for it and also no official list of languages.
    m_meta.supportedLanguages = {
        "bg",
        "zh",
        "hr",
        "cs",
        "da",
        "nl",
        "en",
        "fi",
        "fr",
        "de",
        "el",
        "he",
        "hu",
        "it",
        "ja",
        "ko",
        "no",
        "pl",
        "pt",
        "ru",
        "sl",
        "es",
        "sv",
        "tr",
    };
    m_meta.defaultLocale = "en";

    m_preferredDiscType = "BluRay";

    m_widget = new QWidget(MainWindow::instance());
    m_box = new QComboBox(m_widget);
    m_box->addItem(tr("Bulgarian"), "bg");
    m_box->addItem(tr("Chinese"), "zh");
    m_box->addItem(tr("Croatian"), "hr");
    m_box->addItem(tr("Czech"), "cs");
    m_box->addItem(tr("Danish"), "da");
    m_box->addItem(tr("Dutch"), "nl");
    m_box->addItem(tr("English"), "en");
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

    m_discBox = new QComboBox(m_widget);
    m_discBox->addItem("3D", "3D");
    m_discBox->addItem(tr("Blu-ray"), "BluRay");
    m_discBox->addItem(tr("DVD"), "DVD");
    m_personalApiKeyEdit = new QLineEdit(m_widget);
    auto* layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->addWidget(new QLabel(tr("Preferred Disc Type")), 1, 0);
    layout->addWidget(m_discBox, 1, 1);
    layout->addWidget(new QLabel(tr("Personal API key")), 2, 0);
    layout->addWidget(m_personalApiKeyEdit, 2, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);

    m_apiKey = "842f7a5d1cc7396f142b8dd47c4ba42b";
    m_tmdb = new TmdbMovie(this);
    connect(m_tmdb, &TmdbMovie::searchDone, this, &FanartTv::onSearchMovieFinished);
}

const ImageProvider::ScraperMeta& FanartTv::meta() const
{
    return m_meta;
}

/**
 * \brief Just returns a pointer to the scrapers network access manager
 * \return Network Access Manager
 */
mediaelch::network::NetworkManager* FanartTv::network()
{
    return &m_network;
}

/**
 * \brief Searches for a movie
 * \param searchStr The Movie name/search string
 * \param limit Number of results, if zero, all results are returned
 * \see FanartTv::onSearchMovieFinished
 */
void FanartTv::searchMovie(QString searchStr, int limit)
{
    m_searchResultLimit = limit;
    // TODO: Set TMDb language
    m_tmdb->search(searchStr);
}

/**
 * \brief Searches for a concert
 * \param searchStr The Concert name/search string
 * \param limit Number of results, if zero, all results are returned
 * \see FanartTv::searchMovie
 */
void FanartTv::searchConcert(QString searchStr, int limit)
{
    searchMovie(searchStr, limit);
}

/**
 * \brief Called when the search result was downloaded
 *        Emits "sigSearchDone" if there are no more pages in the result set
 * \param results List of results from scraper
 * \see TMDb::parseSearch
 */
void FanartTv::onSearchMovieFinished(QVector<ScraperSearchResult> results, ScraperError error)
{
    if (m_searchResultLimit == 0) {
        emit sigSearchDone(results, error);
    } else {
        emit sigSearchDone(results.mid(0, m_searchResultLimit), error);
    }
}

/**
 * \brief Loads given image types
 */
void FanartTv::movieImages(Movie* movie, TmdbId tmdbId, QVector<ImageType> types)
{
    loadMovieData(tmdbId, types, movie);
}

/**
 * \brief Load movie posters
 */
void FanartTv::moviePosters(TmdbId tmdbId)
{
    loadMovieData(tmdbId, ImageType::MoviePoster);
}

/**
 * \brief Load movie backdrops
 */
void FanartTv::movieBackdrops(TmdbId tmdbId)
{
    loadMovieData(tmdbId, ImageType::MovieBackdrop);
}

/**
 * \brief Load movie logos
 * \param tmdbId The Movie DB id
 */
void FanartTv::movieLogos(TmdbId tmdbId)
{
    loadMovieData(tmdbId, ImageType::MovieLogo);
}

void FanartTv::movieBanners(TmdbId tmdbId)
{
    loadMovieData(tmdbId, ImageType::MovieBanner);
}

void FanartTv::movieThumbs(TmdbId tmdbId)
{
    loadMovieData(tmdbId, ImageType::MovieThumb);
}

/**
 * \brief Load movie clear arts
 * \param tmdbId The Movie DB id
 */
void FanartTv::movieClearArts(TmdbId tmdbId)
{
    loadMovieData(tmdbId, ImageType::MovieClearArt);
}

/**
 * \brief Load movie cd arts
 * \param tmdbId The Movie DB id
 */
void FanartTv::movieCdArts(TmdbId tmdbId)
{
    loadMovieData(tmdbId, ImageType::MovieCdArt);
}

/**
 * \brief Loads given image types
 */
void FanartTv::concertImages(Concert* concert, TmdbId tmdbId, QVector<ImageType> types)
{
    loadConcertData(tmdbId, types, concert);
}

/**
 * \brief Would load concert posters (not supported by fanart.tv)
 */
void FanartTv::concertPosters(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load concert backdrops
 */
void FanartTv::concertBackdrops(TmdbId tmdbId)
{
    loadMovieData(tmdbId, ImageType::ConcertBackdrop);
}

/**
 * \brief Load concert logos
 * \param tmdbId The Movie DB id
 */
void FanartTv::concertLogos(TmdbId tmdbId)
{
    loadMovieData(tmdbId, ImageType::ConcertLogo);
}

/**
 * \brief Load concert clear arts
 * \param tmdbId The Movie DB id
 */
void FanartTv::concertClearArts(TmdbId tmdbId)
{
    loadMovieData(tmdbId, ImageType::ConcertClearArt);
}

/**
 * \brief Load concert cd arts
 * \param tmdbId The Movie DB id
 */
void FanartTv::concertCdArts(TmdbId tmdbId)
{
    loadMovieData(tmdbId, ImageType::ConcertCdArt);
}

void FanartTv::loadMovieData(TmdbId tmdbId, ImageType type)
{
    QUrl url = QStringLiteral("https://webservice.fanart.tv/v3/movies/%1?%2").arg(tmdbId.toString(), keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    qDebug() << "[FanartTv] Load movie data:" << url;

    QNetworkReply* reply = network()->get(request);
    reply->setProperty("infoToLoad", static_cast<int>(type));
    connect(reply, &QNetworkReply::finished, this, &FanartTv::onLoadMovieDataFinished);
}

void FanartTv::loadMovieData(TmdbId tmdbId, QVector<ImageType> types, Movie* movie)
{
    QUrl url = QStringLiteral("https://webservice.fanart.tv/v3/movies/%1?%2").arg(tmdbId.toString(), keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    qDebug() << "[FanartTv] Load movie data with image types:"
             << url.toString(QUrl::RemoveQuery); // query not relevant as it only contains the API key

    QNetworkReply* reply = network()->get(request);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, types));
    connect(reply, &QNetworkReply::finished, this, &FanartTv::onLoadAllMovieDataFinished);
}

void FanartTv::loadConcertData(TmdbId tmdbId, QVector<ImageType> types, Concert* concert)
{
    QUrl url = QStringLiteral("https://webservice.fanart.tv/v3/movies/%1?%2").arg(tmdbId.toString(), keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    qDebug() << "[FanartTv] Load concert data with image types:" << url;

    QNetworkReply* reply = network()->get(request);
    reply->setProperty("infosToLoad", Storage::toVariant(reply, types));
    reply->setProperty("storage", Storage::toVariant(reply, concert));
    connect(reply, &QNetworkReply::finished, this, &FanartTv::onLoadAllConcertDataFinished);
}

/**
 * \brief Called when the movie images are downloaded
 * \see TMDbImages::parseMovieData
 */
void FanartTv::onLoadMovieDataFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        const bool notFound = (reply->error() == QNetworkReply::ContentNotFoundError);
        if (notFound) {
            emit sigImagesLoaded({}, mediaelch::replyToScraperError(*reply));
        } else {
            emit sigImagesLoaded(
                {}, {ScraperError::Type::NetworkError, tr("Movie not found on Fanart.tv"), reply->errorString()});
        }
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    QVector<Poster> posters = parseMovieData(msg, ImageType(reply->property("infoToLoad").toInt()));
    emit sigImagesLoaded(posters, {});
}

/**
 * \brief Called when all movie images are downloaded
 * \see TMDbImages::parseMovieData
 */
void FanartTv::onLoadAllMovieDataFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* movie = reply->property("storage").value<Storage*>()->movie();
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit sigMovieImagesLoaded(movie, {});
    }

    QMap<ImageType, QVector<Poster>> posters;
    QString msg = QString::fromUtf8(reply->readAll());
    for (const auto type : reply->property("infosToLoad").value<Storage*>()->imageInfosToLoad()) {
        posters.insert(type, parseMovieData(msg, type));
    }

    emit sigMovieImagesLoaded(movie, posters);
}

/**
 * \brief Called when all concert images are downloaded
 * \see TMDbImages::parseMovieData
 */
void FanartTv::onLoadAllConcertDataFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Concert* concert = reply->property("storage").value<Storage*>()->concert();
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit sigConcertImagesLoaded(concert, {});
    }

    QMap<ImageType, QVector<Poster>> posters;
    QString msg = QString::fromUtf8(reply->readAll());
    for (const auto type : reply->property("infosToLoad").value<Storage*>()->imageInfosToLoad()) {
        posters.insert(type, parseMovieData(msg, type));
    }

    emit sigConcertImagesLoaded(concert, posters);
}

/**
 * \brief Parses JSON data for movies
 * \param json JSON data
 * \param type Type of image (ImageType)
 * \return List of posters
 */
QVector<Poster> FanartTv::parseMovieData(QString json, ImageType type)
{
    QMap<ImageType, QStringList> map;
    // clang-format off
    map.insert(ImageType::MoviePoster,     QStringList() << "movieposter");
    map.insert(ImageType::MovieBackdrop,   QStringList() << "moviebackground");
    map.insert(ImageType::MovieLogo,       QStringList() << "hdmovielogo" << "movielogo");
    map.insert(ImageType::MovieClearArt,   QStringList() << "hdmovieclearart" << "movieart");
    map.insert(ImageType::MovieCdArt,      QStringList() << "moviedisc");
    map.insert(ImageType::MovieBanner,     QStringList() << "moviebanner");
    map.insert(ImageType::MovieThumb,      QStringList() << "moviethumb");
    map.insert(ImageType::ConcertBackdrop, QStringList() << "moviebackground");
    map.insert(ImageType::ConcertLogo,     QStringList() << "hdmovielogo" << "movielogo");
    map.insert(ImageType::ConcertClearArt, QStringList() << "hdmovieclearart" << "movieart");
    map.insert(ImageType::ConcertCdArt,    QStringList() << "moviedisc");
    // clang-format on

    QVector<Poster> posters;

    QJsonParseError parseError{};
    // The JSON contains one object with all URLs to fanart images
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing fanart movie json " << parseError.errorString();
        return posters;
    }

    for (const auto& section : map.value(type)) {
        const auto jsonPosters = parsedJson.value(section).toArray();

        for (const auto& it : jsonPosters) {
            const auto poster = it.toObject();
            Poster b;
            b.thumbUrl = poster.value("url").toString().replace("/fanart/", "/preview/");
            b.originalUrl = poster.value("url").toString();

            const auto discType = poster.value("disc_type").toString();
            b.hint = [&section, &discType] {
                if (section == "hdmovielogo" || section == "hdmovieclearart") {
                    return QStringLiteral("HD");
                }
                if (section == "movielogo" || section == "movieart") {
                    return QStringLiteral("SD");
                }
                if (discType == "bluray") {
                    return QStringLiteral("BluRay");
                }
                if (discType == "dvd") {
                    return QStringLiteral("DVD");
                }
                if (discType == "3d") {
                    return QStringLiteral("3D");
                }
                return QStringLiteral("");
            }();

            b.language = poster.value("lang").toString();
            insertPoster(posters, b, m_meta.defaultLocale.toString(), m_preferredDiscType);
        }
    }

    return posters;
}

/**
 * \brief Searches for a TV show
 * \param searchStr The TV show name/search string
 * \param limit Number of results, if zero, all results are returned
 * \see FanartTv::onSearchTvShowFinished
 */
void FanartTv::searchTvShow(QString searchStr, mediaelch::Locale locale, int limit)
{
    using namespace mediaelch;
    using namespace mediaelch::scraper;
    m_searchResultLimit = limit;

    auto* tvdb = dynamic_cast<TheTvDb*>(Manager::instance()->scrapers().tvScraper(TheTvDb::ID));
    if (tvdb == nullptr) {
        qFatal("[FanartTv] Cast to TheTvDb* failed!");
    }
    ShowSearchJob::Config config{searchStr, locale, false};
    auto* searchJob = tvdb->search(config);
    connect(searchJob, &ShowSearchJob::sigFinished, this, &FanartTv::onSearchTvShowFinished, Qt::UniqueConnection);
    searchJob->execute();
}

void FanartTv::onSearchTvShowFinished(ShowSearchJob* searchJob)
{
    const auto results = toOldScraperSearchResult(searchJob->results());
    const auto error = searchJob->error();
    searchJob->deleteLater();

    if (m_searchResultLimit == 0) {
        emit sigSearchDone(results, error);
    } else {
        emit sigSearchDone(results.mid(0, m_searchResultLimit), error);
    }
}

/**
 * \brief Loads given image types
 */
void FanartTv::tvShowImages(TvShow* show, TvDbId tvdbId, QVector<ImageType> types, const mediaelch::Locale& locale)
{
    Q_UNUSED(locale)
    loadTvShowData(tvdbId, types, show);
}

void FanartTv::loadTvShowData(TvDbId tvdbId, ImageType type, SeasonNumber season)
{
    QUrl url = QStringLiteral("https://webservice.fanart.tv/v3/tv/%1?%2").arg(tvdbId.toString(), keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    QNetworkReply* reply = network()->get(request);
    reply->setProperty("infoToLoad", static_cast<int>(type));
    reply->setProperty("season", season.toInt());
    connect(reply, &QNetworkReply::finished, this, &FanartTv::onLoadTvShowDataFinished);
}

void FanartTv::loadTvShowData(TvDbId tvdbId, QVector<ImageType> types, TvShow* show)
{
    QUrl url = QStringLiteral("https://webservice.fanart.tv/v3/tv/%1?%2").arg(tvdbId.toString(), keyParameter());
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);

    QNetworkReply* reply = network()->get(request);
    reply->setProperty("infosToLoad", Storage::toVariant(reply, types));
    reply->setProperty("storage", Storage::toVariant(reply, show));
    connect(reply, &QNetworkReply::finished, this, &FanartTv::onLoadAllTvShowDataFinished);
}

/**
 * \brief Called when the TV show images are downloaded
 * \see TMDbImages::parseTvShowData
 */
void FanartTv::onLoadTvShowDataFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        const bool notFound = (reply->error() == QNetworkReply::ContentNotFoundError);
        if (notFound) {
            emit sigImagesLoaded({}, mediaelch::replyToScraperError(*reply));
        } else {
            emit sigImagesLoaded(
                {}, {ScraperError::Type::NetworkError, tr("TV show not found on Fanart.tv"), reply->errorString()});
        }
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    QVector<Poster> posters = parseTvShowData(
        msg, ImageType(reply->property("infoToLoad").toInt()), SeasonNumber(reply->property("season").toInt()));

    emit sigImagesLoaded(posters, {});
}

/**
 * \brief Called when all TV show images are downloaded
 * \see TMDbImages::parseTvShowData
 */
void FanartTv::onLoadAllTvShowDataFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    TvShow* show = reply->property("storage").value<Storage*>()->show();
    reply->deleteLater();
    QMap<ImageType, QVector<Poster>> posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        for (const auto type : reply->property("infosToLoad").value<Storage*>()->imageInfosToLoad()) {
            posters.insert(type, parseTvShowData(msg, type));
        }
    }
    reply->deleteLater();
    emit sigTvShowImagesLoaded(show, posters);
}

/**
 * \brief Load TV show posters
 * \param tvdbId The TV DB id
 */
void FanartTv::tvShowPosters(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(locale)
    loadTvShowData(tvdbId, ImageType::TvShowPoster);
}

/**
 * \brief Load TV show backdrops
 * \param tvdbId The TV DB id
 */
void FanartTv::tvShowBackdrops(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(locale)
    loadTvShowData(tvdbId, ImageType::TvShowBackdrop);
}

/**
 * \brief Load TV show logos
 * \param tvdbId The TV DB id
 */
void FanartTv::tvShowLogos(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(locale)
    loadTvShowData(tvdbId, ImageType::TvShowLogos);
}

void FanartTv::tvShowThumbs(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(locale)
    loadTvShowData(tvdbId, ImageType::TvShowThumb);
}

/**
 * \brief Load TV show clear arts
 * \param tvdbId The TV DB id
 */
void FanartTv::tvShowClearArts(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(locale)
    loadTvShowData(tvdbId, ImageType::TvShowClearArt);
}

/**
 * \brief Load TV show character arts
 * \param tvdbId The TV DB id
 */
void FanartTv::tvShowCharacterArts(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(locale)
    loadTvShowData(tvdbId, ImageType::TvShowCharacterArt);
}

/**
 * \brief Load TV show banners
 * \param tvdbId The TV DB id
 */
void FanartTv::tvShowBanners(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(locale)
    loadTvShowData(tvdbId, ImageType::TvShowBanner);
}

/**
 * \brief Load TV show thumbs
 * \param tvdbId The TV DB id
 * \param season Season number
 * \param episode Episode number
 */
void FanartTv::tvShowEpisodeThumb(TvDbId tvdbId,
    SeasonNumber season,
    EpisodeNumber episode,
    const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show season
 * \param tvdbId The TV DB id
 * \param season Season number
 */
void FanartTv::tvShowSeason(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(locale);
    loadTvShowData(tvdbId, ImageType::TvShowSeasonPoster, season);
}

void FanartTv::tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale);
}

void FanartTv::tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale);
}

void FanartTv::tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(locale);
    loadTvShowData(tvdbId, ImageType::TvShowSeasonThumb, season);
}

/**
 * \brief Parses JSON data for TV shows
 * \param json JSON data
 * \param type Type of image (ImageType)
 * \return List of posters
 */
QVector<Poster> FanartTv::parseTvShowData(QString json, ImageType type, SeasonNumber season)
{
    QMap<ImageType, QStringList> map;

    // clang-format off
    map.insert(ImageType::TvShowBackdrop,     QStringList() << "showbackground");
    map.insert(ImageType::TvShowLogos,        QStringList() << "hdtvlogo" << "clearlogo");
    map.insert(ImageType::TvShowClearArt,     QStringList() << "hdclearart" << "clearart");
    map.insert(ImageType::TvShowBanner,       QStringList() << "tvbanner");
    map.insert(ImageType::TvShowCharacterArt, QStringList() << "characterart");
    map.insert(ImageType::TvShowThumb,        QStringList() << "tvthumb");
    map.insert(ImageType::TvShowSeasonThumb,  QStringList() << "seasonthumb");
    map.insert(ImageType::TvShowSeasonPoster, QStringList() << "seasonposter");
    map.insert(ImageType::TvShowPoster,       QStringList() << "tvposter");
    // clang-format on

    QVector<Poster> posters;

    QJsonParseError parseError{};
    // The JSON contains one object with all URLs to fanart images
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing fanart TV show json " << parseError.errorString();
        return posters;
    }

    for (const QString& section : map.value(type)) {
        const auto jsonPosters = parsedJson.value(section).toArray();

        for (const auto& it : jsonPosters) {
            const auto poster = it.toObject();

            if (poster.value("url").toString().isEmpty()) {
                continue;
            }

            if ((type == ImageType::TvShowSeasonThumb || type == ImageType::TvShowSeasonPoster)
                && season != SeasonNumber::NoSeason && !poster.value("season").toString().isEmpty()
                && poster.value("season").toString().toInt() != season.toInt()) {
                continue;
            }

            Poster b;
            b.thumbUrl = poster.value("url").toString().replace("/fanart/", "/preview/");
            b.originalUrl = poster.value("url").toString();
            b.season = SeasonNumber(poster.value("season").toString().toInt());

            const auto discType = poster.value("disc_type").toString();

            b.hint = [&section, &discType] {
                if (section == "hdtvlogo" || section == "hdclearart") {
                    return QStringLiteral("HD");
                }
                if (section == "clearlogo" || section == "clearart") {
                    return QStringLiteral("SD");
                }
                if (discType == "bluray") {
                    return QStringLiteral("BluRay");
                }
                if (discType == "dvd") {
                    return QStringLiteral("DVD");
                }
                if (discType == "3d") {
                    return QStringLiteral("3D");
                }
                return QStringLiteral("");
            }();
            b.language = poster.value("lang").toString();
            insertPoster(posters, b, m_meta.defaultLocale.toString(), m_preferredDiscType);
        }
    }

    return posters;
}

bool FanartTv::hasSettings() const
{
    return true;
}

void FanartTv::loadSettings(ScraperSettings& settings)
{
    m_meta.defaultLocale = settings.language(m_meta.defaultLocale);
    m_preferredDiscType = settings.valueString("DiscType", "BluRay");
    m_personalApiKey = settings.valueString("PersonalApiKey", "");
    for (int i = 0, n = m_box->count(); i < n; ++i) {
        if (m_box->itemData(i).toString() == m_meta.defaultLocale) {
            m_box->setCurrentIndex(i);
        }
    }
    for (int i = 0, n = m_discBox->count(); i < n; ++i) {
        if (m_discBox->itemData(i).toString() == m_preferredDiscType) {
            m_discBox->setCurrentIndex(i);
        }
    }
    m_personalApiKeyEdit->setText(m_personalApiKey);
}

void FanartTv::saveSettings(ScraperSettings& settings)
{
    m_meta.defaultLocale = m_box->itemData(m_box->currentIndex()).toString();
    m_preferredDiscType = m_discBox->itemData(m_discBox->currentIndex()).toString();
    m_personalApiKey = m_personalApiKeyEdit->text();
    settings.setLanguage(m_meta.defaultLocale.toString());
    settings.setString("DiscType", m_preferredDiscType);
    settings.setString("PersonalApiKey", m_personalApiKey);
}

QWidget* FanartTv::settingsWidget()
{
    return m_widget;
}

void FanartTv::insertPoster(QVector<Poster>& posters, Poster b, QString language, QString preferredDiscType)
{
    int lastInPreferredLangAndHd = -1;
    int lastInPreferredLang = -1;
    int lastHd = -1;

    for (int i = 0, n = posters.count(); i < n; ++i) {
        if (posters[i].language == language && (posters[i].hint == "HD" || posters[i].hint == preferredDiscType)) {
            lastInPreferredLangAndHd = i;
        }
        if (posters[i].language == language || posters[i].language.isEmpty()) {
            // if "language" is empty then the poster is language-agnostic
            lastInPreferredLang = i;
        }
        if (posters[i].hint == "HD" || posters[i].hint == preferredDiscType) {
            lastHd = i;
        }
    }

    if (b.language == language && (b.hint == "HD" || b.hint == preferredDiscType)) {
        // lastInPreferredLangAndHd < n
        posters.insert(lastInPreferredLangAndHd + 1, b);
    } else if (b.language == language) {
        posters.insert(lastInPreferredLang + 1, b);
    } else if (b.hint == "HD" || b.hint == preferredDiscType) {
        // lastHd < n
        posters.insert(lastHd + 1, b);
    } else {
        posters.append(b);
    }
}

QString FanartTv::keyParameter()
{
    return (!m_personalApiKey.isEmpty()) ? QString("api_key=%1&client_key=%2").arg(m_apiKey).arg(m_personalApiKey)
                                         : QString("api_key=%1").arg(m_apiKey);
}

void FanartTv::searchAlbum(QString artistName, QString searchStr, int limit)
{
    Q_UNUSED(artistName);
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTv::searchArtist(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTv::artistFanarts(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void FanartTv::artistLogos(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void FanartTv::artistThumbs(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void FanartTv::albumCdArts(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void FanartTv::albumThumbs(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void FanartTv::artistImages(Artist* artist, MusicBrainzId mbId, QVector<ImageType> types)
{
    Q_UNUSED(artist);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void FanartTv::albumImages(Album* album, MusicBrainzId mbId, QVector<ImageType> types)
{
    Q_UNUSED(album);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void FanartTv::albumBooklets(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

} // namespace scraper
} // namespace mediaelch
