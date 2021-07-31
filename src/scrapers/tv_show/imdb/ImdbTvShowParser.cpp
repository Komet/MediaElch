#include "scrapers/tv_show/imdb/ImdbTvShowParser.h"

#include "log/Log.h"
#include "scrapers/ScraperInterface.h"
#include "tv_shows/TvShow.h"

#include <QDate>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValueRef>
#include <QRegularExpression>
#include <QTextDocument>

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
    m_show.setOverview(removeHtmlEntities(data.value("description").toString()));
    m_show.setFirstAired(QDate::fromString(data.value("datePublished").toString(), Qt::ISODate));
    m_show.setCertification(Certification(data.value("contentRating").toString()));

    // -------------------------------------
    QJsonObject ratingObj = data.value("aggregateRating").toObject();
    Rating rating;
    rating.source = "imdb";
    rating.minRating = 0;
    rating.maxRating = 10;
    rating.voteCount = ratingObj.value("ratingCount").toInt();

    if (ratingObj.value("ratingValue").type() == QJsonValue::String) {
        // Rating value was stored as a string in IMDb's JSON in 2020.
        rating.rating = ratingObj.value("ratingValue").toString().toDouble();
    } else {
        // In 2021, it's a double.
        rating.rating = ratingObj.value("ratingValue").toDouble();
    }
    if (rating.rating != 0.0 || rating.voteCount != 0) {
        m_show.ratings().setOrAddRating(rating);
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
    for (const QJsonValue& genreVal : genres) {
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
        qCWarning(generic) << "[ImdbTvShowParser] Could not extract JSON details from IMDb page!";
        return {};
    }

    QJsonParseError parseError{};
    QJsonDocument parsedJson = QJsonDocument::fromJson(match.captured(1).toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        m_error.error = ScraperError::Type::InternalError;
        m_error.message = tr("Could not parse JSON from IMDb page!");
        qCWarning(generic) << "[ImdbTvShowParser] Could not parse IMDb json:" << parseError.errorString() //
                           << "at offset" << parseError.offset;

    } else if (!parsedJson.isObject()) {
        m_error.error = ScraperError::Type::InternalError;
        m_error.message = tr("Expected parsed IMDb JSON to be an object!");
        qCWarning(generic) << "[ImdbTvShowParser] IMDb json is not an object!";
    }

    return parsedJson;
}

std::chrono::minutes ImdbTvShowParser::extractRuntime(const QString& html)
{
    QRegularExpression rx(R"(Runtime</span><div [^>]+><ul [^>]+><li [^>]+><span [^>]+>(?:(\d+)h )?(\d+)min)",
        QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = rx.match(html);
    if (!match.hasMatch()) {
        return 0min;
    }

    if (match.capturedTexts().size() >= 3) {
        return std::chrono::hours(match.captured(1).toInt()) + std::chrono::minutes(match.captured(2).toInt());
    }

    return std::chrono::minutes(match.captured(1).toInt());
}

QString ImdbTvShowParser::removeHtmlEntities(QString str) const
{
    QTextDocument doc;
    doc.setHtml(std::move(str));
    return doc.toPlainText();
}


} // namespace scraper
} // namespace mediaelch
