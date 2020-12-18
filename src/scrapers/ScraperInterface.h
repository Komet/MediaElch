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
        NetworkError,
        InternalError,
        ConfigError
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
