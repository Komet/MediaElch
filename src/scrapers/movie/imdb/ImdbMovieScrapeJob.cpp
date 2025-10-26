#include "scrapers/movie/imdb/ImdbMovieScrapeJob.h"

#include "globals/Helper.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/imdb/ImdbReferencePage.h"
#include "scrapers/movie/imdb/ImdbMovie.h"

#include <QJsonObject>
#include <QRegularExpression>

#include "scrapers/ScraperUtils.h"

namespace {

// clang-format off
const QVector<QString> IMDB_JSON_PATH_ID               = { "props", "pageProps", "mainColumnData", "id" };
const QVector<QString> IMDB_JSON_PATH_TITLE            = { "props", "pageProps", "mainColumnData", "titleText", "text" };
const QVector<QString> IMDB_JSON_PATH_ORIGINAL_TITLE   = { "props", "pageProps", "mainColumnData", "originalTitleText", "text" };
const QVector<QString> IMDB_JSON_PATH_OVERVIEW         = { "props", "pageProps", "aboveTheFoldData", "creditGroupings", "summaries", "edges", "0", "node", "plotText", "plaidHtml" };
const QVector<QString> IMDB_JSON_PATH_OUTLINE          = { "props", "pageProps", "mainColumnData", "plot", "plotText", "plainText" };
const QVector<QString> IMDB_JSON_PATH_DIRECTORS        = { "props", "pageProps", "mainColumnData", "directors" };
const QVector<QString> IMDB_JSON_PATH_WRITERS          = { "props", "pageProps", "mainColumnData", "writers" };
const QVector<QString> IMDB_JSON_PATH_CREW             = { "props", "pageProps", "mainColumnData", "crewV2" };
const QVector<QString> IMDB_JSON_PATH_RELEASE_DATE     = { "props", "pageProps", "mainColumnData", "releaseDate" };
const QVector<QString> IMDB_JSON_PATH_RUNTIME_SECONDS  = { "props", "pageProps", "aboveTheFoldData", "runtime", "seconds" };
const QVector<QString> IMDB_JSON_PATH_RATING           = { "props", "pageProps", "aboveTheFoldData", "ratingsSummary", "aggregateRating" };
const QVector<QString> IMDB_JSON_PATH_VOTE_COUNT       = { "props", "pageProps", "aboveTheFoldData", "ratingsSummary", "voteCount" };
const QVector<QString> IMDB_JSON_PATH_GENRES           = { "props", "pageProps", "aboveTheFoldData", "genres", "genres" };
const QVector<QString> IMDB_JSON_PATH_TAGLINE          = { "props", "pageProps", "mainColumnData", "taglines", "edges", "0", "node", "text" };
const QVector<QString> IMDB_JSON_PATH_TAGS             = { "props", "pageProps", "mainColumnData", "storylineKeywords", "edges" };
const QVector<QString> IMDB_JSON_PATH_KEYWORDS         = { "props", "pageProps", "aboveTheFoldData", "keywords", "edges" };
const QVector<QString> IMDB_JSON_PATH_POSTER           = { "props", "pageProps", "aboveTheFoldData", "primaryImage" };
const QVector<QString> IMDB_JSON_PATH_TRAILER          = { "props", "pageProps", "aboveTheFoldData", "primaryVideos", "edges" };

// Cast / Actors
const QVector<QString> IMDB_JSON_PATH_CREDIT_GROUPING = { "props", "pageProps", "mainColumnData", "creditGroupings", "edges" };
const QVector<QString> IMDB_JSON_PATH_CAST_NAME       = { "node", "name", "nameText", "text" };
const QVector<QString> IMDB_JSON_PATH_CAST_URL        = { "node", "name", "primaryImage", "url" };
const QVector<QString> IMDB_JSON_PATH_CAST_ROLE       = { "node", "creditedRoles", "edges",  "0", "node", "text" };

// clang-format on

} // namespace

namespace mediaelch {
namespace scraper {

ImdbMovieScrapeJob::ImdbMovieScrapeJob(ImdbApi& api,
    MovieScrapeJob::Config _config,
    bool loadAllTags,
    QObject* parent) :
    MovieScrapeJob(std::move(_config), parent),
    m_api{api},
    m_imdbId{config().identifier.str()},
    m_loadAllTags{loadAllTags}
{
}

void ImdbMovieScrapeJob::doStart()
{
    m_movie->clear(config().details);
    m_movie->setImdbId(m_imdbId);

    m_api.loadTitle(config().locale, m_imdbId, ImdbApi::PageKind::Reference, [this](QString html, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
            return;
        }

        QJsonDocument json = extractJsonFromHtml(html);

        parseAndAssignInfos(json);
        parseAndAssignPoster(json);
        parseAndStoreActors(json);

        // How many pages do we have to download? Count them.
        m_itemsLeftToDownloads = 1;

        // IMDb has an extra page listing all tags (popular movies can have more than 100 tags).
        if (m_loadAllTags) {
            ++m_itemsLeftToDownloads;
            loadTags();
        }

        // It's possible that none of the above items should be loaded.
        decreaseDownloadCount();
    });
}

void ImdbMovieScrapeJob::loadTags()
{
    const auto cb = [this](QString html, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignTags(html);

        } else {
            setScraperError(error);
        }
        decreaseDownloadCount();
    };
    m_api.loadTitle(config().locale, m_imdbId, ImdbApi::PageKind::Keywords, cb);
}

