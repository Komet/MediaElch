#pragma once

#include "globals/Globals.h"

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QStringList>
#include <QXmlStreamReader>

class AdvancedSettings : public QObject
{
    Q_OBJECT
public:
    explicit AdvancedSettings(QObject* parent = nullptr);
    ~AdvancedSettings() override = default;

    bool debugLog() const;
    QString logFile() const;
    QLocale locale() const;
    QStringList sortTokens() const;
    QHash<QString, QString> genreMappings() const;
    QStringList movieFilters() const;
    QStringList concertFilters() const;
    QStringList tvShowFilters() const;
    QStringList subtitleFilters() const;
    QHash<QString, QString> audioCodecMappings() const;
    QHash<QString, QString> videoCodecMappings() const;
    QHash<QString, QString> certificationMappings() const;
    QHash<QString, QString> studioMappings() const;
    QHash<QString, QString> countryMappings() const;
    bool useFirstStudioOnly() const;
    bool forceCache() const;
    bool portableMode() const;
    int bookletCut() const;
    bool writeThumbUrlsToNfo() const;

private:
    bool m_debugLog;
    QString m_logFile;
    QLocale m_locale;
    QStringList m_sortTokens;
    QHash<QString, QString> m_genreMappings;
    QStringList m_movieFilters;
    QStringList m_concertFilters;
    QStringList m_tvShowFilters;
    QStringList m_subtitleFilters;
    QHash<QString, QString> m_audioCodecMappings;
    QHash<QString, QString> m_videoCodecMappings;
    QHash<QString, QString> m_certificationMappings;
    QHash<QString, QString> m_studioMappings;
    QHash<QString, QString> m_countryMappings;
    bool m_forceCache;
    bool m_portableMode;
    int m_bookletCut;
    bool m_writeThumbUrlsToNfo;
    bool m_useFirstStudioOnly;

    QByteArray getAdvancedSettingsXml() const;
    void loadSettings();
    void reset();
    void setLocale(QString locale);
    void loadLog(QXmlStreamReader& xml);
    void loadGui(QXmlStreamReader& xml);
    void loadSortTokens(QXmlStreamReader& xml);
    void loadFilters(QXmlStreamReader& xml);
    void loadMappings(QXmlStreamReader& xml, QHash<QString, QString>& map);
};
