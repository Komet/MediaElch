#include "scrapers/tv_show/tvmaze/TvMazeConfiguration.h"

#include "scrapers/tv_show/tvmaze/TvMaze.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/tvmaze/Language");

} // namespace


namespace mediaelch {
namespace scraper {


TvMazeConfiguration::TvMazeConfiguration(Settings& settings) : ScraperConfiguration(QString(TvMaze::ID), settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_LANGUAGE, this, [this]() { emit languageChanged(language()); });
}

void TvMazeConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_LANGUAGE, defaultLocale().toString());
}

Locale TvMazeConfiguration::language()
{
    return settings().value(KEY_SCRAPERS_LANGUAGE).toString();
}

void TvMazeConfiguration::setLanguage(const Locale& value)
{
    settings().setValue(KEY_SCRAPERS_LANGUAGE, value.toString());
}

mediaelch::Locale TvMazeConfiguration::defaultLocale()
{
    return mediaelch::Locale::NoLocale;
}

QVector<Locale> TvMazeConfiguration::supportedLanguages()
{
    return QVector<Locale>({Locale::NoLocale});
}


} // namespace scraper
} // namespace mediaelch
