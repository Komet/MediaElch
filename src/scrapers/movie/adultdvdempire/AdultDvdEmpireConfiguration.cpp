#include "scrapers/movie/adultdvdempire/AdultDvdEmpireConfiguration.h"

#include "scrapers/movie/adultdvdempire/AdultDvdEmpire.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/adult-dvd-empire/Language");
static const Settings::Key KEY_SCRAPERS_BACK_COVER_AS_FANART(moduleName,
    "Scrapers/adult-dvd-empire/StoreBackCoverAsFanart");

} // namespace


namespace mediaelch {
namespace scraper {

AdultDvdEmpireConfiguration::AdultDvdEmpireConfiguration(Settings& settings) :
    ScraperConfiguration(QString(AdultDvdEmpire::ID), settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_LANGUAGE, this, [this]() { emit languageChanged(language()); });
    settings.onSettingChanged(KEY_SCRAPERS_BACK_COVER_AS_FANART, this, [this]() {
        emit storeBackCoverAsFanartChanged(storeBackCoverAsFanart());
    });
}

void AdultDvdEmpireConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_LANGUAGE, defaultLocale().toString());
    settings().setDefaultValue(KEY_SCRAPERS_BACK_COVER_AS_FANART, true);
}

Locale AdultDvdEmpireConfiguration::language()
{
    return settings().value(KEY_SCRAPERS_LANGUAGE).toString();
}

void AdultDvdEmpireConfiguration::setLanguage(const Locale& value)
{
    settings().setValue(KEY_SCRAPERS_LANGUAGE, value.toString());
}

mediaelch::Locale AdultDvdEmpireConfiguration::defaultLocale()
{
    return mediaelch::Locale{"en"};
}

QVector<Locale> AdultDvdEmpireConfiguration::supportedLanguages()
{
    return QVector<Locale>({"en"});
}

bool AdultDvdEmpireConfiguration::storeBackCoverAsFanart()
{
    return settings().value(KEY_SCRAPERS_BACK_COVER_AS_FANART).toBool();
}

void AdultDvdEmpireConfiguration::setStoreBackCoverAsFanart(const bool& value)
{
    settings().setValue(KEY_SCRAPERS_BACK_COVER_AS_FANART, value);
}

} // namespace scraper
} // namespace mediaelch
