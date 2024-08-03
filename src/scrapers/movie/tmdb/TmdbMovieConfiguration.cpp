#include "scrapers/movie/tmdb/TmdbMovieConfiguration.h"

#include "scrapers/movie/tmdb/TmdbMovie.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/TMDb/Language");

} // namespace


namespace mediaelch {
namespace scraper {


TmdbMovieConfiguration::TmdbMovieConfiguration(Settings& settings, QObject* parent) :
    QObject(parent), ScraperConfiguration(QString(TmdbMovie::ID), settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_LANGUAGE, this, [this]() { //
        emit languageChanged(language());
    });
}

void TmdbMovieConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_LANGUAGE, defaultLocale().toString());
}

Locale TmdbMovieConfiguration::language()
{
    return settings().value(KEY_SCRAPERS_LANGUAGE).toString();
}

void TmdbMovieConfiguration::setLanguage(const Locale& value)
{
    settings().setValue(KEY_SCRAPERS_LANGUAGE, value.toString());
}

mediaelch::Locale TmdbMovieConfiguration::defaultLocale()
{
    return mediaelch::Locale::English;
}

QVector<Locale> TmdbMovieConfiguration::supportedLanguages()
{
    return QVector<Locale>({
        //
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
