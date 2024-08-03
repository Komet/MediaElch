#pragma once

#include "data/Locale.h"
#include "settings/Settings.h"
#include "utils/Meta.h"

#include <QString>

namespace mediaelch {

class ScraperConfiguration
{
public:
    ScraperConfiguration(QString scraperId, Settings& settings);
    virtual ~ScraperConfiguration() = default;

    virtual void init() = 0;

public:
    // For convenience, all scrapers support language settings.
    // That way, we can have a language-dropdown menu wherever we want.

    ELCH_NODISCARD virtual Locale language() = 0;
    virtual void setLanguage(const Locale& value) = 0;

protected:
    ELCH_NODISCARD Settings& settings() { return m_settings; };

private:
    QString m_scraperId;
    Settings& m_settings;
};


class ScraperConfigurationStub : public ScraperConfiguration
{
public:
    ScraperConfigurationStub(const QString& scraperId, Settings& settings);
    ~ScraperConfigurationStub() override = default;

    void init() override;

    Locale language() override;
    void setLanguage(const Locale& value) override;
};

} // namespace mediaelch
