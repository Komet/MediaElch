#include "test/mocks/settings/SettingsMock.h"

Settings::Value SettingsMock::value(const Settings::Key& key)
{
    return key_string_map[key];
}

void SettingsMock::setValue(const Settings::Key& key, const Settings::Value& value)
{
    key_string_map[key] = value;
}

void SettingsMock::setDefaultValue(const Settings::Key& key, const Settings::Value& value)
{
    key_string_map[key] = value;
}
