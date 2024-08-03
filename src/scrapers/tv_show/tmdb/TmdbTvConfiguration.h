#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperConfiguration.h"
#include "settings/Settings.h"
#include "utils/Meta.h"

#include <QVector>

namespace mediaelch {
namespace scraper {

class TmdbTvConfiguration : public QObject, public ScraperConfiguration
{
    Q_OBJECT
public:
    explicit TmdbTvConfiguration(Settings& settings);
    ~TmdbTvConfiguration() override = default;

    void init() override;

    ELCH_NODISCARD static mediaelch::Locale defaultLocale();
    ELCH_NODISCARD static QVector<Locale> supportedLanguages();

signals:
    void languageChanged(Locale language);

public:
    ELCH_NODISCARD Locale language() override;
    void setLanguage(const Locale& value) override;
};

} // namespace scraper
} // namespace mediaelch
