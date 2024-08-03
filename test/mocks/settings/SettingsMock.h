#pragma once

#include "settings/Settings.h"

#include <QMap>
#include <QString>
#include <QVariant>

class SettingsMock : public Settings
{
public:
    explicit SettingsMock() : Settings(nullptr) {}
    virtual ~SettingsMock() override = default;

    ELCH_NODISCARD Value value(const Key& key) override;
    void setValue(const Key& key, const Value& value) override;
    void setDefaultValue(const Key& key, const Value& value) override;

    QMap<Settings::Key, Settings::Value> key_string_map;
};
