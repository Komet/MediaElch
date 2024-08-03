#include "scrapers/concert/tmdb/TmdbConcertConfiguration.h"

#include "scrapers/concert/tmdb/TmdbConcert.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/TmdbConcert/Language");

} // namespace


namespace mediaelch {
namespace scraper {


TmdbConcertConfiguration::TmdbConcertConfiguration(Settings& settings) :
    ScraperConfiguration(QString(TmdbConcert::ID), settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_LANGUAGE, this, [this]() { emit languageChanged(language()); });
}

void TmdbConcertConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_LANGUAGE, defaultLocale().toString());
}

Locale TmdbConcertConfiguration::language()
{
    return settings().value(KEY_SCRAPERS_LANGUAGE).toString();
}

void TmdbConcertConfiguration::setLanguage(const Locale& value)
{
    settings().setValue(KEY_SCRAPERS_LANGUAGE, value.toString());
}

mediaelch::Locale TmdbConcertConfiguration::defaultLocale()
{
    return mediaelch::Locale::English;
}

QVector<Locale> TmdbConcertConfiguration::supportedLanguages()
{
    // For officially supported languages, see:
    // https://developers.themoviedb.org/3/configuration/get-primary-translations
    return QVector<Locale>({
        "ar-AE",
        "ar-SA",
        "be-BY",
        "bg-BG",
        "bn-BD",
        "ca-ES",
        "ch-GU",
        "cn-CN",
        "cs-CZ",
        "da-DK",
        "de-DE",
        "de-AT",
        "de-CH",
        "el-GR",
        "en-AU",
        "en-CA",
        "en-GB",
        "en-NZ",
        "en-US",
        "eo-EO",
        "es-ES",
        "es-MX",
        "et-EE",
        "eu-ES",
        "fa-IR",
        "fi-FI",
        "fr-CA",
        "fr-FR",
        "gl-ES",
        "he-IL",
        "hi-IN",
        "hu-HU",
        "id-ID",
        "it-IT",
        "ja-JP",
        "ka-GE",
        "kk-KZ",
        "kn-IN",
        "ko-KR",
        "lt-LT",
        "lv-LV",
        "ml-IN",
        "ms-MY",
        "ms-SG",
        "nb-NO",
        "nl-NL",
        "no-NO",
        "pl-PL",
        "pt-BR",
        "pt-PT",
        "ro-RO",
        "ru-RU",
        "si-LK",
        "sk-SK",
        "sl-SI",
        "sq-AL",
        "sr-RS",
        "sv-SE",
        "ta-IN",
        "te-IN",
        "th-TH",
        "tl-PH",
        "tr-TR",
        "uk-UA",
        "vi-VN",
        "zh-CN",
        "zh-HK",
        "zh-TW",
        "zu-ZA", //
    });
}


} // namespace scraper
} // namespace mediaelch
