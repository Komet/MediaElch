#pragma once

#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"
#include "settings/ScraperSettings.h"

#include <QMap>
#include <QString>
#include <QVector>
#include <vector>

class Movie;
struct ScraperSearchResult;

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

/// @brief The MovieScraperInterface class
/// This class is the base for every movie Scraper.
class MovieScraperInterface : public QObject, public ScraperInterface
{
    Q_OBJECT

public:
    virtual void search(QString searchStr) = 0;
    virtual void
    loadData(QMap<MovieScraperInterface*, QString> ids, Movie* movie, QVector<MovieScraperInfos> infos) = 0;
    virtual QVector<MovieScraperInfos> scraperSupports() = 0;
    virtual QVector<MovieScraperInfos> scraperNativelySupports() = 0;
    virtual std::vector<ScraperLanguage> supportedLanguages() = 0;
    virtual void changeLanguage(QString languageKey) = 0;
    // Default language stored in settings.
    virtual QString defaultLanguageKey() = 0;
    virtual QWidget* settingsWidget() = 0;
    virtual bool isAdult() const = 0;

signals:
    void searchDone(QVector<ScraperSearchResult>, ScraperSearchError error);
};
