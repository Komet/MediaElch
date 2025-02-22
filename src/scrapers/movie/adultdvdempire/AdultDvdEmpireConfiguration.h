#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperConfiguration.h"
#include "settings/Settings.h"
#include "utils/Meta.h"

namespace mediaelch {
namespace scraper {

class AdultDvdEmpireConfiguration : public QObject, public ScraperConfiguration
{
    Q_OBJECT
public:
    explicit AdultDvdEmpireConfiguration(Settings& settings);
    ~AdultDvdEmpireConfiguration() override = default;

    void init() override;

    ELCH_NODISCARD static mediaelch::Locale defaultLocale();
    ELCH_NODISCARD static QVector<Locale> supportedLanguages();

public:
    ELCH_NODISCARD Locale language() override;
    void setLanguage(const Locale& value) override;

    ELCH_NODISCARD bool storeBackCoverAsFanart();
    void setStoreBackCoverAsFanart(const bool& value);

signals:
    void languageChanged(Locale language);
    void storeBackCoverAsFanartChanged(bool value);
};

} // namespace scraper
} // namespace mediaelch
