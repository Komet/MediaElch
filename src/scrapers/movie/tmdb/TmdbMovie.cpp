#include "scrapers/movie/tmdb/TmdbMovie.h"

#include <QDebug>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QTextDocument>
#include <QUrlQuery>

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/NetworkReplyWatcher.h"
#include "settings/Settings.h"
#include "ui/main/MainWindow.h"

namespace mediaelch {
namespace scraper {

TmdbMovie::TmdbMovie(QObject* parent) : MovieScraper(parent), m_api("5d832bdf69dcb884922381ab01548d5b")
{
    m_info.name = "TheMovieDb";
    m_info.identifier = "tmdb";
    m_info.description = "The Movie Database (TMDb) is a community built movie and TV database.";
    m_info.website = "https://www.themoviedb.org";
    m_info.privacyPolicy = "https://www.themoviedb.org/privacy-policy";
    m_info.help = "https://www.themoviedb.org/talk";
    m_info.isAdultScraper = false;
    m_info.scraperSupports = QVector<MovieScraperInfos>{MovieScraperInfos::Title,
        MovieScraperInfos::Tagline,
        MovieScraperInfos::Rating,
        MovieScraperInfos::Released,
        MovieScraperInfos::Runtime,
        MovieScraperInfos::Certification,
        MovieScraperInfos::Trailer,
        MovieScraperInfos::Overview,
        MovieScraperInfos::Poster,
        MovieScraperInfos::Backdrop,
        MovieScraperInfos::Actors,
        MovieScraperInfos::Genres,
        MovieScraperInfos::Studios,
        MovieScraperInfos::Countries,
        MovieScraperInfos::Director,
        MovieScraperInfos::Writer,
        MovieScraperInfos::Set};
    m_info.supportedLanguages = TmdbApi::supportedLanguages();
}

const MovieScraper::ScraperInfo& TmdbMovie::info() const
{
    return m_info;
}

TmdbMovieSearchJob* TmdbMovie::search(MovieSearchJob::Config config)
{
    auto* movieSearch = new TmdbMovieSearchJob(m_api, std::move(config), this);
    movieSearch->execute();
    return movieSearch;
}

TmdbMovieScrapeJob* TmdbMovie::scrape(Movie& movie, MovieScrapeJob::Config config)
{
    auto* scrapeJob = new TmdbMovieScrapeJob(*this, m_api, movie, std::move(config), this);
    scrapeJob->execute();
    return scrapeJob;
}

/// \brief Loads the setup parameters from TmdbMovie
/// \see TmdbMovie::setupFinished()
void TmdbMovie::initialize()
{
    qDebug() << "[TmdbMovie] Request setup from server";
    QUrl url(QStringLiteral("https://api.themoviedb.org/3/configuration?api_key=%1").arg(m_api.key()));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply* const reply = m_api.network().get(request);
    new NetworkReplyWatcher(this, reply);
    connect(reply, &QNetworkReply::finished, this, &TmdbMovie::handleConfigurationResponse);
}

bool TmdbMovie::isInitialized() const
{
    return m_configLoaded;
}

/// \brief Called when setup parameters were received.
///        Parses JSON and assigns the base URL.
void TmdbMovie::handleConfigurationResponse()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "TODO";
        return;
    }
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }

    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(reply->readAll(), &parseError).object();
    reply->deleteLater();
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[TmdbMovie] Error parsing setup json:" << parseError.errorString();
        return;
    }

    const auto imagesObject = parsedJson.value("images").toObject();
    m_api.setImageBaseUrl(imagesObject.value("base_url").toString());
    qInfo() << "[TmdbMovie] Config loaded | base url:" << m_api.imageBaseUrl();

    m_configLoaded = true;
    emit initialized();
}


} // namespace scraper
} // namespace mediaelch
