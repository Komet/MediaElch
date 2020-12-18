#pragma once

#include "settings/ScraperSettings.h"

#include <QNetworkReply>
#include <QString>
#include <QUrl>

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

class ScraperInterface
{
public:
    virtual QString name() const = 0;
    virtual QString identifier() const = 0;
    virtual bool hasSettings() const = 0;
    virtual void loadSettings(ScraperSettings& settings) = 0;
    virtual void saveSettings(ScraperSettings& settings) = 0;
    virtual ~ScraperInterface() = default;

    /// \todo Refactor to not do UI stuff here.
    void showNetworkError(const QNetworkReply& reply);
};
