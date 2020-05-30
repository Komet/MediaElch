#include "test/mocks/settings/MockScraperSettings.h"

bool MockScraperSettings::save()
{
    return true;
}

void MockScraperSettings::setString(const QString& /*key*/, const QString& /*value*/)
{
    // no-op
}

void MockScraperSettings::setBool(const QString& /*key*/, bool /*value*/)
{
    // no-op
}

bool MockScraperSettings::valueBool(const QString& key, bool default_value)
{
    return key_bool_map.value(key, default_value);
}

QString MockScraperSettings::valueString(const QString& key, QString default_value)
{
    return key_string_map.value(key, default_value);
}
