#include "scrapers/movie/tmdb/TmdbMovieScrapeJob.h"

#include "globals/Helper.h"
#include "movies/Movie.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTextDocument>

namespace mediaelch {
namespace scraper {

TmdbMovieScrapeJob::TmdbMovieScrapeJob(MovieScraper& scraper,
    TmdbApi& api,
    Movie& movie,
    const MovieScrapeJob::Config& config,
    QObject* parent) :
    MovieScrapeJob(scraper, movie, config, parent), m_api{api}
{
}

void TmdbMovieScrapeJob::execute()
{
    qInfo() << "[TmdbMovieScraperJob] Load movie from TMDb | ID:" << identifier();

    const bool isImdbId = identifier().startsWith("tt");

    if (isImdbId) {
        movie().setId(ImdbId(identifier()));
    } else {
        movie().setTmdbId(TmdbId(identifier()));
    }

    movie().clear(details());

    m_maxDownloads = 0;

    // Infos
    {
        ++m_maxDownloads;
        m_loadsLeft.insert(ScraperData::Infos);
        QUrl url = m_api.movieUrl(identifier(), TmdbApi::TmdbApi::ApiMovieDetails::INFOS);
        QNetworkReply* const reply = m_api.sendGetRequest(url);
        connect(reply, &QNetworkReply::finished, this, &TmdbMovieScrapeJob::loadFinished);
    }

    // Casts
    if (details().contains(MovieScraperInfos::Actors) || details().contains(MovieScraperInfos::Director)
        || details().contains(MovieScraperInfos::Writer)) {
        ++m_maxDownloads;
        m_loadsLeft.insert(ScraperData::Casts);
        QUrl url = m_api.movieUrl(identifier(), TmdbApi::TmdbApi::ApiMovieDetails::CASTS);
        QNetworkReply* const reply = m_api.sendGetRequest(url);
        connect(reply, &QNetworkReply::finished, this, &TmdbMovieScrapeJob::loadCastsFinished);
    }

    // Trailers
    if (details().contains(MovieScraperInfos::Trailer)) {
        ++m_maxDownloads;
        m_loadsLeft.insert(ScraperData::Trailers);
        QUrl url = m_api.movieUrl(identifier(), TmdbApi::TmdbApi::ApiMovieDetails::TRAILERS);
        QNetworkReply* const reply = m_api.sendGetRequest(url);
        connect(reply, &QNetworkReply::finished, this, &TmdbMovieScrapeJob::loadTrailersFinished);
    }

    // Images
    if (details().contains(MovieScraperInfos::Poster) || details().contains(MovieScraperInfos::Backdrop)) {
        ++m_maxDownloads;
        m_loadsLeft.insert(ScraperData::Images);
        QUrl url = m_api.movieUrl(identifier(), TmdbApi::TmdbApi::ApiMovieDetails::IMAGES);
        QNetworkReply* const reply = m_api.sendGetRequest(url);
        connect(reply, &QNetworkReply::finished, this, &TmdbMovieScrapeJob::loadImagesFinished);
    }

    // Releases
    if (details().contains(MovieScraperInfos::Certification)) {
        ++m_maxDownloads;
        m_loadsLeft.insert(ScraperData::Releases);
        QUrl url = m_api.movieUrl(identifier(), TmdbApi::ApiMovieDetails::RELEASES);
        QNetworkReply* const reply = m_api.sendGetRequest(url);
        connect(reply, &QNetworkReply::finished, this, &TmdbMovieScrapeJob::loadReleasesFinished);
    }
}

void TmdbMovieScrapeJob::loadCollection(const TmdbId& collectionTmdbId)
{
    if (!collectionTmdbId.isValid()) {
        removeFromloadsLeft(ScraperData::Infos);
        return;
    }

    QNetworkReply* const reply = m_api.sendGetRequest(m_api.collectionUrl(collectionTmdbId.toString()));
    connect(reply, &QNetworkReply::finished, this, &TmdbMovieScrapeJob::loadCollectionFinished);
}

void TmdbMovieScrapeJob::removeFromloadsLeft(ScraperData data)
{
    QMutexLocker locker(&m_loadMutex);
    m_loadsLeft.remove(data);

    emit sigProgress(m_loadsLeft.size(), m_maxDownloads);

    if (m_loadsLeft.isEmpty() && !m_aborted) {
        emit sigScrapeSuccess();
    }
}

void TmdbMovieScrapeJob::abort(QNetworkReply* networkReply)
{
    ScraperLoadError error;
    error.error = ScraperLoadError::ErrorType::NetworkError;
    error.message = (networkReply != nullptr) ? networkReply->errorString() : "";

    QMutexLocker locker(&m_loadMutex);
    if (!m_aborted) {
        m_aborted = true;
        locker.unlock();
        emit sigScrapeError(error);
    }
}

void TmdbMovieScrapeJob::abort(QJsonParseError parseError)
{
    ScraperLoadError error;
    error.error = ScraperLoadError::ErrorType::NetworkError;
    error.message = tr("Error parsing response from TMDb (message: %1)").arg(parseError.errorString());

    QMutexLocker locker(&m_loadMutex);
    if (!m_aborted) {
        m_aborted = true;
        locker.unlock();
        emit sigScrapeError(error);
    }
}

void TmdbMovieScrapeJob::loadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qFatal("[TmdbMovieScrapeJob] Dynamic cast failed!");
        return;
    }
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[TmdbMovieScraperJob] Network Error (load)" << reply->errorString();
        abort(reply);
        return;
    }

    const QString msg = QString::fromUtf8(reply->readAll());
    parseAndAssignInfos(msg);

    // If the movie is part of a collection then download the collection data
    // and delay the call to removeFromm_loadsLeft(ScraperData::Infos)
    // to loadCollectionFinished()
    if (details().contains(MovieScraperInfos::Set)) {
        loadCollection(movie().set().tmdbId);
        return;
    }

    removeFromloadsLeft(ScraperData::Infos);
}

