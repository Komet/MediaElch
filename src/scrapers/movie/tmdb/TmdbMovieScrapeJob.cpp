#include "scrapers/movie/tmdb/TmdbMovieScrapeJob.h"

#include "movies/Movie.h"
#include "scrapers/tmdb/TmdbApi.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTextDocument>

namespace mediaelch {
namespace scraper {

TmdbMovieScrapeJob::TmdbMovieScrapeJob(TmdbApi& api, MovieScrapeJob::Config _config, QObject* parent) :
    MovieScrapeJob(std::move(_config), parent), m_api{api}
{
}

void TmdbMovieScrapeJob::execute()
{
    const QString& id = config().identifier.str();

    if (ImdbId::isValidFormat(id)) {
        m_movie->setImdbId(ImdbId(id));
    } else {
        m_movie->setTmdbId(TmdbId(id));
    }


    // Infos
    {
        m_loadsLeft.append(ScraperData::Infos);

        m_api.sendGetRequest(config().locale,
            m_api.getMovieUrl(id, config().locale, TmdbApi::ApiMovieDetails::INFOS),
            [this](QJsonDocument json, ScraperError error) {
                if (!error.hasError()) {
                    parseAndAssignInfos(json);

                    // if the movie is part of a collection then download the collection data
                    // and delay the call to removeFromLoadsLeft(ScraperData::Infos)
                    // to loadCollectionFinished()
                    if (config().details.contains(MovieScraperInfo::Set)) {
                        loadCollection(m_movie->set().tmdbId);
                        return;
                    }

                } else {
                    m_error = error;
                }

                onDownloadDone(ScraperData::Infos);
            });
    }

    // Casts
    {
        m_loadsLeft.append(ScraperData::Casts);
        m_api.sendGetRequest(config().locale,
            m_api.getMovieUrl(id, config().locale, TmdbApi::ApiMovieDetails::CASTS),
            [this](QJsonDocument json, ScraperError error) {
                if (!error.hasError()) {
                    parseAndAssignInfos(json);
                } else {
                    m_error = error;
                }
                onDownloadDone(ScraperData::Casts);
            });
    }

    // Trailers
    {
        m_loadsLeft.append(ScraperData::Trailers);
        m_api.sendGetRequest(config().locale,
            m_api.getMovieUrl(id, config().locale, TmdbApi::ApiMovieDetails::TRAILERS),
            [this](QJsonDocument json, ScraperError error) {
                if (!error.hasError()) {
                    parseAndAssignInfos(json);
                } else {
                    m_error = error;
                }
                onDownloadDone(ScraperData::Trailers);
            });
    }

    // Images
    {
        m_loadsLeft.append(ScraperData::Images);
        m_api.sendGetRequest(config().locale,
            m_api.getMovieUrl(id, config().locale, TmdbApi::ApiMovieDetails::IMAGES),
            [this](QJsonDocument json, ScraperError error) {
                if (!error.hasError()) {
                    parseAndAssignInfos(json);
                } else {
                    m_error = error;
                }
                onDownloadDone(ScraperData::Images);
            });
    }

    // Releases
    {
        m_loadsLeft.append(ScraperData::Releases);
        m_api.sendGetRequest(config().locale,
            m_api.getMovieUrl(id, config().locale, TmdbApi::ApiMovieDetails::RELEASES),
            [this](QJsonDocument json, ScraperError error) {
                if (!error.hasError()) {
                    parseAndAssignInfos(json);
                } else {
                    m_error = error;
                }
                onDownloadDone(ScraperData::Releases);
            });
    }
}


void TmdbMovieScrapeJob::loadCollection(const TmdbId& collectionTmdbId)
{
    if (!collectionTmdbId.isValid()) {
        onDownloadDone(ScraperData::Infos);
        return;
    }

    m_api.sendGetRequest(config().locale,
        m_api.getCollectionUrl(collectionTmdbId.toString(), config().locale),
        [this](QJsonDocument json, ScraperError error) {
            onDownloadDone(ScraperData::Infos);

            if (error.hasError()) {
                m_error = error;
                return;
            }

            QJsonObject parsedJson = json.object();
            if (parsedJson.keys().contains("success") && !parsedJson.value("success").toBool()) {
                ScraperError tmdbError;
                tmdbError.error = ScraperError::Type::ApiError;
                tmdbError.message = tr("TMDb returned an unsuccessful response for a movie collection request.");
                tmdbError.technical = parsedJson.value("status_message").toString();
                m_error = error;
                return;
            }

            MovieSet set;
            set.tmdbId = TmdbId(parsedJson.value("id").toInt());
            set.name = parsedJson.value("name").toString();
            set.overview = parsedJson.value("overview").toString();
            m_movie->setSet(set);
        });
}

void TmdbMovieScrapeJob::onDownloadDone(ScraperData data)
{
    m_loadsLeft.removeOne(data);
    if (m_loadsLeft.isEmpty()) {
        emit sigFinished(this);
    }
}


void TmdbMovieScrapeJob::parseAndAssignInfos(const QJsonDocument& json)
{
    QJsonObject parsedJson = json.object();
    // Infos
    int tmdbId = parsedJson.value("id").toInt(-1);
    if (tmdbId > -1) {
        m_movie->setTmdbId(TmdbId(tmdbId));
    }
    if (!parsedJson.value("imdb_id").toString().isEmpty()) {
        m_movie->setImdbId(ImdbId(parsedJson.value("imdb_id").toString()));
    }
    {
        if (!parsedJson.value("title").toString().isEmpty()) {
            m_movie->setName(parsedJson.value("title").toString());
        }
        if (!parsedJson.value("original_title").toString().isEmpty()) {
            m_movie->setOriginalName(parsedJson.value("original_title").toString());
        }
    }
    if (parsedJson.value("belongs_to_collection").isObject()) {
        const auto collection = parsedJson.value("belongs_to_collection").toObject();
        MovieSet set;
        set.tmdbId = TmdbId(collection.value("id").toInt());
        set.name = collection.value("name").toString();
        m_movie->setSet(set);
    }
    {
        QTextDocument doc;
        doc.setHtml(parsedJson.value("overview").toString());
        const auto overviewStr = doc.toPlainText();
        if (!overviewStr.isEmpty()) {
            m_movie->setOverview(overviewStr);
        }
    }
    // Either set both vote_average and vote_count or neither one.
    if (parsedJson.value("vote_average").toDouble(-1) >= 0) {
        Rating rating;
        rating.source = "themoviedb";
        rating.maxRating = 10;
        rating.rating = parsedJson.value("vote_average").toDouble();
        rating.voteCount = parsedJson.value("vote_count").toInt();
        m_movie->ratings().push_back(rating);
    }
    if (!parsedJson.value("tagline").toString().isEmpty()) {
        m_movie->setTagline(parsedJson.value("tagline").toString());
    }
    if (!parsedJson.value("release_date").toString().isEmpty()) {
        m_movie->setReleased(QDate::fromString(parsedJson.value("release_date").toString(), "yyyy-MM-dd"));
    }
    if (parsedJson.value("runtime").toInt(-1) >= 0) {
        m_movie->setRuntime(std::chrono::minutes(parsedJson.value("runtime").toInt()));
    }
    if (parsedJson.value("genres").isArray()) {
        const auto genres = parsedJson.value("genres").toArray();
        for (const auto& it : genres) {
            const auto genre = it.toObject();
            if (genre.value("id").toInt(-1) == -1) {
                continue;
            }
            m_movie->addGenre(helper::mapGenre(genre.value("name").toString()));
        }
    }
    if (parsedJson.value("production_companies").isArray()) {
        const auto companies = parsedJson.value("production_companies").toArray();
        for (const auto& it : companies) {
            const auto company = it.toObject();
            if (company.value("id").toInt(-1) == -1) {
                continue;
            }
            m_movie->addStudio(helper::mapStudio(company.value("name").toString()));
        }
    }
    if (parsedJson.value("production_countries").isArray()) {
        const auto countries = parsedJson.value("production_countries").toArray();
        for (const auto& it : countries) {
            const auto country = it.toObject();
            if (country.value("name").toString().isEmpty()) {
                continue;
            }
            m_movie->addCountry(helper::mapCountry(country.value("name").toString()));
        }
    }

    // Casts
    if (parsedJson.value("cast").isArray()) {
        // clear actors
        m_movie->setActors({});

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
                a.thumb = m_api.config().imageBaseUrl + "original" + actor.value("profile_path").toString();
            }
            m_movie->addActor(a);
        }
    }

