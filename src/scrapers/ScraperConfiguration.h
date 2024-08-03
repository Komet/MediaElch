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

protected:
    ELCH_NODISCARD Settings& settings() { return m_settings; };

private:
    QString m_scraperId;
    Settings& m_settings;
};


} // namespace mediaelch
