#include "scrapers/image/TheTvDbImagesConfiguration.h"

#include "scrapers/image/TheTvDbImages.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/images.thetvdb/Language");

} // namespace


namespace mediaelch {
namespace scraper {

TheTvDbImagesConfiguration::TheTvDbImagesConfiguration(Settings& settings) :
    ScraperConfiguration(QString(TheTvDbImages::ID), settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_LANGUAGE, this, [this]() { emit languageChanged(language()); });
}

void TheTvDbImagesConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_LANGUAGE, defaultLocale().toString());
}

mediaelch::Locale TheTvDbImagesConfiguration::defaultLocale()
{
    return {"en"};
}

QVector<Locale> TheTvDbImagesConfiguration::supportedLanguages()
{
    // Multiple languages, but no way to query for it and also no official list of languages.
    return QVector<Locale>({
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
        "tr",
    });
}

Locale TheTvDbImagesConfiguration::language()
{
    return settings().value(KEY_SCRAPERS_LANGUAGE).toString();
}

void TheTvDbImagesConfiguration::setLanguage(const Locale& value)
{
    settings().setValue(KEY_SCRAPERS_LANGUAGE, value.toString());
}


} // namespace scraper
} // namespace mediaelch
