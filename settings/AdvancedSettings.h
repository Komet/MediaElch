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

private:
    bool m_debugLog;
    QString m_logFile;
    QStringList m_sortTokens;
    QHash<QString, QString> m_genreMappings;

    void loadSettings();
    void reset();
    void loadLog(QXmlStreamReader &xml);
    void loadSortTokens(QXmlStreamReader &xml);
    void loadGenreMappings(QXmlStreamReader &xml);

};

#endif // ADVANCEDSETTINGS_H