QJsonDocument ImdbMovieScrapeJob::extractJsonFromHtml(const QString& html)
{
    QRegularExpression rx(R"re(<script id="__NEXT_DATA__" type="application/json">(.*)</script>)re",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match = rx.match(html);
    if (match.hasMatch()) {
        return QJsonDocument::fromJson(match.captured(1).toUtf8());
    }
    return QJsonDocument{};
}

QJsonValue ImdbMovieScrapeJob::followJsonPath(const QJsonDocument& json, const QVector<QString>& paths)
{
    return followJsonPath(json.object(), paths);
}

QJsonValue ImdbMovieScrapeJob::followJsonPath(const QJsonObject& json, const QVector<QString>& paths)
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

QStringList ImdbMovieScrapeJob::valueToJsonStringArray(const QJsonArray& json)
{
    QStringList entries;
    for (const auto& director : json) {
        entries.append(removeHtmlEntities(director.toString()).trimmed());
    }
    return entries;
}

void ImdbMovieScrapeJob::parseAndAssignInfos(const QJsonDocument& json)
{
    using namespace std::chrono;

    QJsonValue value;

    value = followJsonPath(json, IMDB_JSON_PATH_ID);
    if (value.isString()) {
        QString id = value.toString();
        m_movie->setImdbId(ImdbId(id));
    }

    value = followJsonPath(json, IMDB_JSON_PATH_TITLE);
    if (value.isString()) {
        m_movie->setTitle(value.toString().trimmed());
    }

    value = followJsonPath(json, IMDB_JSON_PATH_ORIGINAL_TITLE);
    if (value.isString()) {
        m_movie->setOriginalTitle(value.toString().trimmed());
    }

    value = followJsonPath(json, IMDB_JSON_PATH_OVERVIEW);
    if (value.isString()) {
        m_movie->setOverview(removeHtmlEntities(value.toString().trimmed()));
    }

    value = followJsonPath(json, IMDB_JSON_PATH_OUTLINE);
    if (value.isString()) {
        m_movie->setOutline(removeHtmlEntities(value.toString().trimmed()));
    }

    value = followJsonPath(json, IMDB_JSON_PATH_DIRECTORS);
    if (value.isArray()) {
        QStringList directors = valueToJsonStringArray(value.toArray());
        m_movie->setDirector(directors.join(", "));
    }

    value = followJsonPath(json, IMDB_JSON_PATH_WRITERS);
    if (value.isArray()) {
        QStringList writers = valueToJsonStringArray(value.toArray());
        m_movie->setWriter(writers.join(", "));
    }

    value = followJsonPath(json, IMDB_JSON_PATH_GENRES);
    if (value.isArray()) {
        QStringList genres = valueToJsonStringArray(value.toArray());
        m_movie->setGenres(genres);
    }

    value = followJsonPath(json, IMDB_JSON_PATH_TAGLINE);
    if (value.isString()) {
        m_movie->setTagline(removeHtmlEntities(value.toString().trimmed()));
    }

    value = followJsonPath(json, IMDB_JSON_PATH_RUNTIME_SECONDS);
    if (value.isDouble()) {
        const int runtime = value.toInt(-1);
        if (runtime > 0) {
            m_movie->setRuntime(minutes(qCeil(runtime / 60.)));
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
                m_movie->setReleased(date);
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
            m_movie->ratings().setOrAddRating(rating);
        }
    }


    value = followJsonPath(json, IMDB_JSON_PATH_TAGS);
    if (value.isArray()) {
        for (const auto& tagObj : value.toArray()) {
            QString tag = tagObj.toObject().value("node").toObject().value("text").toString().trimmed();
            if (!tag.isEmpty()) {
                m_movie->addTag(tag);
            }
        }
    }
}

void ImdbMovieScrapeJob::parseAndAssignPoster(const QJsonDocument& json)
{
}

void ImdbMovieScrapeJob::parseAndStoreActors(const QJsonDocument& json)
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
            const QString name = followJsonPath(actorObj, IMDB_JSON_PATH_CAST_NAME).toString();
            const QString url = followJsonPath(actorObj, IMDB_JSON_PATH_CAST_URL).toString();
            const QString role = followJsonPath(actorObj, IMDB_JSON_PATH_CAST_ROLE).toString();
            if (!name.isEmpty()) {
                Actor actor;
                actor.name = name;
                actor.role = role;
                actor.thumb = url;
                m_movie->addActor(actor);
            }
        }
    }
}

void ImdbMovieScrapeJob::parseAndAssignTags(const QJsonDocument& json)
{
}

