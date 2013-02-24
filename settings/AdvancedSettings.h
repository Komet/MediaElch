#ifndef ADVANCEDSETTINGS_H
#define ADVANCEDSETTINGS_H

#include <QHash>
#include <QObject>
#include <QStringList>
#include <QXmlStreamReader>
#include "globals/Globals.h"

class AdvancedSettings : public QObject
{
    Q_OBJECT
public:
    explicit AdvancedSettings(QObject *parent = 0);
    ~AdvancedSettings();

    bool debugLog() const;
    QString logFile() const;
    QStringList sortTokens() const;
    QHash<QString, QString> genreMappings() const;
    QStringList movieFilters() const;
    QStringList concertFilters() const;
    QStringList tvShowFilters() const;
    QHash<QString, QString> audioCodecMappings() const;
    QHash<QString, QString> videoCodecMappings() const;

private:
    bool m_debugLog;
    QString m_logFile;
    QStringList m_sortTokens;
    QHash<QString, QString> m_genreMappings;
    QStringList m_movieFilters;
    QStringList m_concertFilters;
    QStringList m_tvShowFilters;
    QHash<QString, QString> m_audioCodecMappings;
    QHash<QString, QString> m_videoCodecMappings;

    void loadSettings();
    void reset();
    void loadLog(QXmlStreamReader &xml);
    void loadSortTokens(QXmlStreamReader &xml);
    void loadGenreMappings(QXmlStreamReader &xml);
    void loadFilters(QXmlStreamReader &xml);
    void loadAudioCodecMappings(QXmlStreamReader &xml);
    void loadVideoCodecMappings(QXmlStreamReader &xml);
};

#endif // ADVANCEDSETTINGS_H
