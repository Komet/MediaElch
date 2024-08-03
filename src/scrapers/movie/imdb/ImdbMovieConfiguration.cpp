#include "scrapers/movie/imdb/ImdbMovieConfiguration.h"

#include "scrapers/movie/imdb/ImdbMovie.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/IMDb/Language");
static const Settings::Key KEY_SCRAPERS_LOAD_ALL_TAGS(moduleName, "Scrapers/IMDb/LoadAllTags");

} // namespace


namespace mediaelch {
namespace scraper {

ImdbMovieConfiguration::ImdbMovieConfiguration(Settings& settings) :
    ScraperConfiguration(QString(ImdbMovie::ID), settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_LANGUAGE, this, [this]() { emit languageChanged(language()); });
    settings.onSettingChanged(
        KEY_SCRAPERS_LOAD_ALL_TAGS, this, [this]() { emit loadAllTagsChanged(shouldLoadAllTags()); });
}

void ImdbMovieConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_LANGUAGE, defaultLocale().toString());
    settings().setDefaultValue(KEY_SCRAPERS_LOAD_ALL_TAGS, false);
}

Locale ImdbMovieConfiguration::language()
{
    return settings().value(KEY_SCRAPERS_LANGUAGE).toString();
}

void ImdbMovieConfiguration::setLanguage(const Locale& value)
{
    settings().setValue(KEY_SCRAPERS_LANGUAGE, value.toString());
}

mediaelch::Locale ImdbMovieConfiguration::defaultLocale()
{
    return mediaelch::Locale{"en"};
}

QVector<Locale> ImdbMovieConfiguration::supportedLanguages()
{
    return QVector<Locale>({"en"});
}

bool ImdbMovieConfiguration::shouldLoadAllTags()
{
    return settings().value(KEY_SCRAPERS_LOAD_ALL_TAGS).toBool();
}

void ImdbMovieConfiguration::setLoadAllTags(const bool& value)
{
    settings().setValue(KEY_SCRAPERS_LOAD_ALL_TAGS, value);
}


} // namespace scraper
} // namespace mediaelch