void ImdbMovieScrapeJob::parseAndAssignInfos(const QString& html)
{
    using namespace std::chrono;

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    const QString title = ImdbReferencePage::extractTitle(html);
    if (!title.isEmpty()) {
        m_movie->setTitle(title);
    }
    const QString originalTitle = ImdbReferencePage::extractOriginalTitle(html);
    if (!originalTitle.isEmpty()) {
        m_movie->setOriginalTitle(originalTitle);
    }

    ImdbReferencePage::extractDirectors(m_movie, html);
    ImdbReferencePage::extractWriters(m_movie, html);
    ImdbReferencePage::extractGenres(m_movie, html);
    ImdbReferencePage::extractTaglines(m_movie, html);

    if (!m_loadAllTags) {
        ImdbReferencePage::extractTags(m_movie, html);
    }

    QDate date = ImdbReferencePage::extractReleaseDate(html);
    if (date.isValid()) {
        m_movie->setReleased(date);
    }

    ImdbReferencePage::extractCertification(m_movie, html);

    rx.setPattern(R"re(Runtime</td>.*<li class="ipl-inline-list__item">\n\s+(\d+) min)re");
    match = rx.match(html);

    if (match.hasMatch()) {
        minutes runtime = minutes(match.captured(1).toInt());
        m_movie->setRuntime(runtime);
    }

    rx.setPattern(R"(<h4 class="inline">Runtime:</h4>[^<]*<time datetime="PT([0-9]+)M">)");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setRuntime(minutes(match.captured(1).toInt()));
    }

    ImdbReferencePage::extractOverview(m_movie, html);
    ImdbReferencePage::extractRating(m_movie, html);
    ImdbReferencePage::extractStudios(m_movie, html);
    ImdbReferencePage::extractCountries(m_movie, html);
}

void ImdbMovieScrapeJob::parseAndStoreActors(const QString& html)
{
    QRegularExpression rx(R"(<table class="cast_list">(.*)</table>)",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match = rx.match(html);
    if (!match.hasMatch()) {
        return;
    }

    const QString content = match.captured(1);
    rx.setPattern(R"(<tr class="[^"]*">(.*)</tr>)");

    QRegularExpressionMatchIterator actorRowsMatch = rx.globalMatch(content);

    while (actorRowsMatch.hasNext()) {
        QString actorHtml = actorRowsMatch.next().captured(1);

        QPair<Actor, QUrl> actorUrl;

        // Name
        rx.setPattern(R"re(<span class="itemprop" itemprop="name">([^<]+)</span>)re");
        match = rx.match(actorHtml);
        if (match.hasMatch()) {
            actorUrl.first.name = match.captured(1).trimmed();
        }

        // URL
        rx.setPattern(R"re(<a href="(/name/[^"]+)")re");
        match = rx.match(actorHtml);
        if (match.hasMatch()) {
            actorUrl.second = QUrl("https://www.imdb.com" + match.captured(1));
        }

        // Character
        rx.setPattern(R"(<td class="character">(.*)</td>)");
        match = rx.match(actorHtml);
        if (match.hasMatch()) {
            QString role = match.captured(1);
            // Everything between <div> and </div>
            rx.setPattern(R"(>(.*)</)");
            match = rx.match(role);
            if (match.hasMatch()) {
                role = match.captured(1);
            }
            actorUrl.first.role = role.remove("(voice)")
                                      .trimmed() //
                                      .replace(QRegularExpression("\\s\\s+"), " ")
                                      .trimmed();
        }

        rx.setPattern(R"re(loadlate="([^"]+)")re");
        match = rx.match(actorHtml);
        if (match.hasMatch()) {
            actorUrl.first.thumb = sanitizeAmazonMediaUrl(match.captured(1));
        }

        m_movie->addActor(actorUrl.first);
    }
}

void ImdbMovieScrapeJob::parseAndAssignTags(const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    if (m_loadAllTags) {
        rx.setPattern(R"(<a[^>]+href="/search/(?:title/\?)keyword[^"]+"\n?>([^<]+)</a>)");
    } else {
        rx.setPattern(R"(<a[^>]+href="/keyword/[^"]+"[^>]*>([^<]+)</a>)");
    }

    QRegularExpressionMatchIterator match = rx.globalMatch(html);
    while (match.hasNext()) {
        m_movie->addTag(match.next().captured(1).trimmed());
    }
}

void ImdbMovieScrapeJob::parseAndAssignPoster(const QString& html)
{
    QString regex = QStringLiteral(R"url(<meta property='og:image' content="([^"]+)")url");
    QRegularExpression rx(regex, QRegularExpression::InvertedGreedinessOption);

    QRegularExpressionMatch match = rx.match(html);
    if (match.hasMatch()) {
        const QUrl url(sanitizeAmazonMediaUrl(match.captured(1)));
        if (!url.isValid()) {
            return;
        }

        Poster p;
        p.thumbUrl = url;
        p.originalUrl = url;
        m_movie->images().addPoster(p);
    }
}

QString ImdbMovieScrapeJob::sanitizeAmazonMediaUrl(QString url)
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

void ImdbMovieScrapeJob::decreaseDownloadCount()
{
    --m_itemsLeftToDownloads;
    if (m_itemsLeftToDownloads == 0) {
        emitFinished();
    }
}

} // namespace scraper
} // namespace mediaelch
