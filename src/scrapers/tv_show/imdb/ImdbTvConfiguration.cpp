#include "scrapers/tv_show/imdb/ImdbTvConfiguration.h"

#include "scrapers/tv_show/imdb/ImdbTv.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/imdbtv/Language");

} // namespace


namespace mediaelch {
namespace scraper {


ImdbTvConfiguration::ImdbTvConfiguration(Settings& settings) : ScraperConfiguration(ImdbTv::ID, settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_LANGUAGE, this, [this]() { emit languageChanged(language()); });
}

void ImdbTvConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_LANGUAGE, defaultLocale().toString());
}

Locale ImdbTvConfiguration::language()
{
    return settings().value(KEY_SCRAPERS_LANGUAGE).toString();
}

void ImdbTvConfiguration::setLanguage(const Locale& value)
{
    settings().setValue(KEY_SCRAPERS_LANGUAGE, value.toString());
}

mediaelch::Locale ImdbTvConfiguration::defaultLocale()
{
    return mediaelch::Locale{"en"};
}

QVector<Locale> ImdbTvConfiguration::supportedLanguages()
{
    // With the GraphQL API migration, localization is supported via AKAs and
    // country-specific certificates. Plots remain English-only.
    return QVector<Locale>({
        "en",
        "de",
        "fr",
        "es",
        "it",
        "pt",
        "ja",
        "ko",
        "zh",
        "ru",
        "nl",
        "pl",
        "sv",
        "da",
        "fi",
        "no",
    });
}


} // namespace scraper
} // namespace mediaelch
