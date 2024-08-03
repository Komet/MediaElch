#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperConfiguration.h"
#include "settings/Settings.h"
#include "utils/Meta.h"

namespace mediaelch {
namespace scraper {

class CustomMovieScraperConfiguration : public QObject, public ScraperConfiguration
{
    Q_OBJECT
public:
    explicit CustomMovieScraperConfiguration(Settings& settings, QObject* parent = nullptr);
    virtual ~CustomMovieScraperConfiguration() = default;

    void init() override;

public:
    ELCH_NODISCARD Locale language() override;
    void setLanguage(const Locale& value) override;

    ELCH_NODISCARD QMap<MovieScraperInfo, QString> detailScraperMap() const;
    void setDetailScraperMap(QMap<MovieScraperInfo, QString> customMovieScraper);

signals:
    void languageChanged(Locale language);
    void detailScraperMapChanged(Locale language);
};

} // namespace scraper
} // namespace mediaelch
