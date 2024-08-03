#include "scrapers/tv_show/imdb/ImdbTvConfiguration.h"

#include "scrapers/tv_show/imdb/ImdbTv.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/imdbtv/Language");

} // namespace


namespace mediaelch {
namespace scraper {


ImdbTvConfiguration::ImdbTvConfiguration(Settings& settings) : ScraperConfiguration(QString(ImdbTv::ID), settings)
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
    return mediaelch::Locale::NoLocale;
}

QVector<Locale> ImdbTvConfiguration::supportedLanguages()
{
    return QVector<Locale>({Locale::NoLocale});
}


} // namespace scraper
} // namespace mediaelch
