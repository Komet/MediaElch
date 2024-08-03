#include "scrapers/tv_show/thetvdb/TheTvDbConfiguration.h"

#include "scrapers/tv_show/thetvdb/TheTvDb.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/thetvdb/Language");

} // namespace


namespace mediaelch {
namespace scraper {


TheTvDbConfiguration::TheTvDbConfiguration(Settings& settings) : ScraperConfiguration(QString(TheTvDb::ID), settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_LANGUAGE, this, [this]() { emit languageChanged(language()); });
}

void TheTvDbConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_LANGUAGE, defaultLocale().toString());
}

Locale TheTvDbConfiguration::language()
{
    return settings().value(KEY_SCRAPERS_LANGUAGE).toString();
}

void TheTvDbConfiguration::setLanguage(const Locale& value)
{
    settings().setValue(KEY_SCRAPERS_LANGUAGE, value.toString());
}

mediaelch::Locale TheTvDbConfiguration::defaultLocale()
{
    return mediaelch::Locale("en");
}

QVector<Locale> TheTvDbConfiguration::supportedLanguages()
{
    return QVector<Locale>({//
        "bg",
        "zh",
        "hr",
        "cs",
        "da",
        "nl",
        "en",
        "fi",
        "fr",
        "de",
        "el",
        "he",
        "hu",
        "it",
        "ja",
        "ko",
        "no",
        "pl",
        "pt",
        "ru",
        "sl",
        "es",
        "sv",
        "tr"});
}


} // namespace scraper
} // namespace mediaelch
