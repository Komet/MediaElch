#ifndef SCRAPER_H
#define SCRAPER_H

#include "globals/Globals.h"

#include <QList>
#include <QMap>
#include <QSettings>
#include <QString>
#include <vector>

class Movie;
struct ScraperSearchResult;

/**
 * @brief The ScraperInterface class
 * This class is the base for every movie Scraper.
 */
class ScraperInterface : public QObject
{
public:
    virtual QString name() = 0;
    virtual QString identifier() = 0;
    virtual void search(QString searchStr) = 0;
    virtual void loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<MovieScraperInfos> infos) = 0;
    virtual bool hasSettings() = 0;
    virtual void loadSettings(QSettings &settings) = 0;
    virtual void saveSettings(QSettings &settings) = 0;
    virtual QList<MovieScraperInfos> scraperSupports() = 0;
    virtual QList<MovieScraperInfos> scraperNativelySupports() = 0;
    virtual std::vector<ScraperLanguage> supportedLanguages() = 0;
    virtual void changeLanguage(QString languageKey) = 0;
    // Default language stored in settings.
    virtual QString defaultLanguageKey() = 0;
    virtual QWidget *settingsWidget() = 0;
    virtual bool isAdult() = 0;

signals:
    virtual void searchDone(QList<ScraperSearchResult>) = 0;
};

#endif // SCRAPER_H
