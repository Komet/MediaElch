#pragma once

#include <QString>

// TODO: Add namespace mediaelch when all scrapers are inside this namespace.

struct ScraperError
{
    enum class Type
    {
        NoError,
        /// \brief A network error, e.g. no internet connection, timeout, ...
        NetworkError,
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
    QString message;

    bool hasError() const { return (error != Type::NoError); }
};
