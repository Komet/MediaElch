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
    m_audioCodecMappings.clear();
    m_videoCodecMappings.clear();

    m_movieFilters << "*.mkv" << "*.avi" << "*.mpg" << "*.mpeg" << "*.mp4" << "*.m2ts" << "*.disc" << "*.m4v" << "*.strm"
                   << "*.dat" << "*.flv" << "*.vob" << "*.ts" << "*.iso" << "*.ogg" << "*.ogm" << "*.rmvb" << "*.img" << "*.wmv"
                   << "*.mov" << "*.divx" << "VIDEO_TS.IFO" << "index.bdmv" << "*.wtv";
    m_tvShowFilters << "*.mkv" << "*.avi" << "*.mpg" << "*.mpeg" << "*.mp4" << "*.m2ts" << "*.disc" << "*.m4v" << "*.strm"
                    << "*.dat" << "*.flv" << "*.vob" << "*.ts" << "*.rmvb" << "*.wmv" << "*.ogm" << "*.mov" << "*.divx"
                    << "*.wtv";
    m_concertFilters << "*.mkv" << "*.avi" << "*.mpg" << "*.mpeg" << "*.mp4" << "*.m2ts" << "*.disc" << "*.m4v" << "*.strm"
                     << "*.dat" << "*.flv" << "*.vob" << "*.ts" << "*.rmvb" << "*.img" << "*.wmv" << "*.ogm" << "*.mov" << "*.divx"
                     << "*.wtv";

    m_audioCodecMappings.insert("MPA1L3", "MP3");
    m_videoCodecMappings.insert("v_mpeg4/iso/avc", "h264");
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
        else if (xml.name() == "fileFilters")
            loadFilters(xml);
        else if (xml.name() == "audioCodecs")
            loadAudioCodecMappings(xml);
        else if (xml.name() == "videoCodecs")
            loadVideoCodecMappings(xml);
        else
            xml.skipCurrentElement();
    }

    qDebug() << "Advanced settings";
    qDebug() << "    debugLog           " << m_debugLog;
    qDebug() << "    logFile            " << m_logFile;
    qDebug() << "    sortTokens         " << m_sortTokens;
    qDebug() << "    genreMappings      " << m_genreMappings;
    qDebug() << "    movieFilters       " << m_movieFilters;
    qDebug() << "    concertFilters     " << m_concertFilters;
    qDebug() << "    tvShowFilters      " << m_tvShowFilters;
    qDebug() << "    audioCodecMappings " << m_audioCodecMappings;
    qDebug() << "    videoCodecMappings " << m_videoCodecMappings;
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
            xml.readElementText();
        } else {
            xml.skipCurrentElement();
        }
    }
}

void AdvancedSettings::loadFilters(QXmlStreamReader &xml)
{
    qDebug() << "loading filters";
    while (xml.readNextStartElement()) {
        if (xml.name() == "movies") {
            m_movieFilters.clear();
            foreach (const QString &filter, xml.readElementText().split(",", QString::SkipEmptyParts))
                m_movieFilters << filter.trimmed();
        } else if (xml.name() == "concerts") {
            m_concertFilters.clear();
            foreach (const QString &filter, xml.readElementText().split(",", QString::SkipEmptyParts))
                m_concertFilters << filter.trimmed();
        } else if (xml.name() == "tvShows") {
            m_tvShowFilters.clear();
            foreach (const QString &filter, xml.readElementText().split(",", QString::SkipEmptyParts))
                m_tvShowFilters << filter.trimmed();
        } else {
            xml.skipCurrentElement();
        }
    }
}

void AdvancedSettings::loadAudioCodecMappings(QXmlStreamReader &xml)
{
    while (xml.readNextStartElement()) {
        if (xml.name() == "map") {
            if (!xml.attributes().value("from").isEmpty())
                m_audioCodecMappings.insert(xml.attributes().value("from").toString(), xml.attributes().value("to").toString());
            xml.readElementText();
        } else {
            xml.skipCurrentElement();
        }
    }
}

void AdvancedSettings::loadVideoCodecMappings(QXmlStreamReader &xml)
{
    while (xml.readNextStartElement()) {
        if (xml.name() == "map") {
            if (!xml.attributes().value("from").isEmpty())
                m_videoCodecMappings.insert(xml.attributes().value("from").toString(), xml.attributes().value("to").toString());
            xml.readElementText();
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

QStringList AdvancedSettings::movieFilters() const
{
    return m_movieFilters;
}

QStringList AdvancedSettings::concertFilters() const
{
    return m_concertFilters;
}

QStringList AdvancedSettings::tvShowFilters() const
{
    return m_tvShowFilters;
}

QHash<QString, QString> AdvancedSettings::audioCodecMappings() const
{
    return m_audioCodecMappings;
}

QHash<QString, QString> AdvancedSettings::videoCodecMappings() const
{
    return m_videoCodecMappings;
}

