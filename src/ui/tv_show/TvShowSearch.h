#pragma once

#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "globals/ScraperResult.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/tv_show/TvScraper.h"

#include <QDialog>

namespace Ui {
class TvShowSearch;
}

class TvShowSearch : public QDialog
{
    Q_OBJECT

public:
    explicit TvShowSearch(QWidget* parent = nullptr);
    ~TvShowSearch() override;

    void setSearchType(TvShowType type);

public slots:
    int execWithSearch(QString searchString);

public:
    QString showIdentifier();
    mediaelch::scraper::TvScraper* scraper();
    const mediaelch::Locale& locale() const;
    SeasonOrder seasonOrder() const;
    const QSet<ShowScraperInfo>& showDetailsToLoad() const;
    const QSet<EpisodeScraperInfo>& episodeDetailsToLoad() const;
    TvShowUpdateType updateType() const;

private:
    Ui::TvShowSearch* ui = nullptr;
};
