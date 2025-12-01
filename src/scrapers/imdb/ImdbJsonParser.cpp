#include "ImdbJsonParser.h"

#include "data/Actor.h"
#include "data/ImdbId.h"
#include "data/Poster.h"
#include "globals/Helper.h"
#include "scrapers/ScraperUtils.h"

#include <QJsonArray>
#include <QJsonObject>
#include <qmath.h>

namespace {

// clang-format off
const QVector<QString> IMDB_JSON_PATH_ID               = { "props", "pageProps", "mainColumnData", "id" };
const QVector<QString> IMDB_JSON_PATH_TITLE            = { "props", "pageProps", "mainColumnData", "titleText", "text" };
const QVector<QString> IMDB_JSON_PATH_ORIGINAL_TITLE   = { "props", "pageProps", "mainColumnData", "originalTitleText", "text" };
const QVector<QString> IMDB_JSON_PATH_OVERVIEW         = { "props", "pageProps", "mainColumnData", "summaries", "edges", "0", "node", "plotText", "plaidHtml" };
const QVector<QString> IMDB_JSON_PATH_OUTLINE          = { "props", "pageProps", "mainColumnData", "plot", "plotText", "plainText" };
const QVector<QString> IMDB_JSON_PATH_RELEASE_DATE     = { "props", "pageProps", "mainColumnData", "releaseDate" };
const QVector<QString> IMDB_JSON_PATH_RUNTIME_SECONDS  = { "props", "pageProps", "aboveTheFoldData", "runtime", "seconds" };
const QVector<QString> IMDB_JSON_PATH_TOP250           = { "props", "pageProps", "mainColumnData", "ratingsSummary", "topRanking", "rank" };
const QVector<QString> IMDB_JSON_PATH_RATING           = { "props", "pageProps", "mainColumnData", "ratingsSummary", "aggregateRating" };
const QVector<QString> IMDB_JSON_PATH_VOTE_COUNT       = { "props", "pageProps", "mainColumnData", "ratingsSummary", "voteCount" };
const QVector<QString> IMDB_JSON_PATH_GENRES           = { "props", "pageProps", "mainColumnData", "genres", "genres" };
const QVector<QString> IMDB_JSON_PATH_TAGLINE          = { "props", "pageProps", "mainColumnData", "taglines", "edges", "0", "node", "text" };
const QVector<QString> IMDB_JSON_PATH_KEYWORDS         = { "props", "pageProps", "mainColumnData", "storylineKeywords", "edges" };
const QVector<QString> IMDB_JSON_PATH_CERTIFICATIONS   = { "props", "pageProps", "mainColumnData", "certificates", "edges" };
const QVector<QString> IMDB_JSON_PATH_STUDIOS          = { "props", "pageProps", "mainColumnData", "production", "edges" };
const QVector<QString> IMDB_JSON_PATH_STUDIO_NAME      = { "node", "company", "companyText", "text" };
const QVector<QString> IMDB_JSON_PATH_COUNTRIES        = { "props", "pageProps", "mainColumnData", "countriesOfOrigin", "countries" };
const QVector<QString> IMDB_JSON_PATH_POSTER_URL       = { "props", "pageProps", "aboveTheFoldData", "primaryImage", "url" };
// TODO: Select highest definition
const QVector<QString> IMDB_JSON_PATH_TRAILER_URL      = { "props", "pageProps", "mainColumnData", "primaryVideos", "edges", "0", "node", "playbackURLs", "0", "url" };

// Cast / Actors / Directors
// TODO: Scrape more actors from reference page
const QVector<QString> IMDB_JSON_PATH_CREDIT_GROUPING = { "props", "pageProps", "mainColumnData", "creditGroupings", "edges" };
const QVector<QString> IMDB_JSON_PATH_CAST_NAME       = { "node", "name", "nameText", "text" };
const QVector<QString> IMDB_JSON_PATH_CAST_URL        = { "node", "name", "primaryImage", "url" };
const QVector<QString> IMDB_JSON_PATH_CAST_ROLE       = { "node", "creditedRoles", "edges",  "0", "node", "text" };

// TV Shows
const QVector<QString> IMDB_JSON_PATH_SEASONS         = { "props", "pageProps", "contentData", "entityMetadata",/*??*/ "data", "title", "episodes", "seasons" };
const QVector<QString> IMDB_JSON_PATH_SEASON_EPISODES = { "props", "pageProps", "contentData", "section", "episodes", "items" };
// clang-format on

} // namespace

