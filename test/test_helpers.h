#ifndef TEST_QT_CATCH_HELPER
#define TEST_QT_CATCH_HELPER

#define CATCH_CONFIG_ENABLE_CHRONO_STRINGMAKER

// Catch2 is included in each test cpp file
#include "third_party/catch2/catch.hpp"

#include "test/helpers/debug_output.h"
#include "test/helpers/matchers.h"

#include "data/Movie.h"
#include "globals/Globals.h"
#include "movies/MovieController.h"

#include <QEventLoop>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

/**
 * @brief Searches for searchStr and returns the results synchronously using the given Scraper.
 */
template<class ScraperInterfaceT>
QList<ScraperSearchResult> searchScraperSync(ScraperInterfaceT &scraper, QString search)
{
    QList<ScraperSearchResult> results;
    QEventLoop loop;
    loop.connect(&scraper, &ScraperInterfaceT::searchDone, [&](QList<ScraperSearchResult> res) {
        results = res;
        loop.quit();
    });
    scraper.search(search);
    loop.exec();
    return results;
}

/**
 * @brief Loads movie data synchronously
 */
template<class ScraperInterfaceT>
void loadDataSync(ScraperInterfaceT &scraper,
    QMap<MovieScraperInterface *, QString> ids,
    Movie &movie,
    QList<MovieScraperInfos> infos)
{
    QList<ScraperSearchResult> results;
    QEventLoop loop;
    loop.connect(movie.controller(), &MovieController::sigInfoLoadDone, &loop, &QEventLoop::quit);
    scraper.loadData(ids, &movie, infos);
    loop.exec();
}

#endif // TEST_QT_CATCH_HELPER
