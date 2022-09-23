#pragma once

#include "globals/Globals.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/ScraperResult.h"
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
    /// \brief Returns the selected locale.
    /// \note Do not call this method "locale" as it refers to QWidget's locale().
    const mediaelch::Locale& scraperLocale() const;
    SeasonOrder seasonOrder() const;
    const QSet<ShowScraperInfo>& showDetailsToLoad() const;
    const QSet<EpisodeScraperInfo>& episodeDetailsToLoad() const;
    TvShowUpdateType updateType() const;

private:
    Ui::TvShowSearch* ui = nullptr;
};
