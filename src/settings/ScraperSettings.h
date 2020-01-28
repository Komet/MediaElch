#pragma once

#include <QDebug>
#include <QSettings>
#include <QString>

class ScraperInterface;

/**
 * @brief Base class for scraper settings.
 * @see ScraperSettingsQt
 */
class ScraperSettings
{
public:
    explicit ScraperSettings(ScraperInterface& scraper);
    virtual ~ScraperSettings();

    // Convenience functions
    QString language(const QString& defaultValue = "en") const;
    QString genre(const QString& defaultValue) const;
    void setLanguage(const QString& value);
    void setGenre(const QString& value);

    virtual bool valueBool(const QString& key, bool default_value = false) const = 0;
    virtual QString valueString(const QString& key, QString default_value = "") const = 0;

    virtual void setString(const QString& key, const QString& value) = 0;
    virtual void setBool(const QString& key, bool value) = 0;

protected:
    QString settings_key(const QString& key) const;

private:
    ScraperInterface& m_scraper;
};

/**
 * @brief Scraper settings class which uses QSettings to store settings.
 */
class ScraperSettingsQt : public ScraperSettings
{
public:
    explicit ScraperSettingsQt(ScraperInterface& scraper, QSettings& settings) :
        ScraperSettings(scraper), m_settings{settings}
    {
    }
    virtual ~ScraperSettingsQt() override = default;

    bool valueBool(const QString& key, bool default_value = false) const override;
    QString valueString(const QString& key, QString default_value = "") const override;

    void setString(const QString& key, const QString& value) override;
    void setBool(const QString& key, bool value) override;

private:
    QSettings& m_settings;
};
