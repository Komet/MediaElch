#include "settings/ScraperSettings.h"

#include "scrapers/ScraperInterface.h"

#include <QMutexLocker>

static constexpr auto ScraperKeyLanguage = "Language";
static constexpr auto ScraperKeyGenre = "Genre";

ScraperSettings::ScraperSettings(QString scraperId) : m_scraperId(std::move(scraperId))
{
}

mediaelch::Locale ScraperSettings::language(const mediaelch::Locale& defaultValue)
{
    return valueString(ScraperKeyLanguage, defaultValue.toString());
}

QString ScraperSettings::genre(const QString& defaultValue)
{
    return valueString(ScraperKeyGenre, defaultValue);
}

void ScraperSettings::setLanguage(const mediaelch::Locale& value)
{
    setString(ScraperKeyLanguage, value.toString());
}

void ScraperSettings::setGenre(const QString& value)
{
    setString(ScraperKeyGenre, value);
}

QString ScraperSettings::settingsKey(const QString& key) const
{
    return QStringLiteral("Scrapers/%1/%2").arg(m_scraperId, key);
}

// ScraperSettingsQt ----------------------------------------------------------

bool ScraperSettingsQt::save()
{
    QMutexLocker locker(&m_savingMutex);
    auto i = m_cachedSettings.constBegin();
    while (i != m_cachedSettings.constEnd()) {
        m_settings.setValue(settingsKey(i.key()), i.value());
        ++i;
    }
    // All settings stored => clear cache.
    m_cachedSettings.clear();
    return true;
}

bool ScraperSettingsQt::valueBool(const QString& key, bool default_value)
{
    QMutexLocker locker(&m_savingMutex);
    if (!m_cachedSettings.contains(key)) {
        m_cachedSettings[key] = m_settings.value(settingsKey(key), default_value);
    }
    return m_cachedSettings[key].toBool();
}

QString ScraperSettingsQt::valueString(const QString& key, QString default_value)
{
    QMutexLocker locker(&m_savingMutex);
    if (!m_cachedSettings.contains(key)) {
        m_cachedSettings[key] = m_settings.value(settingsKey(key), default_value);
    }
    return m_cachedSettings[key].toString();
}

void ScraperSettingsQt::setString(const QString& key, const QString& value)
{
    QMutexLocker locker(&m_savingMutex);
    m_cachedSettings[key] = value;
}

void ScraperSettingsQt::setBool(const QString& key, bool value)
{
    QMutexLocker locker(&m_savingMutex);
    m_cachedSettings[key] = value;
}
