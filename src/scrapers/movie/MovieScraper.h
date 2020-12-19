#pragma once

#include "data/Locale.h"
#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "globals/ScraperResult.h"
#include "scrapers/ScraperInterface.h"
#include "settings/ScraperSettings.h"

#include <QMap>
#include <QString>
#include <QVector>
#include <vector>

class Movie;

namespace mediaelch {
namespace scraper {

/// \brief The MovieScraper class
/// This class is the base for every movie Scraper.
class MovieScraper : public QObject, public ScraperInterface
{
    Q_OBJECT

public:
    virtual void search(QString searchStr) = 0;
    virtual void loadData(QHash<MovieScraper*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos) = 0;
    virtual QSet<MovieScraperInfo> scraperSupports() = 0;
    virtual QSet<MovieScraperInfo> scraperNativelySupports() = 0;
    virtual QVector<mediaelch::Locale> supportedLanguages() = 0;
    virtual void changeLanguage(mediaelch::Locale locale) = 0;
    // Default language stored in settings.
    virtual mediaelch::Locale defaultLanguage() = 0;
    virtual QWidget* settingsWidget() = 0;
    virtual bool isAdult() const = 0;

signals:
    void searchDone(QVector<ScraperSearchResult>, ScraperError error);
};

} // namespace scraper
} // namespace mediaelch