    // Crew
    if (parsedJson.value("crew").isArray()) {
        const auto crew = parsedJson.value("crew").toArray();
        for (const auto& it : crew) {
            const auto member = it.toObject();
            if (member.value("name").toString().isEmpty()) {
                continue;
            }
            if (member.value("department").toString() == "Writing") {
                QString writer = m_movie->writer();
                if (writer.contains(member.value("name").toString())) {
                    continue;
                }
                if (!writer.isEmpty()) {
                    writer.append(", ");
                }
                writer.append(member.value("name").toString());
                m_movie->setWriter(writer);
            }
            if (member.value("job").toString() == "Director" && member.value("department").toString() == "Directing") {
                m_movie->setDirector(member.value("name").toString());
            }
        }
    }

    // Trailers
    if (parsedJson.value("youtube").isArray()) {
        // Look for "type" key in each element and look for the first instance of "Trailer" as value
        const auto videos = parsedJson.value("youtube").toArray();
        for (const auto& it : videos) {
            const auto videoObj = it.toObject();
            const QString videoType = videoObj.value("type").toString();
            if (videoType.toLower() == "trailer") {
                const QString youtubeSrc = videoObj.value("source").toString();
                m_movie->setTrailer(QUrl(
                    helper::formatTrailerUrl(QStringLiteral("https://www.youtube.com/watch?v=%1").arg(youtubeSrc))));
                break;
            }
        }
    }

