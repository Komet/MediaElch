#include "scrapers/image/FanartTvConfiguration.h"

#include "scrapers/image/FanartTv.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/images.fanarttv/Language");
static const Settings::Key KEY_SCRAPERS_DISC_TYPE(moduleName, "Scrapers/images.fanarttv/DiscType");
static const Settings::Key KEY_SCRAPERS_PERSONAL_API_KEY(moduleName, "Scrapers/images.fanarttv/PersonalApiKey");

} // namespace


namespace mediaelch {
namespace scraper {


FanartTvConfiguration::FanartTvConfiguration(Settings& settings) : ScraperConfiguration(QString(FanartTv::ID), settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_LANGUAGE, this, [this]() { emit languageChanged(language()); });
    settings.onSettingChanged(
        KEY_SCRAPERS_DISC_TYPE, this, [this]() { emit preferredDiscTypeChanged(preferredDiscType()); });
    settings.onSettingChanged(
        KEY_SCRAPERS_PERSONAL_API_KEY, this, [this]() { emit personalApiKeyChanged(personalApiKey()); });
}

void FanartTvConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_LANGUAGE, defaultLocale().toString());
    settings().setDefaultValue(KEY_SCRAPERS_DISC_TYPE, QStringLiteral("BluRay"));
    settings().setDefaultValue(KEY_SCRAPERS_PERSONAL_API_KEY, QStringLiteral(""));
}

mediaelch::Locale FanartTvConfiguration::defaultLocale()
{
    return {"en"};
}

QVector<Locale> FanartTvConfiguration::supportedLanguages()
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

Locale FanartTvConfiguration::language()
{
    return settings().value(KEY_SCRAPERS_LANGUAGE).toString();
}

void FanartTvConfiguration::setLanguage(const Locale& value)
{
    settings().setValue(KEY_SCRAPERS_LANGUAGE, value.toString());
}

QString FanartTvConfiguration::preferredDiscType()
{
    return settings().value(KEY_SCRAPERS_DISC_TYPE).toString();
}

void FanartTvConfiguration::setPreferredDiscType(const QString& value)
{
    settings().setValue(KEY_SCRAPERS_DISC_TYPE, value);
}

QString FanartTvConfiguration::personalApiKey()
{
    return settings().value(KEY_SCRAPERS_PERSONAL_API_KEY).toString();
}

void FanartTvConfiguration::setPersonalApiKey(const QString& value)
{
    settings().setValue(KEY_SCRAPERS_PERSONAL_API_KEY, value);
}


} // namespace scraper
} // namespace mediaelch
