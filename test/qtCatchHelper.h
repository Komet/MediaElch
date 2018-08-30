
#include "globals/Globals.h"

#include <QEventLoop>
#include <QList>
#include <QString>
#include <ostream>

// Some operators for better warnings with Catch2

inline std::ostream &operator<<(std::ostream &os, const QByteArray &value)
{
    return os << '"' << (value.isEmpty() ? "" : value.constData()) << '"';
}

inline std::ostream &operator<<(std::ostream &os, const QLatin1String &value)
{
    return os << '"' << value.latin1() << '"';
}

inline std::ostream &operator<<(std::ostream &os, const QString &value)
{
    return os << value.toLocal8Bit();
}


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
