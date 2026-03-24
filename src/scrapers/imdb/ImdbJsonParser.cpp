#include "ImdbJsonParser.h"

#include "data/Actor.h"
#include "data/ImdbId.h"
#include "data/Poster.h"
#include "globals/Helper.h"
#include "scrapers/ScraperUtils.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
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
const QVector<QString> IMDB_JSON_PATH_METACRITIC       = { "props", "pageProps", "mainColumnData", "metacritic", "metascore", "score" };
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

// Plot-Summary page
const QVector<QString> IMDB_JSON_PATH_PLOTSUMMARY_SYNOPSIS  = { "props", "pageProps", "contentData", "data", "title", "plotSynopsis", "edges", "0", "node", "plotText", "plaidHtml" };

// clang-format on

} // namespace

namespace mediaelch {
namespace scraper {

// =============================================================================
// GraphQL-based parsing (new)
// =============================================================================

ImdbData ImdbJsonParser::parseFromGraphQL(const QString& json, const Locale& locale)
{
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return {};
    }

    const QJsonObject title = doc.object().value("data").toObject().value("title").toObject();
    if (title.isEmpty()) {
        return {};
    }

    ImdbJsonParser parser;
    parser.parseGraphQLTitle(title, locale);
    parser.parseGraphQLCredits(title);
    parser.parseGraphQLActors(title);
    return parser.m_data;
}

void ImdbJsonParser::parseGraphQLTitle(const QJsonObject& title, const Locale& locale)
{
    using namespace std::chrono;

    // IMDB ID
    const QString id = title.value("id").toString();
    if (!id.isEmpty()) {
        m_data.imdbId = ImdbId(id);
    }

    // Title + Original Title
    m_data.title = title.value("titleText").toObject().value("text").toString().trimmed();
    const QString origTitle = title.value("originalTitleText").toObject().value("text").toString().trimmed();
    if (!origTitle.isEmpty()) {
        m_data.originalTitle = origTitle;
    }

    // Localized title from AKAs
    if (locale.language() != "en") {
        const QString country = locale.country().toUpper();
        const QJsonArray akas = title.value("akas").toObject().value("edges").toArray();
        for (const auto& akaEntry : akas) {
            const QJsonObject node = akaEntry.toObject().value("node").toObject();
            const QString akaCountry = node.value("country").toObject().value("id").toString();
            if (akaCountry == country) {
                const QString localizedTitle = node.value("text").toString().trimmed();
                if (!localizedTitle.isEmpty()) {
                    m_data.localizedTitle = localizedTitle;
                    break;
                }
            }
        }
    }

    // Plot / Overview — use the longest plot text
    const QJsonArray plots = title.value("plots").toObject().value("edges").toArray();
    QString longestPlot;
    QString shortestPlot;
    for (const auto& plotEntry : plots) {
        const QString plotText =
            plotEntry.toObject().value("node").toObject().value("plotText").toObject().value("plainText").toString();
        if (!plotText.isEmpty()) {
            if (plotText.length() > longestPlot.length()) {
                longestPlot = plotText;
            }
            if (shortestPlot.isEmpty() || plotText.length() < shortestPlot.length()) {
                shortestPlot = plotText;
            }
        }
    }
    // Fallback to the single "plot" field if plots array is empty
    if (longestPlot.isEmpty()) {
        longestPlot = title.value("plot").toObject().value("plotText").toObject().value("plainText").toString();
    }
    if (!longestPlot.isEmpty()) {
        m_data.overview = longestPlot.trimmed();
    }
    if (!shortestPlot.isEmpty() && shortestPlot != longestPlot) {
        m_data.outline = shortestPlot.trimmed();
    } else if (!longestPlot.isEmpty()) {
        // Use first sentence as outline if no separate short plot
        const qsizetype dotPos = longestPlot.indexOf(". ");
        if (dotPos > 0 && dotPos < longestPlot.length() - 2) {
            m_data.outline = longestPlot.left(dotPos + 1).trimmed();
        }
    }

    // Genres
    const QJsonArray genres = title.value("genres").toObject().value("genres").toArray();
    for (const auto& genreObj : genres) {
        const QString genre = genreObj.toObject().value("text").toString().trimmed();
        if (!genre.isEmpty()) {
            m_data.genres.insert(genre);
        }
    }

    // Studios (production companies)
    const QJsonArray companies = title.value("companyCredits").toObject().value("edges").toArray();
    for (const auto& companyEntry : companies) {
        const QString studio = companyEntry.toObject()
                                   .value("node")
                                   .toObject()
                                   .value("company")
                                   .toObject()
                                   .value("companyText")
                                   .toObject()
                                   .value("text")
                                   .toString()
                                   .trimmed();
        if (!studio.isEmpty()) {
            m_data.studios.insert(helper::mapStudio(studio));
        }
    }

    // Countries
    const QJsonArray countries = title.value("countriesOfOrigin").toObject().value("countries").toArray();
    for (const auto& countryObj : countries) {
        const QString country = countryObj.toObject().value("id").toString().trimmed();
        if (!country.isEmpty()) {
            m_data.countries.insert(helper::mapCountry(country));
        }
    }

    // Tagline
    const QJsonArray taglines = title.value("taglines").toObject().value("edges").toArray();
    if (!taglines.isEmpty()) {
        const QString tagline =
            taglines.at(0).toObject().value("node").toObject().value("text").toString().trimmed();
        if (!tagline.isEmpty()) {
            m_data.tagline = tagline;
        }
    }

    // Runtime
    const int runtimeSeconds = title.value("runtime").toObject().value("seconds").toInt(-1);
    if (runtimeSeconds > 0) {
        m_data.runtime = minutes(qCeil(runtimeSeconds / 60.));
    }

    // Release date
    const QJsonObject releaseDateObj = title.value("releaseDate").toObject();
    const int year = releaseDateObj.value("year").toInt(-1);
    if (year > 0) {
        const int month = releaseDateObj.value("month").toInt(1);
        const int day = releaseDateObj.value("day").toInt(1);
        QDate date(year, month, day);
        if (date.isValid()) {
            m_data.released = date;
        }
    }

    // Localized release date (override if available)
    if (locale.language() != "en") {
        const QString country = locale.country().toUpper();
        const QJsonArray releaseDates = title.value("releaseDates").toObject().value("edges").toArray();
        for (const auto& rdEntry : releaseDates) {
            const QJsonObject node = rdEntry.toObject().value("node").toObject();
            if (node.value("country").toObject().value("id").toString() == country) {
                const int rdYear = node.value("year").toInt(-1);
                const int rdMonth = node.value("month").toInt(1);
                const int rdDay = node.value("day").toInt(1);
                if (rdYear > 0) {
                    QDate localDate(rdYear, rdMonth, rdDay);
                    if (localDate.isValid()) {
                        m_data.released = localDate;
                        break;
                    }
                }
            }
        }
    }

    // Rating (IMDB)
    const QJsonObject ratingsSummary = title.value("ratingsSummary").toObject();
    const double avgRating = ratingsSummary.value("aggregateRating").toDouble(0.0);
    const int voteCount = ratingsSummary.value("voteCount").toInt(0);
    if (avgRating > 0 || voteCount > 0) {
        Rating rating;
        rating.rating = avgRating;
        rating.voteCount = voteCount;
        rating.source = "imdb";
        rating.maxRating = 10;
        m_data.ratings.append(rating);
    }

    // Metacritic
    const int metascore =
        title.value("metacritic").toObject().value("metascore").toObject().value("score").toInt(-1);
    if (metascore > 0) {
        Rating rating;
        rating.rating = metascore;
        rating.voteCount = 0;
        rating.source = "metacritic";
        rating.maxRating = 100;
        m_data.ratings.append(rating);
    }

    // Top250 (via meterRanking — this is STARmeter, not Top250; kept for compatibility)
    // Note: The actual Top250 is not directly available via GraphQL.

    // Keywords
    const QJsonArray keywords = title.value("keywords").toObject().value("edges").toArray();
    for (const auto& kwEntry : keywords) {
        const QString keyword = kwEntry.toObject().value("node").toObject().value("text").toString().trimmed();
        if (!keyword.isEmpty()) {
            m_data.keywords.insert(keyword);
        }
    }

    // Certification — locale-specific, fallback to US
    const QJsonArray certificates = title.value("certificates").toObject().value("edges").toArray();
    Certification localeCert;
    Certification usCert;
    const QString localeCountry = locale.country().toUpper();
    for (const auto& certEntry : certificates) {
        const QJsonObject node = certEntry.toObject().value("node").toObject();
        const QString certCountry = node.value("country").toObject().value("id").toString();
        const QString certRating = node.value("rating").toString().trimmed();
        const Certification cert = Certification(certRating);
        if (certCountry == "US") {
            usCert = cert;
        }
        if (certCountry == localeCountry) {
            localeCert = cert;
        }
    }
    if (localeCert.isValid()) {
        m_data.localizedCertification = helper::mapCertification(localeCert);
        m_data.certification = m_data.localizedCertification;
    } else if (usCert.isValid()) {
        m_data.certification = helper::mapCertification(usCert);
    }

    // Also check the simple "certificate" field as fallback
    if (!m_data.certification.hasValue()) {
        const QString simpleCert = title.value("certificate").toObject().value("rating").toString().trimmed();
        if (!simpleCert.isEmpty()) {
            m_data.certification = helper::mapCertification(Certification(simpleCert));
        }
    }

    // Poster
    const QString posterUrl = title.value("primaryImage").toObject().value("url").toString();
    if (!posterUrl.isEmpty()) {
        const QUrl url(sanitizeAmazonMediaUrl(posterUrl));
        if (url.isValid()) {
            Poster p;
            p.thumbUrl = url;
            p.originalUrl = url;
            m_data.poster = p;
        }
    }

    // Trailer — store IMDB video page URL (works in browser, not in Kodi)
    const QJsonArray videos = title.value("primaryVideos").toObject().value("edges").toArray();
    if (!videos.isEmpty()) {
        const QString videoId = videos.at(0).toObject().value("node").toObject().value("id").toString();
        if (!videoId.isEmpty()) {
            m_data.trailer = QUrl(QStringLiteral("https://www.imdb.com/video/%1/").arg(videoId));
        }
    }

    // TV show specific: ongoing status
    const QJsonObject episodes = title.value("episodes").toObject();
    if (!episodes.isEmpty()) {
        m_data.isOngoing = episodes.value("isOngoing").toBool(false);
    }
}

void ImdbJsonParser::parseGraphQLCredits(const QJsonObject& title)
{
    // Directors
    const QJsonArray directors = title.value("directors").toObject().value("edges").toArray();
    for (const auto& dirEntry : directors) {
        const QString name =
            dirEntry.toObject().value("node").toObject().value("name").toObject().value("nameText").toObject().value(
                "text").toString().trimmed();
        if (!name.isEmpty()) {
            m_data.directors.insert(name);
        }
    }

    // Writers
    const QJsonArray writers = title.value("writers").toObject().value("edges").toArray();
    for (const auto& writerEntry : writers) {
        const QString name = writerEntry.toObject()
                                 .value("node")
                                 .toObject()
                                 .value("name")
                                 .toObject()
                                 .value("nameText")
                                 .toObject()
                                 .value("text")
                                 .toString()
                                 .trimmed();
        if (!name.isEmpty()) {
            m_data.writers.insert(name);
        }
    }
}

void ImdbJsonParser::parseGraphQLActors(const QJsonObject& title)
{
    const QJsonArray cast = title.value("cast").toObject().value("edges").toArray();
    for (const auto& castEntry : cast) {
        const QJsonObject node = castEntry.toObject().value("node").toObject();
        const QJsonObject nameObj = node.value("name").toObject();
        const QString name = nameObj.value("nameText").toObject().value("text").toString().trimmed();
        if (name.isEmpty()) {
            continue;
        }

        Actor actor;
        actor.name = name;
        actor.id = nameObj.value("id").toString();
        actor.thumb = sanitizeAmazonMediaUrl(nameObj.value("primaryImage").toObject().value("url").toString());

        // Character name(s)
        const QJsonArray characters = node.value("characters").toArray();
        if (!characters.isEmpty()) {
            actor.role = characters.at(0).toObject().value("name").toString().trimmed();
        }

        m_data.actors.append(actor);
    }
}

QVector<ImdbEpisodeData> ImdbJsonParser::parseEpisodesFromGraphQL(const QString& json)
{
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return {};
    }

