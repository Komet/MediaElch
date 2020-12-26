#include "scrapers/ScraperError.h"

#include <QObject>

namespace mediaelch {

ScraperError replyToScraperError(const QNetworkReply& reply)
{
    if (reply.error() == QNetworkReply::NoError) {
        return {};
    }

    const auto httpStatusCode = HttpStatusCode(reply.attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    ScraperError error;

    if (reply.error() == QNetworkReply::ContentNotFoundError) {
        error.error = ScraperError::Type::NetworkNotFoundError;
        error.message = QObject::tr("Network Error: %1").arg(mediaelch::translateNetworkError(reply.error()));
        error.technical = reply.errorString();
        return error;
    }

    if (reply.error() != QNetworkReply::UnknownNetworkError) {
        error.error = ScraperError::Type::NetworkError;
        error.message = QObject::tr("Network Error: %1").arg(mediaelch::translateNetworkError(reply.error()));
        error.technical = reply.errorString();
        return error;
    }

    if (httpStatusCode == HttpStatusCode::TooManyRequests) {
        error.error = ScraperError::Type::ApiRateLimitReached;
        error.message = QObject::tr("The scraper's rate limit reached. Please wait ~10 seconds and try again.");
        return error;
    }

    error.error = ScraperError::Type::NetworkError;
    error.message = QObject::tr("An unknown network error occurred. Are you connected to the internet?");
    error.technical = reply.errorString();

    return error;
}

ScraperError makeScraperError(const QString& data, const QNetworkReply& reply, const QJsonParseError& parseError)
{
    ScraperError error;
    if (reply.error() != QNetworkReply::NoError) {
        return replyToScraperError(reply);
    }
    if (data.isEmpty()) {
        error.error = ScraperError::Type::ApiError;
        error.message = QObject::tr("The scraper did not respond with any data.");

    } else if (parseError.error != QJsonParseError::NoError) {
        error.error = ScraperError::Type::ApiError;
        error.message = QObject::tr("The scraper response could not be parsed.");
        error.technical = parseError.errorString();
    }

    return error;
}
} // namespace mediaelch
