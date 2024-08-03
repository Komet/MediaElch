#pragma once

#include "scrapers/ScraperError.h"

#include <QNetworkReply>
#include <QString>
#include <QUrl>

// TODO: Remove when the GUI is finally split of our scrapers

class ScraperInterface
{
public:
    virtual ~ScraperInterface() = default;

    /// \todo Refactor to not do UI stuff here.
    void showNetworkError(const QNetworkReply& reply);
    /// \todo Refactor to not do UI stuff here.
    void showNetworkError(const mediaelch::ScraperError& error);
};
