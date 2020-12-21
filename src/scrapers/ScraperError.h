#pragma once

#include "network/HttpStatusCodes.h"

#include <QJsonParseError>
#include <QNetworkReply>
#include <QString>

namespace mediaelch {

struct ScraperError
{
    enum class Type
    {
        NoError,
        /// \brief A network error, e.g. no internet connection, timeout, ...
        NetworkError,
        /// \brief Special case of the network error. Some providers may return a 404 even for searches.
        NetworkNotFoundError,
        /// \brief Some internal error occurred. Should never happen.
        InternalError,
        /// \brief Scraper configuration error, e.g. if a invalid query was used.
        ConfigError,
        /// \brief General API error
        ApiError,
        /// \brief An API may have a rate limit.
        ApiRateLimitReached,
        /// \brief A token may become invalid after a time.
        ApiUnauthorized
    };
    Type error = Type::NoError;
    /// \brief User-readable error message. Should be translated.
    QString message;
    /// \brief Technical error, used for debug output.
    QString technical;

    bool hasError() const { return (error != Type::NoError); }
    bool is404() const { return (error == Type::NetworkNotFoundError); }
};

/// \brief A utility function to create a scraper error object based on common API steps.
/// \details Most scrapers have a similar setup:  They use JSON as a response and may have
///          a rate limit.  This function checks all those cases and creates a scraper error
///          with default messages, etc.
ScraperError makeScraperError(const QString& data, const QNetworkReply& reply, const QJsonParseError& parseError);

/// \brief A utility function to create a scraper error object based on a network reply.
ScraperError replyToScraperError(const QNetworkReply& reply);

} // namespace mediaelch
