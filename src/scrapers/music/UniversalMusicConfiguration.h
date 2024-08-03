#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperConfiguration.h"
#include "settings/Settings.h"
#include "utils/Meta.h"

#include <QVector>

namespace mediaelch {
namespace scraper {

class UniversalMusicConfiguration : public QObject, public ScraperConfiguration
{
    Q_OBJECT
public:
    explicit UniversalMusicConfiguration(Settings& settings);
    ~UniversalMusicConfiguration() override;

    void init() override;

    ELCH_NODISCARD static mediaelch::Locale defaultLocale();
    ELCH_NODISCARD static QVector<Locale> supportedLanguages();

signals:
    void languageChanged(Locale language);
    void preferredScraperChanged(QString preferredScraped);

public:
    ELCH_NODISCARD Locale language() override;
    void setLanguage(const Locale& value) override;

    ELCH_NODISCARD QString preferredScraper();
    void setPreferredScraper(const QString& value);
};

} // namespace scraper
} // namespace mediaelch
