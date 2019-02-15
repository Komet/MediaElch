#pragma once

#include "settings/ScraperSettings.h"

#include <QMap>
#include <QString>

// TODO: Maybe use GoogleMock?

class MockScraperSettings : public ScraperSettings
{
public:
    explicit MockScraperSettings(ScraperInterface& scraper) : ScraperSettings(scraper) {}
    virtual ~MockScraperSettings() override = default;

    bool valueBool(const QString& key, bool default_value = false) const override;
    QString valueString(const QString& key, QString default_value = "") const override;

    void setBool(const QString&, bool) override;
    void setString(const QString&, const QString&) override;

    QMap<QString, bool> key_bool_map;
    QMap<QString, QString> key_string_map;
};
