#include "scrapers/ScraperConfiguration.h"

namespace mediaelch {

ScraperConfiguration::ScraperConfiguration(QString scraperId, Settings& settings) :
    m_scraperId(scraperId), m_settings{settings}
{
}


} // namespace mediaelch
