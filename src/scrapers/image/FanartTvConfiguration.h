#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperConfiguration.h"
#include "settings/Settings.h"
#include "utils/Meta.h"

#include <QVector>

namespace mediaelch {
namespace scraper {

class FanartTvConfiguration : public QObject, public ScraperConfiguration
{
    Q_OBJECT
public:
    explicit FanartTvConfiguration(Settings& settings);
    ~FanartTvConfiguration() override = default;

    void init() override;

    ELCH_NODISCARD static mediaelch::Locale defaultLocale();
    ELCH_NODISCARD static QVector<Locale> supportedLanguages();

signals:
    void languageChanged(Locale language);
    void preferredDiscTypeChanged(QString discType);
    void personalApiKeyChanged(QString key);

public:
    ELCH_NODISCARD Locale language() override;
    void setLanguage(const Locale& value) override;

    ELCH_NODISCARD QString preferredDiscType();
    void setPreferredDiscType(const QString& value);

    ELCH_NODISCARD QString personalApiKey();
    void setPersonalApiKey(const QString& value);
};

} // namespace scraper
} // namespace mediaelch