    const QJsonArray episodes = doc.object()
                                    .value("data")
                                    .toObject()
                                    .value("title")
                                    .toObject()
                                    .value("episodes")
                                    .toObject()
                                    .value("episodes")
                                    .toObject()
                                    .value("edges")
                                    .toArray();

    QVector<ImdbEpisodeData> result;
    for (const auto& epEntry : episodes) {
        const QJsonObject node = epEntry.toObject().value("node").toObject();
        ImdbEpisodeData ep;

        ep.imdbId = ImdbId(node.value("id").toString());

        // Episode/season numbers
        const QJsonObject epNum = node.value("series")
                                      .toObject()
                                      .value("displayableEpisodeNumber")
                                      .toObject()
                                      .value("episodeNumber")
                                      .toObject();
        ep.seasonNumber = epNum.value("seasonNumber").toInt(-1);
        ep.episodeNumber = epNum.value("episodeNumber").toInt(-1);

        // Title
        const QString epTitle = node.value("titleText").toObject().value("text").toString().trimmed();
        if (!epTitle.isEmpty()) {
            ep.title = epTitle;
        }

        // Plot
        const QString plot = node.value("plot").toObject().value("plotText").toObject().value("plainText").toString();
        if (!plot.isEmpty()) {
            ep.overview = plot.trimmed();
        }

        // First aired
        const QJsonObject rd = node.value("releaseDate").toObject();
        const int rdYear = rd.value("year").toInt(-1);
        if (rdYear > 0) {
            ep.firstAired = QDate(rdYear, rd.value("month").toInt(1), rd.value("day").toInt(1));
        }

        // Rating
        const QJsonObject rs = node.value("ratingsSummary").toObject();
        const double rating = rs.value("aggregateRating").toDouble(0.0);
        const int votes = rs.value("voteCount").toInt(0);
        if (rating > 0 || votes > 0) {
            Rating r;
            r.rating = rating;
            r.voteCount = votes;
            r.source = "imdb";
            r.maxRating = 10;
            ep.ratings.append(r);
        }

        // Runtime
        const int rtSeconds = node.value("runtime").toObject().value("seconds").toInt(-1);
        if (rtSeconds > 0) {
            ep.runtime = std::chrono::minutes(qCeil(rtSeconds / 60.));
        }

        // Thumbnail
        const QString thumbUrl = node.value("primaryImage").toObject().value("url").toString();
        if (!thumbUrl.isEmpty()) {
            Poster p;
            p.thumbUrl = QUrl(sanitizeAmazonMediaUrl(thumbUrl));
            p.originalUrl = p.thumbUrl;
            ep.thumbnail = p;
        }

        // Certification
        const QString certRating = node.value("certificate").toObject().value("rating").toString().trimmed();
        if (!certRating.isEmpty()) {
            ep.certification = helper::mapCertification(Certification(certRating));
        }

        // Directors
        const QJsonArray dirs = node.value("directors").toObject().value("edges").toArray();
        for (const auto& d : dirs) {
            const QString name =
                d.toObject().value("node").toObject().value("name").toObject().value("nameText").toObject().value(
                    "text").toString().trimmed();
            if (!name.isEmpty()) {
                ep.directors.insert(name);
            }
        }

        // Writers
        const QJsonArray wrs = node.value("writers").toObject().value("edges").toArray();
        for (const auto& w : wrs) {
            const QString name =
                w.toObject().value("node").toObject().value("name").toObject().value("nameText").toObject().value(
                    "text").toString().trimmed();
            if (!name.isEmpty()) {
                ep.writers.insert(name);
            }
        }

        // Actors
        const QJsonArray castArr = node.value("cast").toObject().value("edges").toArray();
        for (const auto& c : castArr) {
            const QJsonObject cNode = c.toObject().value("node").toObject();
            const QJsonObject nameObj = cNode.value("name").toObject();
            const QString actorName = nameObj.value("nameText").toObject().value("text").toString().trimmed();
            if (!actorName.isEmpty()) {
                Actor actor;
                actor.name = actorName;
                actor.id = nameObj.value("id").toString();
                actor.thumb = sanitizeAmazonMediaUrl(nameObj.value("primaryImage").toObject().value("url").toString());
                const QJsonArray chars = cNode.value("characters").toArray();
                if (!chars.isEmpty()) {
                    actor.role = chars.at(0).toObject().value("name").toString().trimmed();
                }
                ep.actors.append(actor);
            }
        }

        if (ep.imdbId.isValid()) {
            result.append(ep);
        }
    }

