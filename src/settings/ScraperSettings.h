#pragma once

#include "data/Locale.h"

#include <QDebug>
#include <QMutex>
#include <QSettings>
#include <QString>

class ScraperInterface;

/// \brief Base class for scraper settings.
/// \details This class is used to load and store scraper settings on the
///          user's system.  It provides methods for simple access to
///          often-used settings like language. By using these wrapper
///          methods, typos are avoided.
///          This class caches changes and only stores them on disk when
///          \p save is called.  Implementation is defined by its sub-
///          classes.
/// \see ScraperSettingsQt
class ScraperSettings
{
public:
    explicit ScraperSettings(QString scraperId);
    virtual ~ScraperSettings() = default;

    /// \brief Save the cached settings onto disk.
    virtual bool save() = 0;

    // Convenience methods
    mediaelch::Locale language(const mediaelch::Locale& defaultValue);
    QString genre(const QString& defaultValue);
    void setLanguage(const mediaelch::Locale& value);
    void setGenre(const QString& value);

    virtual bool valueBool(const QString& key, bool default_value) = 0;
    virtual QString valueString(const QString& key, QString default_value) = 0;

    virtual void setString(const QString& key, const QString& value) = 0;
    virtual void setBool(const QString& key, bool value) = 0;

protected:
    QString settingsKey(const QString& key) const;

private:
    QString m_scraperId;
};


/// \brief Scraper settings class which uses QSettings to store settings.
/// \details This class is used to load and store scraper settings on the
///          user's system using Qt's settings functionality.
class ScraperSettingsQt : public ScraperSettings
{
public:
    explicit ScraperSettingsQt(QString scraperId, QSettings& settings) :
        ScraperSettings(std::move(scraperId)), m_settings{settings}
    {
    }
    ~ScraperSettingsQt() override = default;

    bool save() override;

    bool valueBool(const QString& key, bool default_value = false) override;
    QString valueString(const QString& key, QString default_value = "") override;

    void setString(const QString& key, const QString& value) override;
    void setBool(const QString& key, bool value) override;

private:
    QSettings& m_settings;
    QMap<QString, QVariant> m_cachedSettings;
    QMutex m_savingMutex;
};
