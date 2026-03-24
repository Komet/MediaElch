#include "ImdbJsonParser.h"

#include "data/Actor.h"
#include "data/ImdbId.h"
#include "data/Poster.h"
#include "globals/Helper.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
#include <qmath.h>

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

    // Backdrops — from images list (excluding the primary poster image)
    const QJsonArray imageEdges = title.value("images").toObject().value("edges").toArray();
    for (const QJsonValue& edge : imageEdges) {
        const QJsonObject node = edge.toObject().value("node").toObject();
        const QString imgUrl = node.value("url").toString();
        if (!imgUrl.isEmpty() && imgUrl != posterUrl) {
            const QUrl url(sanitizeAmazonMediaUrl(imgUrl));
            if (url.isValid()) {
                Poster p;
                p.thumbUrl = url;
                p.originalUrl = url;
                m_data.backdrops.append(p);
            }
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

        // Episode/season numbers (returned as text strings from displayableEpisodeNumber)
        const QJsonObject den = node.value("series")
                                    .toObject()
                                    .value("displayableEpisodeNumber")
                                    .toObject();
        bool seasonOk = false;
        bool episodeOk = false;
        ep.seasonNumber = den.value("displayableSeason").toObject().value("text").toString().toInt(&seasonOk);
        ep.episodeNumber = den.value("episodeNumber").toObject().value("text").toString().toInt(&episodeOk);
        if (!seasonOk) {
            ep.seasonNumber = -1;
        }
        if (!episodeOk) {
            ep.episodeNumber = -1;
        }

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
                                   .toArray();

    QVector<int> result;
    for (const auto& seasonEntry : seasons) {
        const int num = seasonEntry.toObject().value("number").toInt(-1);
        if (num >= 0) {
            result.append(num);
        }
    }
    return result;
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
