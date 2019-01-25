#pragma once

#include "data/ScraperInterface.h"
#include "globals/Globals.h"
#include "settings/ScraperSettings.h"

#include <QList>
#include <QMap>
#include <QString>
#include <vector>

class Movie;
struct ScraperSearchResult;

/**
 * @brief The MovieScraperInterface class
 * This class is the base for every movie Scraper.
 */
class MovieScraperInterface : public ScraperInterface, public QObject
{
public:
    virtual void search(QString searchStr) = 0;
    virtual void loadData(QMap<MovieScraperInterface *, QString> ids, Movie *movie, QList<MovieScraperInfos> infos) = 0;
    virtual QList<MovieScraperInfos> scraperSupports() = 0;
    virtual QList<MovieScraperInfos> scraperNativelySupports() = 0;
    virtual std::vector<ScraperLanguage> supportedLanguages() = 0;
    virtual void changeLanguage(QString languageKey) = 0;
    // Default language stored in settings.
    virtual QString defaultLanguageKey() = 0;
    virtual QWidget *settingsWidget() = 0;
    virtual bool isAdult() const = 0;

signals:
    virtual void searchDone(QList<ScraperSearchResult>) = 0;
};
