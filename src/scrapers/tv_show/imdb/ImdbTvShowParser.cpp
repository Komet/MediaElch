#include "scrapers/tv_show/imdb/ImdbTvShowParser.h"

#include "scrapers/ScraperInterface.h"
#include "tv_shows/TvShow.h"

#include <QDate>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValueRef>
#include <QRegularExpression>

using namespace std::chrono_literals;

namespace mediaelch {
namespace scraper {

void ImdbTvShowParser::parseInfos(const QString& html)
{
    QJsonDocument json = extractMetaJson(html);
    if (json.isEmpty() || !json.isObject()) {
        return;
    }

    QJsonObject data = json.object();

    m_show.setTitle(data.value("name").toString());
    m_show.setOverview(data.value("description").toString());
    m_show.setFirstAired(QDate::fromString(data.value("datePublished").toString(), "yyyy-MM-dd"));
    m_show.setCertification(Certification(data.value("contentRating").toString()));

    // -------------------------------------
    QJsonObject ratingObj = data.value("aggregateRating").toObject();
    Rating rating;
    rating.source = "imdb";
    rating.minRating = 0;
    rating.maxRating = 10;
    rating.voteCount = ratingObj.value("ratingCount").toInt();
    // Rating value is stored as a string in IMDb's JSON.
    rating.rating = ratingObj.value("ratingValue").toString().toDouble();
    if (rating.rating != 0.0 || rating.voteCount != 0) {
        m_show.ratings().push_back(rating);
    }

    // -------------------------------------
    const QStringList tags = data.value("keywords").toString().split(',');
    for (const auto& tag : tags) {
        m_show.addTag(tag);
    }

    // -------------------------------------
    Poster poster;
    poster.id = data.value("image").toString();
    poster.thumbUrl = poster.id;
    poster.originalUrl = poster.id;
    if (!poster.id.isEmpty()) {
        m_show.addPoster(poster);
    }

    // -------------------------------------
    const QJsonArray genres = data.value("genre").toArray();
    for (QJsonValue genreVal : genres) {
        QString genre = genreVal.toString();
        if (!genre.isEmpty()) {
            m_show.addGenre(genre);
        }
    }

    m_show.setRuntime(extractRuntime(html));
}


QJsonDocument ImdbTvShowParser::extractMetaJson(const QString& html)
{
    QRegularExpression rx(
        R"(<script type="application/ld\+json">(.*?)</script>)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = rx.match(html);
    if (!match.hasMatch()) {
        m_error.error = ScraperError::Type::InternalError;
        m_error.message = tr("Could not extract JSON details from IMDb page!");
        qWarning() << "[ImdbTvShowParser] Could not extract JSON details from IMDb page!";
        return {};
    }

    QJsonParseError parseError{};
    QJsonDocument parsedJson = QJsonDocument::fromJson(match.captured(1).toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        m_error.error = ScraperError::Type::InternalError;
        m_error.message = tr("Could not parse JSON from IMDb page!");
        qWarning() << "[ImdbTvShowParser] Could not parse IMDb json:" << parseError.errorString() //
                   << "at offset" << parseError.offset;

    } else if (!parsedJson.isObject()) {
        m_error.error = ScraperError::Type::InternalError;
        m_error.message = tr("Expected parsed IMDb JSON to be an object!");
        qWarning() << "[ImdbTvShowParser] IMDb json is not an object!";
    }

    return parsedJson;
}

std::chrono::minutes ImdbTvShowParser::extractRuntime(const QString& html)
{
    QRegularExpression rx(
        R"(<h3 class="subheading">Technical Specs</h3>(.*?)</time>)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = rx.match(html);
    if (!match.hasMatch()) {
        return 0min;
    }

    rx.setPatternOptions(QRegularExpression::NoPatternOption);
    rx.setPattern(R"(<time datetime="PT(\d+)M">)");

    QRegularExpressionMatch timeMatch = rx.match(match.captured(1));
    if (!timeMatch.hasMatch()) {
        return 0min;
    }

    return std::chrono::minutes(timeMatch.captured(1).toInt());
}


} // namespace scraper
} // namespace mediaelch