void TmdbMovieScrapeJob::loadCollectionFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qFatal("[TmdbMovieScrapeJob] Dynamic cast failed!");
        return;
    }
    reply->deleteLater();

    const QString msg = QString::fromUtf8(reply->readAll());
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(msg.toUtf8(), &parseError).object();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[TmdbMovieScrapeJob] Error parsing info json " << parseError.errorString();
        abort(parseError);
        return;
    }

    if (parsedJson.keys().contains("success") && !parsedJson.value("success").toBool()) {
        qWarning() << "[TmdbMovieScrapeJob] Error message from TmdbMovie:" << parsedJson.value("status_message");
        abort(parseError);
        return;
    }

    MovieSet set;
    set.tmdbId = TmdbId(parsedJson.value("id").toInt());
    set.name = parsedJson.value("name").toString();
    set.overview = parsedJson.value("overview").toString();
    movie().setSet(set);

    removeFromloadsLeft(ScraperData::Infos);
}

void TmdbMovieScrapeJob::loadCastsFinished()
{
    loadPartFinished(ScraperData::Casts);
}

void TmdbMovieScrapeJob::loadTrailersFinished()
{
    loadPartFinished(ScraperData::Trailers);
}

void TmdbMovieScrapeJob::loadImagesFinished()
{
    loadPartFinished(ScraperData::Images);
}

void TmdbMovieScrapeJob::loadReleasesFinished()
{
    loadPartFinished(ScraperData::Releases);
}

void TmdbMovieScrapeJob::loadPartFinished(ScraperData data)
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qFatal("[TmdbMovieScrapeJob] Dynamic cast failed!");
        return;
    }
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[TmdbMovieScraperJob] Network Error" << static_cast<int>(data) << reply->errorString();
        abort(reply);
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    parseAndAssignInfos(msg);

    removeFromloadsLeft(data);
}

