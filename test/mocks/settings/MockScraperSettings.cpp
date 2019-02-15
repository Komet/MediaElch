#include "test/mocks/settings/MockScraperSettings.h"

void MockScraperSettings::setString(const QString&, const QString&)
{
    // no-op
}

void MockScraperSettings::setBool(const QString&, bool)
{
    // no-op
}

bool MockScraperSettings::valueBool(const QString& key, bool default_value) const
{
    return key_bool_map.value(key, default_value);
}

QString MockScraperSettings::valueString(const QString& key, QString default_value) const
{
    return key_string_map.value(key, default_value);
}
