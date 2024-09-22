#include "scrapers/tv_show/fernsehserien_de/FernsehserienDeConfiguration.h"

#include "scrapers/tv_show/fernsehserien_de/FernsehserienDe.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/fernsehserien.de/Language");

} // namespace


namespace mediaelch {
namespace scraper {


FernsehserienDeConfiguration::FernsehserienDeConfiguration(Settings& settings) :
    ScraperConfiguration(FernsehserienDe::ID, settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_LANGUAGE, this, [this]() { emit languageChanged(language()); });
}

void FernsehserienDeConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_LANGUAGE, defaultLocale().toString());
}

Locale FernsehserienDeConfiguration::language()
{
    return settings().value(KEY_SCRAPERS_LANGUAGE).toString();
}

void FernsehserienDeConfiguration::setLanguage(const Locale& value)
{
    settings().setValue(KEY_SCRAPERS_LANGUAGE, value.toString());
}

mediaelch::Locale FernsehserienDeConfiguration::defaultLocale()
{
    return mediaelch::Locale::NoLocale;
}

QVector<Locale> FernsehserienDeConfiguration::supportedLanguages()
{
    return QVector<Locale>({Locale::NoLocale});
}


} // namespace scraper
} // namespace mediaelch
