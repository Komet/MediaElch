#include "scrapers/ScraperConfiguration.h"

namespace mediaelch {

ScraperConfiguration::ScraperConfiguration(QString scraperId, Settings& settings) :
    m_scraperId(scraperId), m_settings{settings}
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
