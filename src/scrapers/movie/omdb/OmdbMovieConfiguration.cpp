#include "scrapers/movie/omdb/OmdbMovieConfiguration.h"

#include "scrapers/movie/omdb/OmdbMovie.h"

namespace {

static constexpr char moduleName[] = "scrapers";
static const Settings::Key KEY_SCRAPERS_API_KEY(moduleName, "Scrapers/omdb/ApiKey");

} // namespace

namespace mediaelch {
namespace scraper {

OmdbMovieConfiguration::OmdbMovieConfiguration(Settings& settings, QObject* parent) :
    QObject(parent), ScraperConfiguration(QString(OmdbMovie::ID), settings)
{
    settings.onSettingChanged(KEY_SCRAPERS_API_KEY, this, [this]() { emit apiKeyChanged(apiKey()); });
}

void OmdbMovieConfiguration::init()
{
    settings().setDefaultValue(KEY_SCRAPERS_API_KEY, QStringLiteral(""));
}

QString OmdbMovieConfiguration::apiKey()
{
    return settings().value(KEY_SCRAPERS_API_KEY).toString();
}

Locale OmdbMovieConfiguration::language()
{
    return Locale::English;
}

void OmdbMovieConfiguration::setLanguage(const Locale& value)
{
    // OMDb only supports English — ignore language changes
    Q_UNUSED(value);
}

void OmdbMovieConfiguration::setApiKey(const QString& value)
{
    settings().setValue(KEY_SCRAPERS_API_KEY, value);
}

} // namespace scraper
} // namespace mediaelch
