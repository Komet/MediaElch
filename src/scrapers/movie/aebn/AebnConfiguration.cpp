#include "scrapers/movie/aebn/AebnConfiguration.h"

#include "scrapers/movie/aebn/AEBN.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_LANGUAGE(moduleName, "Scrapers/aebn/Language");
static const Settings::Key KEY_SCRAPERS_GENRE(moduleName, "Scrapers/aebn/Genre");

} // namespace


namespace mediaelch {
namespace scraper {


AebnConfiguration::AebnConfiguration(Settings& settings, QObject* parent) :
    QObject(parent), ScraperConfiguration(QString(AEBN::ID), settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_LANGUAGE, this, [this]() { emit languageChanged(language()); });
    settings.onSettingChanged(KEY_SCRAPERS_GENRE, this, [this]() { emit genreIdChanged(genreId()); });
}

void AebnConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_LANGUAGE, defaultLocale().toString());
    settings().setDefaultValue(KEY_SCRAPERS_GENRE, defaultGenre());
}

Locale AebnConfiguration::language()
{
    return settings().value(KEY_SCRAPERS_LANGUAGE).toString();
}

void AebnConfiguration::setLanguage(const Locale& value)
{
    settings().setValue(KEY_SCRAPERS_LANGUAGE, value.toString());
}

QString AebnConfiguration::genreId()
{
    return settings().value(KEY_SCRAPERS_GENRE).toString();
}

void AebnConfiguration::setGenreId(const QString& value)
{
    settings().setValue(KEY_SCRAPERS_GENRE, value);
}

mediaelch::Locale AebnConfiguration::defaultLocale()
{
    return mediaelch::Locale{"en"};
}

QVector<Locale> AebnConfiguration::supportedLanguages()
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

QString AebnConfiguration::defaultGenre()
{
    return QStringLiteral("101"); // 101 => Straight
}

} // namespace scraper
} // namespace mediaelch
