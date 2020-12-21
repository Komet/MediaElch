#pragma once

#include "concerts/Concert.h"
#include "globals/Globals.h"
#include "globals/ScraperResult.h"
#include "scrapers/ScraperInterface.h"

class Concert;

namespace mediaelch {
namespace scraper {

/// \brief The ConcertScraperInterface class
/// This class is the base for every concert Scraper.
class ConcertScraperInterface : public QObject, public ScraperInterface
{
    Q_OBJECT

public:
    explicit ConcertScraperInterface(QObject* parent = nullptr) : QObject(parent) {}
    virtual QString name() const = 0;
    virtual QString identifier() const = 0;
    virtual void search(QString searchStr) = 0;
    virtual void loadData(TmdbId id, Concert* concert, QSet<ConcertScraperInfo> infos) = 0;
    virtual QSet<ConcertScraperInfo> scraperSupports() = 0;
    virtual QWidget* settingsWidget() = 0;

signals:
    void searchDone(QVector<ScraperSearchResult>);
};

} // namespace scraper
} // namespace mediaelch