void TmdbMovieScrapeJob::parseAndAssignInfos(QString json)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[TmdbMovieScrapeJob] Error parsing info json " << parseError.errorString();
        abort(parseError);
        return;
    }

    if (parsedJson.keys().contains("success") && !parsedJson.value("success").toBool()) {
        qWarning() << "[TmdbMovieScrapeJob] Error message from TmdbMovie:" << parsedJson.value("status_message");
        abort(parseError);
        return;
    }

    // Infos
    int tmdbId = parsedJson.value("id").toInt(-1);
    if (tmdbId > -1) {
        movie().setTmdbId(TmdbId(tmdbId));
    }

    if (!parsedJson.value("imdb_id").toString().isEmpty()) {
        movie().setId(ImdbId(parsedJson.value("imdb_id").toString()));
    }

    if (details().contains(MovieScraperInfos::Title)) {
        if (!parsedJson.value("title").toString().isEmpty()) {
            movie().setName(parsedJson.value("title").toString());
        }
        if (!parsedJson.value("original_title").toString().isEmpty()) {
            movie().setOriginalName(parsedJson.value("original_title").toString());
        }
    }

    if (details().contains(MovieScraperInfos::Set) && parsedJson.value("belongs_to_collection").isObject()) {
        const auto collection = parsedJson.value("belongs_to_collection").toObject();
        MovieSet set;
        set.tmdbId = TmdbId(collection.value("id").toInt());
        set.name = collection.value("name").toString();
        movie().setSet(set);
    }

    if (details().contains(MovieScraperInfos::Overview)) {
        QTextDocument doc;
        doc.setHtml(parsedJson.value("overview").toString());
        const auto overviewStr = doc.toPlainText();
        if (!overviewStr.isEmpty()) {
            movie().setOverview(overviewStr);
            if (usePlotForOutline()) {
                movie().setOutline(overviewStr);
            }
        }
    }

    // Either set both vote_average and vote_count or neither one.
    if (details().contains(MovieScraperInfos::Rating) && parsedJson.value("vote_average").toDouble(-1) >= 0) {
        Rating rating;
        rating.source = "themoviedb";
        rating.maxRating = 10;
        rating.rating = parsedJson.value("vote_average").toDouble();
        rating.voteCount = parsedJson.value("vote_count").toInt();
        movie().ratings().push_back(rating);
    }

    if (details().contains(MovieScraperInfos::Tagline) && !parsedJson.value("tagline").toString().isEmpty()) {
        movie().setTagline(parsedJson.value("tagline").toString());
    }

    if (details().contains(MovieScraperInfos::Released) && !parsedJson.value("release_date").toString().isEmpty()) {
        movie().setReleased(QDate::fromString(parsedJson.value("release_date").toString(), "yyyy-MM-dd"));
    }

    if (details().contains(MovieScraperInfos::Runtime) && parsedJson.value("runtime").toInt(-1) >= 0) {
        movie().setRuntime(std::chrono::minutes(parsedJson.value("runtime").toInt()));
    }

    if (details().contains(MovieScraperInfos::Genres) && parsedJson.value("genres").isArray()) {
        const auto genres = parsedJson.value("genres").toArray();
        for (const auto& it : genres) {
            const auto genre = it.toObject();
            if (genre.value("id").toInt(-1) == -1) {
                continue;
            }
            movie().addGenre(helper::mapGenre(genre.value("name").toString()));
        }
    }
    if (details().contains(MovieScraperInfos::Studios) && parsedJson.value("production_companies").isArray()) {
        const auto companies = parsedJson.value("production_companies").toArray();
        for (const auto& it : companies) {
            const auto company = it.toObject();
            if (company.value("id").toInt(-1) == -1) {
                continue;
            }
            movie().addStudio(helper::mapStudio(company.value("name").toString()));
        }
    }
    if (details().contains(MovieScraperInfos::Countries) && parsedJson.value("production_countries").isArray()) {
        const auto countries = parsedJson.value("production_countries").toArray();
        for (const auto& it : countries) {
            const auto country = it.toObject();
            if (country.value("name").toString().isEmpty()) {
                continue;
            }
            movie().addCountry(helper::mapCountry(country.value("name").toString()));
        }
    }

    // Casts
    if (details().contains(MovieScraperInfos::Actors) && parsedJson.value("cast").isArray()) {
        // clear actors
        movie().setActors({});

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
                a.thumb = m_api.imageBaseUrl() + "original" + actor.value("profile_path").toString();
            }
            movie().addActor(a);
        }
    }

    // Crew
    if ((details().contains(MovieScraperInfos::Director) || details().contains(MovieScraperInfos::Writer))
        && parsedJson.value("crew").isArray()) {
        const auto crew = parsedJson.value("crew").toArray();
        for (const auto& it : crew) {
            const auto member = it.toObject();
            if (member.value("name").toString().isEmpty()) {
                continue;
            }
            if (details().contains(MovieScraperInfos::Writer) && member.value("department").toString() == "Writing") {
                QString writer = movie().writer();
                if (writer.contains(member.value("name").toString())) {
                    continue;
                }
                if (!writer.isEmpty()) {
                    writer.append(", ");
                }
                writer.append(member.value("name").toString());
                movie().setWriter(writer);
            }
            if (details().contains(MovieScraperInfos::Director) && member.value("job").toString() == "Director"
                && member.value("department").toString() == "Directing") {
                movie().setDirector(member.value("name").toString());
            }
        }
    }

    // Trailers
    if (details().contains(MovieScraperInfos::Trailer) && parsedJson.value("youtube").isArray()) {
        // Look for "type" key in each element and look for the first instance of "Trailer" as value
        const auto videos = parsedJson.value("youtube").toArray();
        for (const auto& it : videos) {
            const auto videoObj = it.toObject();
            const QString videoType = videoObj.value("type").toString();
            if (videoType.toLower() == "trailer") {
                const QString youtubeSrc = videoObj.value("source").toString();
                movie().setTrailer(QUrl(
                    helper::formatTrailerUrl(QStringLiteral("https://www.youtube.com/watch?v=%1").arg(youtubeSrc))));
                break;
            }
        }
    }

    // Images
    if (details().contains(MovieScraperInfos::Backdrop) && parsedJson.value("backdrops").isArray()) {
        const auto backdrops = parsedJson.value("backdrops").toArray();
        for (const auto& it : backdrops) {
            const auto backdrop = it.toObject();
            const QString filePath = backdrop.value("file_path").toString();
            if (filePath.isEmpty()) {
                continue;
            }
            Poster b;
            b.thumbUrl = m_api.imageBaseUrl() + "w780" + filePath;
            b.originalUrl = m_api.imageBaseUrl() + "original" + filePath;
            b.originalSize.setWidth(backdrop.value("width").toInt());
            b.originalSize.setHeight(backdrop.value("height").toInt());
            movie().images().addBackdrop(b);
        }
    }

    if (details().contains(MovieScraperInfos::Poster) && parsedJson.value("posters").isArray()) {
        const auto posters = parsedJson.value("posters").toArray();
        for (const auto& it : posters) {
            const auto poster = it.toObject();
            const QString filePath = poster.value("file_path").toString();
            if (filePath.isEmpty()) {
                continue;
            }
            Poster b;
            b.thumbUrl = m_api.imageBaseUrl() + "w342" + filePath;
            b.originalUrl = m_api.imageBaseUrl() + "original" + filePath;
            b.originalSize.setWidth(poster.value("width").toInt());
            b.originalSize.setHeight(poster.value("height").toInt());
            b.language = poster.value("iso_639_1").toString();
            bool primaryLang = (b.language == locale().language());
            movie().images().addPoster(b, primaryLang);
        }
    }

    // Releases
    if (details().contains(MovieScraperInfos::Certification) && parsedJson.value("countries").isArray()) {
        Certification _locale;
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
            if (iso3166.toUpper() == locale().country()) {
                _locale = certification;
            }
        }

        if (locale().country() == QLocale::UnitedStates && us.isValid()) {
            movie().setCertification(helper::mapCertification(us));

        } else if (locale().language() == QLocale::English && gb.isValid()) {
            movie().setCertification(helper::mapCertification(gb));

        } else if (_locale.isValid()) {
            movie().setCertification(helper::mapCertification(_locale));

        } else if (us.isValid()) {
            movie().setCertification(helper::mapCertification(us));

        } else if (gb.isValid()) {
            movie().setCertification(helper::mapCertification(gb));
        }
    }
}

} // namespace scraper
} // namespace mediaelch
