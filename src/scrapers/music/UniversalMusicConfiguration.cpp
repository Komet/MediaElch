#include "scrapers/music/UniversalMusicConfiguration.h"

#include "scrapers/tv_show/tmdb/TmdbTv.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/UniversalMusicScraper/Language");
static const Settings::Key KEY_SCRAPERS_PREFERRED(moduleName, "Scrapers/UniversalMusicScraper/Prefer");

} // namespace


namespace mediaelch {
namespace scraper {


UniversalMusicConfiguration::UniversalMusicConfiguration(Settings& settings) :
    ScraperConfiguration(QString(TmdbTv::ID), settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_LANGUAGE, this, [this]() { emit languageChanged(language()); });
    settings.onSettingChanged(
        KEY_SCRAPERS_PREFERRED, this, [this]() { emit preferredScraperChanged(preferredScraper()); });
}

UniversalMusicConfiguration::~UniversalMusicConfiguration() = default;

void UniversalMusicConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_LANGUAGE, defaultLocale().toString());
    settings().setDefaultValue(KEY_SCRAPERS_PREFERRED, QStringLiteral("theaudiodb"));
}

Locale UniversalMusicConfiguration::language()
{
    return settings().value(KEY_SCRAPERS_LANGUAGE).toString();
}

void UniversalMusicConfiguration::setLanguage(const Locale& value)
{
    settings().setValue(KEY_SCRAPERS_LANGUAGE, value.toString());
}

QString UniversalMusicConfiguration::preferredScraper()
{
    return settings().value(KEY_SCRAPERS_PREFERRED).toString();
}

void UniversalMusicConfiguration::setPreferredScraper(const QString& value)
{
    settings().setValue(KEY_SCRAPERS_PREFERRED, value);
}


mediaelch::Locale UniversalMusicConfiguration::defaultLocale()
{
    return {"en"};
}

QVector<Locale> UniversalMusicConfiguration::supportedLanguages()
{
    return QVector<Locale>({
        "cn",
        "nl",
        "en",
        "fr",
        "de",
        "he",
        "hu",
        "it",
        "ja",
        "no",
        "pl",
        "pt",
        "ru",
        "es",
        "sv", //
    });
}

} // namespace scraper
} // namespace mediaelch
