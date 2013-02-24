#include "AdvancedSettings.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFile>

AdvancedSettings::AdvancedSettings(QObject *parent) :
    QObject(parent)
{
    loadSettings();
}

AdvancedSettings::~AdvancedSettings()
{
}

void AdvancedSettings::reset()
{
    m_debugLog = false;
    m_logFile = "";
    m_sortTokens = QStringList() << "Der" << "Die" << "Das" << "The" << "Le" << "La" << "Les" << "Un" << "Une" << "Des";
    m_genreMappings.clear();
}

void AdvancedSettings::loadSettings()
{
    qDebug() << "Loading advanced settings";
    reset();

    QXmlStreamReader xml;
    QFile file(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/advancedsettings.xml");
    if (!file.exists())
        return;

    if (!file.open(QIODevice::ReadOnly))
        return;
    xml.addData(file.readAll());
    file.close();

    if (!xml.readNextStartElement() || xml.name().toString() != "advancedsettings")
        return;

    while (xml.readNextStartElement()) {
        if (xml.name() == "log")
            loadLog(xml);
        else if (xml.name() == "sorttokens")
            loadSortTokens(xml);
        else if (xml.name() == "genres")
            loadGenreMappings(xml);
        else
            xml.skipCurrentElement();
    }

    qDebug() << "Advanced settings";
    qDebug() << "    debugLog      " << m_debugLog;
    qDebug() << "    logFile       " << m_logFile;
    qDebug() << "    sortTokens    " << m_sortTokens;
    qDebug() << "    genreMappings " << m_genreMappings;
}

void AdvancedSettings::loadLog(QXmlStreamReader &xml)
{
    while (xml.readNextStartElement()) {
        if (xml.name() == "debug")
            m_debugLog = (xml.readElementText() == "true");
        else if (xml.name() == "file")
            m_logFile = xml.readElementText();
        else
            xml.skipCurrentElement();
    }
}

void AdvancedSettings::loadSortTokens(QXmlStreamReader &xml)
{
    m_sortTokens.clear();
    while (xml.readNextStartElement()) {
        if (xml.name() == "token")
            m_sortTokens << xml.readElementText();
        else
            xml.skipCurrentElement();
    }
}

void AdvancedSettings::loadGenreMappings(QXmlStreamReader &xml)
{
    while (xml.readNextStartElement()) {
        if (xml.name() == "map") {
            if (!xml.attributes().value("from").isEmpty())
                m_genreMappings.insert(xml.attributes().value("from").toString(), xml.attributes().value("to").toString());
        } else {
            xml.skipCurrentElement();
        }
    }
}

bool AdvancedSettings::debugLog() const
{
    return m_debugLog;
}

QString AdvancedSettings::logFile() const
{
    return m_logFile;
}

QStringList AdvancedSettings::sortTokens() const
{
    return m_sortTokens;
}

QHash<QString, QString> AdvancedSettings::genreMappings() const
{
    return m_genreMappings;
}