namespace mediaelch {
namespace scraper {

ImdbData ImdbJsonParser::parseFromReferencePage(const QString& html, const Locale& preferredLocale)
{
    // Note: Expects HTML from https://www.imdb.com/title/tt________/reference
    QJsonDocument json = extractJsonFromHtml(html);

    ImdbJsonParser parser{};
    parser.parserAndAssignDetails(json, preferredLocale);
    parser.parseAndAssignDirectors(json);
    parser.parseAndAssignWriters(json);
    parser.parseAndStoreActors(json);

    return parser.m_data;
}

QVector<int> ImdbJsonParser::parseSeasonNumbersFromEpisodesPage(const QString& html)
{
    QVector<int> seasons;
    QJsonObject json = extractJsonFromHtml(html).object();
    QJsonArray seasonsArray = followJsonPath(json, IMDB_JSON_PATH_SEASONS).toArray();
    for (const auto& season : seasonsArray) {
        const int number = season.toObject().value("number").toInt(-1);
        if (number > -1) {
            seasons.append(number);
        }
    }
    return seasons;
}

QVector<ImdbShortEpisodeData> ImdbJsonParser::parseEpisodeIds(const QString& html)
{
    QVector<ImdbShortEpisodeData> episodes;
    QJsonObject json = extractJsonFromHtml(html).object();
    QJsonArray episodesArray = followJsonPath(json, IMDB_JSON_PATH_SEASON_EPISODES).toArray();
    for (const auto& episodeValue : episodesArray) {
        QJsonObject episodeObject = episodeValue.toObject();
        ImdbShortEpisodeData data;
        {
            bool ok{false};
            data.imdbId = episodeObject.value("id").toString();
            data.seasonNumber = episodeObject.value("season").toString().toInt(&ok);
            if (!ok) {
                continue;
            }
        }
        {
            bool ok{false};
            data.episodeNumber = episodeObject.value("episode").toString().toInt(&ok);
            if (!ok) {
                continue;
            }
        }
        episodes.append(data);
    }
    return episodes;
}

void ImdbJsonParser::parserAndAssignDetails(const QJsonDocument& json, const Locale& preferredLocale)
{
    using namespace std::chrono;

    QJsonValue value;

    value = followJsonPath(json, IMDB_JSON_PATH_ID);
    if (value.isString()) {
        QString id = value.toString();
        m_data.imdbId = ImdbId(id);
    }

    value = followJsonPath(json, IMDB_JSON_PATH_TITLE);
    if (value.isString()) {
        m_data.title = value.toString().trimmed();
    }

    value = followJsonPath(json, IMDB_JSON_PATH_ORIGINAL_TITLE);
    if (value.isString()) {
        m_data.originalTitle = value.toString().trimmed();
    }

    value = followJsonPath(json, IMDB_JSON_PATH_OVERVIEW);
    if (value.isString()) {
        m_data.overview = removeHtmlEntities(value.toString().trimmed());
    }

    value = followJsonPath(json, IMDB_JSON_PATH_OUTLINE);
    if (value.isString()) {
        m_data.outline = removeHtmlEntities(value.toString().trimmed());
    }

    value = followJsonPath(json, IMDB_JSON_PATH_GENRES);
    if (value.isArray()) {
        for (const auto& genreObj : value.toArray()) {
            QString genre = genreObj.toObject().value("text").toString().trimmed();
            if (!genre.isEmpty()) {
                m_data.genres.insert(genre);
            }
        }
    }

    value = followJsonPath(json, IMDB_JSON_PATH_STUDIOS);
    if (value.isArray()) {
        for (const auto& studioObj : value.toArray()) {
            QString studio = followJsonPath(studioObj.toObject(), IMDB_JSON_PATH_STUDIO_NAME).toString().trimmed();
            if (!studio.isEmpty()) {
                m_data.studios.insert(helper::mapStudio(studio));
            }
        }
    }

    value = followJsonPath(json, IMDB_JSON_PATH_COUNTRIES);
    if (value.isArray()) {
        for (const auto& countryObj : value.toArray()) {
            QString country = countryObj.toObject().value("id").toString().trimmed();
            if (!country.isEmpty()) {
                m_data.countries.insert(helper::mapCountry(country));
            }
        }
    }

    value = followJsonPath(json, IMDB_JSON_PATH_TAGLINE);
    if (value.isString()) {
        m_data.tagline = removeHtmlEntities(value.toString().trimmed());
    }

    value = followJsonPath(json, IMDB_JSON_PATH_RUNTIME_SECONDS);
    if (value.isDouble()) {
        const int runtime = value.toInt(-1);
        if (runtime > 0) {
            m_data.runtime = minutes(qCeil(runtime / 60.));
        }
    }

    value = followJsonPath(json, IMDB_JSON_PATH_RELEASE_DATE);
    if (value.isObject()) {
        QJsonObject releaseDateObj = value.toObject();
        int day = releaseDateObj.value("day").toInt(-1);
        int month = releaseDateObj.value("month").toInt(-1);
        int year = releaseDateObj.value("year").toInt(-1);
        if (day > -1 && month > -1 && year > -1) {
            QDate date(year, month, day);
            if (date.isValid()) {
                m_data.released = date;
            }
        }
    }

    value = followJsonPath(json, IMDB_JSON_PATH_RATING);
    if (value.isDouble()) {
        const double avgRating = value.toDouble();
        const int voteCount = followJsonPath(json, IMDB_JSON_PATH_VOTE_COUNT).toInt(-1);
        if (avgRating > 0 || voteCount > 0) {
            Rating rating;
            rating.rating = avgRating;
            rating.voteCount = voteCount;
            rating.source = "imdb";
            rating.maxRating = 10;
            m_data.ratings.append(rating);
        }
    }

    value = followJsonPath(json, IMDB_JSON_PATH_TOP250);
    if (value.isDouble()) {
        const double top250 = value.toInt(-1);
        if (top250 > 0 && top250 <= 250) {
            m_data.top250 = top250;
        }
    }

    value = followJsonPath(json, IMDB_JSON_PATH_KEYWORDS);
    if (value.isArray()) {
        for (const auto& keywordObj : value.toArray()) {
            QString keyword = keywordObj.toObject().value("node").toObject().value("text").toString().trimmed();
            if (!keyword.isEmpty()) {
                m_data.keywords.insert(keyword);
            }
        }
    }

    value = followJsonPath(json, IMDB_JSON_PATH_CERTIFICATIONS);
    if (value.isArray()) {
        // TODO: Since IMDB only supports one locale at the moment, this has no real effect, yet!
        Certification locale;
        Certification us;

        for (const auto& certObj : value.toArray()) {
            QJsonObject node = certObj.toObject().value("node").toObject();
            QString certificationCountry = node.value("country").toObject().value("id").toString().trimmed();
            QString certificationCode = node.value("rating").toString().trimmed();

            const Certification certification = Certification(certificationCode);
            if (certificationCountry == "US") {
                us = certification;
            }
            if (certificationCountry == preferredLocale.country()) {
                locale = certification;
            }
        }

        if (locale.isValid()) {
            m_data.certification = helper::mapCertification(locale);
        } else if (us.isValid()) {
            m_data.certification = helper::mapCertification(us);
        }
    }

    value = followJsonPath(json, IMDB_JSON_PATH_POSTER_URL);
    if (value.isString()) {
        const QUrl url(sanitizeAmazonMediaUrl(value.toString()));
        if (url.isValid()) {
            Poster p;
            p.thumbUrl = url;
            p.originalUrl = url;
            m_data.poster = p;
        }
    }

    value = followJsonPath(json, IMDB_JSON_PATH_TRAILER_URL);
    if (value.isString()) {
        const QUrl url(value.toString());
        if (url.isValid()) {
            m_data.trailer = url;
        }
    }
}


QJsonDocument ImdbJsonParser::extractJsonFromHtml(const QString& html)
{
    QRegularExpression rx(R"re(<script id="__NEXT_DATA__" type="application/json">(.*)</script>)re",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match = rx.match(html);
    if (match.hasMatch()) {
        return QJsonDocument::fromJson(match.captured(1).toUtf8());
    }
    return QJsonDocument{};
}

QJsonValue ImdbJsonParser::followJsonPath(const QJsonDocument& json, const QVector<QString>& paths)
{
    return followJsonPath(json.object(), paths);
}

QJsonValue ImdbJsonParser::followJsonPath(const QJsonObject& json, const QVector<QString>& paths)
{
    QJsonValue next = json;
    QJsonObject obj;

    for (const QString& path : paths) {
        if (path == "0") { // special case for first entry of arrays
            if (!next.isArray()) {
                return QJsonValue::Null;
            }
            QJsonArray array = next.toArray();
            if (array.isEmpty()) {
                return QJsonValue::Null;
            }
            next = array.at(0);

        } else {
            if (!next.isObject()) {
                return QJsonValue::Null;
            }
            obj = next.toObject();
            if (!obj.contains(path)) {
                return QJsonValue::Null;
            }
            next = obj.value(path);
        }
    }
    return next;
}

void ImdbJsonParser::parseAndAssignDirectors(const QJsonDocument& json)
{
    QJsonValue groupings = followJsonPath(json, IMDB_JSON_PATH_CREDIT_GROUPING);
    if (!groupings.isArray()) {
        return;
    }

    for (QJsonValue grouping : groupings.toArray()) {
        QString groupingType =
            grouping.toObject().value("node").toObject().value("grouping").toObject().value("text").toString();

        if (groupingType != "Director" && groupingType != "Directors") {
            // It seems the type depends on number of entries.
            continue;
        }

        QJsonArray directorsJson =
            grouping.toObject().value("node").toObject().value("credits").toObject().value("edges").toArray();
        for (const auto& directorEntry : directorsJson) {
            // TODO: We could/should also store images, etc. of directors and writers
            const QJsonObject directorObj = directorEntry.toObject();
            const QString name = followJsonPath(directorObj, IMDB_JSON_PATH_CAST_NAME).toString().trimmed();
            if (!name.isEmpty()) {
                m_data.directors.insert(name);
            }
        }
    }
}

void ImdbJsonParser::parseAndAssignWriters(const QJsonDocument& json)
{
    QJsonValue groupings = followJsonPath(json, IMDB_JSON_PATH_CREDIT_GROUPING);
    if (!groupings.isArray()) {
        return;
    }


    for (QJsonValue grouping : groupings.toArray()) {
        QString groupingType =
            grouping.toObject().value("node").toObject().value("grouping").toObject().value("text").toString();

        if (groupingType != "Writer" && groupingType != "Writers") {
            // It seems the type depends on number of entries.
            continue;
        }

        QJsonArray writersJson =
            grouping.toObject().value("node").toObject().value("credits").toObject().value("edges").toArray();
        for (const auto& writerEntry : writersJson) {
            // TODO: We could/should also store images, etc. of directors and writers
            const QJsonObject writerObj = writerEntry.toObject();
            const QString name = followJsonPath(writerObj, IMDB_JSON_PATH_CAST_NAME).toString().trimmed();
            if (!name.isEmpty()) {
                m_data.writers.insert(name);
            }
        }
    }
}

void ImdbJsonParser::parseAndStoreActors(const QJsonDocument& json)
{
    QJsonValue groupings = followJsonPath(json, IMDB_JSON_PATH_CREDIT_GROUPING);
    if (!groupings.isArray()) {
        return;
    }

    for (QJsonValue grouping : groupings.toArray()) {
        QString groupingType =
            grouping.toObject().value("node").toObject().value("grouping").toObject().value("text").toString();

        if (groupingType != "Cast") {
            continue;
        }

        QJsonArray actorsJson =
            grouping.toObject().value("node").toObject().value("credits").toObject().value("edges").toArray();

        for (const auto& actorEntry : actorsJson) {
            const QJsonObject actorObj = actorEntry.toObject();
            const QString name = followJsonPath(actorObj, IMDB_JSON_PATH_CAST_NAME).toString().trimmed();
            const QString url = followJsonPath(actorObj, IMDB_JSON_PATH_CAST_URL).toString().trimmed();
            const QString role = followJsonPath(actorObj, IMDB_JSON_PATH_CAST_ROLE).toString().trimmed();
            if (!name.isEmpty()) {
                Actor actor;
                actor.name = name;
                actor.role = role;
                actor.thumb = sanitizeAmazonMediaUrl(url);
                m_data.actors.append(actor);
            }
        }
    }
}

QString ImdbJsonParser::sanitizeAmazonMediaUrl(QString url)
{
    // The URL can look like this:
    //   https://m.media-amazon.com/images/M/<image ID>._V1_UY1400_CR90,0,630,1200_AL_.jpg
    // To get the original image, everything after `._V` can be removed.

    if (!url.endsWith(".jpg")) {
        return url;
    }
    QRegularExpression rx(R"re(._V([^/]+).jpg$)re", QRegularExpression::InvertedGreedinessOption);
    url.replace(rx, ".jpg");

    return url;
}

} // namespace scraper
} // namespace mediaelch
