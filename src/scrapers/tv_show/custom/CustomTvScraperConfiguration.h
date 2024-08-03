#pragma once

#include "scrapers/ScraperConfiguration.h"
#include "scrapers/ScraperInfos.h"
#include "settings/Settings.h"
#include "utils/Meta.h"

#include <QMap>
#include <QString>

namespace mediaelch {
namespace scraper {

class TmdbTv;
class ImdbTv;
class TvScraper;

class CustomTvScraperConfiguration : public QObject, public ScraperConfiguration
{
    Q_OBJECT
public:
    using ScraperForShowDetails = QMap<ShowScraperInfo, QString>;
    using ScraperForEpisodeDetails = QMap<EpisodeScraperInfo, QString>;

    CustomTvScraperConfiguration(Settings& settings,
        TmdbTv& _tmdbTv,
        ImdbTv& _imdbTv,
        ScraperForShowDetails _scraperForShowDetails,
        ScraperForEpisodeDetails _scraperForEpisodeDetails);
    ~CustomTvScraperConfiguration() override = default;

    void init() override;

public:
    ELCH_NODISCARD Locale language() override { return Locale::NoLocale; };
    void setLanguage(const Locale& value) override { Q_UNUSED(value) /* no-op */ };

private:
    Settings& m_settings;

public:
    TmdbTv* tmdbTv = nullptr;
    ImdbTv* imdbTv = nullptr;

    ScraperForShowDetails scraperForShowDetails;
    ScraperForEpisodeDetails scraperForEpisodeDetails;

public:
    TvScraper* scraperForId(const QString& id) const;
};

} // namespace scraper
} // namespace mediaelch
