#include "scrapers/movie/tmdb/TmdbMovieScrapeJob.h"

#include "data/movie/Movie.h"
#include "log/Log.h"
#include "scrapers/tmdb/TmdbApi.h"
#include "settings/Settings.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTextDocument>

namespace mediaelch {
namespace scraper {

TmdbMovieScrapeJob::TmdbMovieScrapeJob(TmdbApi& api, MovieScrapeJob::Config _config, QObject* parent) :
    MovieScrapeJob(std::move(_config), parent), m_api{api}, m_baseUrl{"http://image.tmdb.org/t/p/"}
{
}

void TmdbMovieScrapeJob::doStart()
{
    // TODO
}


void TmdbMovieScrapeJob::loadCollection(const TmdbId& collectionTmdbId)
{
    // TODO
    Q_UNUSED(collectionTmdbId)
}

void TmdbMovieScrapeJob::onDownloadDone(ScraperData data)
{
    // TODO
    Q_UNUSED(data)
}


void TmdbMovieScrapeJob::parseAndAssignInfos(const QString& json,
    Movie* movie,
    const QSet<MovieScraperInfo>& infos,
    const QString& language,
    const QString& country)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(generic) << "Error parsing info json " << parseError.errorString();
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
        movie->ratings().setOrAddRating(rating);
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
            bool primaryLang = (b.language == language);
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
            if (iso3166.toUpper() == country) {
                locale = certification;
            }
        }

        if (country == "US" && us.isValid()) {
            movie->setCertification(helper::mapCertification(us));

        } else if (language == "en" && gb.isValid()) {
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