    return result;
}

QVector<int> ImdbJsonParser::parseSeasonsFromGraphQL(const QString& json)
{
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return {};
    }

    const QJsonArray seasons = doc.object()
                                   .value("data")
                                   .toObject()
                                   .value("title")
                                   .toObject()
                                   .value("episodes")
                                   .toObject()
                                   .value("seasons")
                                   .toObject()
                                   .value("edges")
                                   .toArray();

    QVector<int> result;
    for (const auto& seasonEntry : seasons) {
        const int num = seasonEntry.toObject().value("node").toObject().value("seasonNumber").toInt(-1);
        if (num >= 0) {
            result.append(num);
        }
    }
    return result;
}

// =============================================================================
// Legacy HTML-based parsing (kept until Phase 6 cleanup)
// =============================================================================

ImdbData ImdbJsonParser::parseFromReferencePage(const QString& html, const Locale& preferredLocale)
{
    // Note: Expects HTML from https://www.imdb.com/title/tt________/reference
    QJsonDocument json = extractJsonFromHtml(html);

    ImdbJsonParser parser{};
    parser.parseAndAssignDetails(json, preferredLocale);
    parser.parseAndAssignDirectors(json);
    parser.parseAndAssignWriters(json);
    parser.parseAndStoreActors(json);

    return parser.m_data;
}

Optional<QString> ImdbJsonParser::parseOverviewFromPlotSummaryPage(const QString& html)
{
    // Note: Expects HTML from https://www.imdb.com/title/tt________/plotsummray
    QJsonDocument json = extractJsonFromHtml(html);

    ImdbJsonParser parser{};
    parser.parseAndAssignOverviewFromPlotSummary(json);

    return parser.m_data.overview;
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

void ImdbJsonParser::parseAndAssignDetails(const QJsonDocument& json, const Locale& preferredLocale)
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

    value = followJsonPath(json, IMDB_JSON_PATH_METACRITIC);
    if (value.isDouble()) {
        const int metascore = value.toInt(-1);
        if (metascore > 0) {
            Rating rating;
            rating.rating = metascore;
            rating.voteCount = 0;
            rating.source = "metacritic";
            rating.maxRating = 100;
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

void ImdbJsonParser::parseAndAssignOverviewFromPlotSummary(const QJsonDocument& json)
{
    const QJsonValue value = followJsonPath(json, IMDB_JSON_PATH_PLOTSUMMARY_SYNOPSIS);
    if (value.isString()) {
        m_data.overview = removeHtmlEntities(value.toString().trimmed());
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
