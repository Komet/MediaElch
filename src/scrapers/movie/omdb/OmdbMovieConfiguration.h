#pragma once

#include "scrapers/ScraperConfiguration.h"
#include "settings/Settings.h"
#include "utils/Meta.h"

#include "data/Locale.h"

#include <QObject>

namespace mediaelch {
namespace scraper {

class OmdbMovieConfiguration : public QObject, public ScraperConfiguration
{
    Q_OBJECT
public:
    explicit OmdbMovieConfiguration(Settings& settings, QObject* parent = nullptr);
    ~OmdbMovieConfiguration() override = default;

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
