#include "scrapers/ScraperConfiguration.h"

#include <utility>

namespace mediaelch {

ScraperConfiguration::ScraperConfiguration(QString scraperId, Settings& settings) :
    m_scraperId(std::move(scraperId)), m_settings{settings}
{
}

ScraperConfigurationStub::ScraperConfigurationStub(const QString& scraperId, Settings& settings) :
    ScraperConfiguration(scraperId, settings)
{
}

void ScraperConfigurationStub::init()
{
    // no-op
}

Locale ScraperConfigurationStub::language()
{
    return Locale::NoLocale;
}

void ScraperConfigurationStub::setLanguage(const Locale& value)
{
    Q_UNUSED(value)
    // no-op
}


} // namespace mediaelch
