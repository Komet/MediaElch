#include "scrapers/tv_show/omdb/OmdbTvConfiguration.h"

#include "scrapers/tv_show/omdb/OmdbTv.h"

namespace {

static constexpr char moduleName[] = "scrapers";
// Share the same settings key as the movie scraper so users only enter the key once
static const Settings::Key KEY_SCRAPERS_API_KEY(moduleName, "Scrapers/omdb/ApiKey");

} // namespace

namespace mediaelch {
namespace scraper {

OmdbTvConfiguration::OmdbTvConfiguration(Settings& settings, QObject* parent) :
    QObject(parent), ScraperConfiguration(QString(OmdbTv::ID), settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_API_KEY, this, [this]() { emit apiKeyChanged(apiKey()); });
}

void OmdbTvConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_API_KEY, QStringLiteral(""));
}

Locale OmdbTvConfiguration::language()
{
    return Locale::English;
}

void OmdbTvConfiguration::setLanguage(const Locale& value)
{
    Q_UNUSED(value);
}

QString OmdbTvConfiguration::apiKey()
{
    return settings().value(KEY_SCRAPERS_API_KEY).toString();
}

void OmdbTvConfiguration::setApiKey(const QString& value)
{
    settings().setValue(KEY_SCRAPERS_API_KEY, value);
}

} // namespace scraper
} // namespace mediaelch
