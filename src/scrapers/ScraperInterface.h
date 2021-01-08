#pragma once

#include "scrapers/ScraperError.h"
#include "settings/ScraperSettings.h"

#include <QNetworkReply>
#include <QString>
#include <QUrl>

// TODO: Remove when the GUI is finally split of our scrapers

class ScraperInterface
{
public:
    virtual bool hasSettings() const = 0;
    virtual void loadSettings(ScraperSettings& settings) = 0;
    virtual void saveSettings(ScraperSettings& settings) = 0;
    virtual ~ScraperInterface() = default;

    /// \todo Refactor to not do UI stuff here.
    void showNetworkError(const QNetworkReply& reply);
    /// \todo Refactor to not do UI stuff here.
    void showNetworkError(const mediaelch::ScraperError& error);
};
