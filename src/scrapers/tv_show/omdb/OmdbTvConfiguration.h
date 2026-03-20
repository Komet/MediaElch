#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperConfiguration.h"
#include "settings/Settings.h"
#include "utils/Meta.h"

#include <QObject>

namespace mediaelch {
namespace scraper {

class OmdbTvConfiguration : public QObject, public ScraperConfiguration
{
    Q_OBJECT
public:
    explicit OmdbTvConfiguration(Settings& settings, QObject* parent = nullptr);
    ~OmdbTvConfiguration() override = default;

    void init();

    ELCH_NODISCARD Locale language() override;
    void setLanguage(const Locale& value) override;

    ELCH_NODISCARD QString apiKey();
    void setApiKey(const QString& value);

signals:
    void apiKeyChanged(QString apiKey);
};

} // namespace scraper
} // namespace mediaelch
