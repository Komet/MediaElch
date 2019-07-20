#include "settings/ScraperSettings.h"

#include "scrapers/ScraperInterface.h"

static constexpr auto ScraperKeyLanguage = "Language";
static constexpr auto ScraperKeyGenre = "Genre";

ScraperSettings::ScraperSettings(ScraperInterface& scraper) : m_scraper(scraper)
{
}

ScraperSettings::~ScraperSettings() = default;

QString ScraperSettings::language(const QString& defaultValue) const
{
    return valueString(ScraperKeyLanguage, defaultValue);
}

QString ScraperSettings::genre(const QString& defaultValue) const
{
    return valueString(ScraperKeyGenre, defaultValue);
}

void ScraperSettings::setLanguage(const QString& value)
{
    setString(ScraperKeyLanguage, value);
}

void ScraperSettings::setGenre(const QString& value)
{
    setString(ScraperKeyGenre, value);
}

QString ScraperSettings::settings_key(const QString& key) const
{
    return QStringLiteral("Scrapers/%1/%2").arg(m_scraper.identifier(), key);
}

// ScraperSettingsQt ----------------------------------------------------------

bool ScraperSettingsQt::valueBool(const QString& key, bool default_value) const
{
    return m_settings.value(settings_key(key), default_value).toBool();
}

QString ScraperSettingsQt::valueString(const QString& key, QString default_value) const
{
    return m_settings.value(settings_key(key), default_value).toString();
}

void ScraperSettingsQt::setString(const QString& key, const QString& value)
{
    m_settings.setValue(settings_key(key), value);
}

void ScraperSettingsQt::setBool(const QString& key, bool value)
{
    m_settings.setValue(settings_key(key), value);
}
