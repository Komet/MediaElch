#pragma once

#include "settings/ScraperSettings.h"

#include <QNetworkReply>
#include <QString>
#include <QUrl>

/// @brief Language name/key pair for scrapers. Used for settings.
struct ScraperLanguage
{
    QString languageName;
    QString languageKey;
};

struct ScraperSearchError
{
    enum class ErrorType
    {
        NoError,
        NetworkError,
        InternalError
    };
    ErrorType error = ErrorType::NoError;
    QString message;

    bool hasError() const { return (error != ErrorType::NoError); }
};

struct ScraperLoadError
{
    enum class ErrorType
    {
        NoError,
        NetworkError
    };
    ErrorType error = ErrorType::NoError;
    QString message;

    bool hasError() const { return (error != ErrorType::NoError); }
};

class ScraperInterface
{
public:
    virtual QString name() const = 0;
    virtual QString identifier() const = 0;
    virtual bool hasSettings() const = 0;
    virtual void loadSettings(const ScraperSettings& settings) = 0;
    virtual void saveSettings(ScraperSettings& settings) = 0;
    virtual ~ScraperInterface() = default;

    /// \todo Refactor to not do UI stuff here.
    void showNetworkError(const QNetworkReply& reply);
};
