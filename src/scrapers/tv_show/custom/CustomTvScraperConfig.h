#pragma once

#include "globals/ScraperInfos.h"

#include <QMap>
#include <QString>

namespace mediaelch {
namespace scraper {

class TmdbTv;
class TheTvDb;
class ImdbTv;
class TvScraper;

class CustomTvScraperConfig
{
public:
    using ScraperForShowDetails = QMap<ShowScraperInfo, QString>;
    using ScraperForEpisodeDetails = QMap<EpisodeScraperInfo, QString>;

    CustomTvScraperConfig(TmdbTv& _tmdbTv,
        TheTvDb& _theTvDb,
        ImdbTv& _imdbTv,
        ScraperForShowDetails _scraperForShowDetails,
        ScraperForEpisodeDetails _scraperForEpisodeDetails);

public:
    TmdbTv* tmdbTv = nullptr;
    TheTvDb* theTvDb = nullptr;
    ImdbTv* imdbTv = nullptr;

    ScraperForShowDetails scraperForShowDetails;
    ScraperForEpisodeDetails scraperForEpisodeDetails;

public:
    TvScraper* scraperForId(const QString& id) const;
};

} // namespace scraper
} // namespace mediaelch