    // Images
    if (parsedJson.value("backdrops").isArray()) {
        const auto backdrops = parsedJson.value("backdrops").toArray();
        for (const auto& it : backdrops) {
            const auto backdrop = it.toObject();
            const QString filePath = backdrop.value("file_path").toString();
            if (filePath.isEmpty()) {
                continue;
            }
            Poster b;
            b.thumbUrl = m_api.config().imageBaseUrl + "w780" + filePath;
            b.originalUrl = m_api.config().imageBaseUrl + "original" + filePath;
            b.originalSize.setWidth(backdrop.value("width").toInt());
            b.originalSize.setHeight(backdrop.value("height").toInt());
            m_movie->images().addBackdrop(b);
        }
    }

    if (parsedJson.value("posters").isArray()) {
        const auto posters = parsedJson.value("posters").toArray();
        for (const auto& it : posters) {
            const auto poster = it.toObject();
            const QString filePath = poster.value("file_path").toString();
            if (filePath.isEmpty()) {
                continue;
            }
            Poster b;
            b.thumbUrl = m_api.config().imageBaseUrl + "w342" + filePath;
            b.originalUrl = m_api.config().imageBaseUrl + "original" + filePath;
            b.originalSize.setWidth(poster.value("width").toInt());
            b.originalSize.setHeight(poster.value("height").toInt());
            b.language = poster.value("iso_639_1").toString();
            bool primaryLang = (b.language == config().locale.language());
            m_movie->images().addPoster(b, primaryLang);
        }
    }

    // Releases
    if (parsedJson.value("countries").isArray()) {
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
            if (iso3166.toUpper() == config().locale.country()) {
                locale = certification;
            }
        }

        if (config().locale.country() == "US" && us.isValid()) {
            m_movie->setCertification(helper::mapCertification(us));

        } else if (config().locale.language() == "en" && gb.isValid()) {
            m_movie->setCertification(helper::mapCertification(gb));

        } else if (locale.isValid()) {
            m_movie->setCertification(helper::mapCertification(locale));

        } else if (us.isValid()) {
            m_movie->setCertification(helper::mapCertification(us));

        } else if (gb.isValid()) {
            m_movie->setCertification(helper::mapCertification(gb));
        }
    }
}
} // namespace scraper
} // namespace mediaelch
