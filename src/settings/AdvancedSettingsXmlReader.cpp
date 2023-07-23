#include "settings/AdvancedSettingsXmlReader.h"

#include "log/Log.h"
#include "settings/Settings.h"

#include <QDesktopServices>
#include <QFile>
#include <QStandardPaths>

/// translations for parser errors / messages
const QMap<AdvancedSettingsXmlReader::ParseErrorType, QString> AdvancedSettingsXmlReader::errors = {
    {ParseErrorType::FileNotFound, QObject::tr("File not found:")},
    {ParseErrorType::FileIsReadOnly, QObject::tr("Cannot open advancedsettings.xml in read-only mode.")},
    {ParseErrorType::NoMainTag, QObject::tr("Couldn't find an <advancedsettings> tag!")},
    {ParseErrorType::UnsupportedTag, QObject::tr("Found unsupported xml tag:")},
    {ParseErrorType::InvalidValue, QObject::tr("Invalid value for xml tag:")}};

QPair<AdvancedSettings, AdvancedSettingsXmlReader::ValidationMessages> AdvancedSettingsXmlReader::loadFromDefaultPath()
{
    AdvancedSettingsXmlReader reader;
    mediaelch::FilePath path = reader.getFilePath();

    qCDebug(generic) << "Loading advanced settings from:" << path;

    // Only parse the xml if the file exists. Otherwise, use defaults.
    if (QFile(path.toString()).exists()) {
        QString xml = reader.getAdvancedSettingsXml(path);
        if (!xml.isEmpty()) {
            reader.parseSettings(xml);
        }
        reader.m_settings.m_userDefined = true;
    } else {
        qCWarning(generic) << "[AdvancedSettings] advancedsettings.xml not found at " << path.toString();
        reader.m_settings.m_userDefined = false;
        reader.addWarning("advancedsettings.xml", ParseErrorType::FileNotFound);
    }

    return {reader.m_settings, reader.m_messages};
}

QPair<AdvancedSettings, AdvancedSettingsXmlReader::ValidationMessages> AdvancedSettingsXmlReader::loadFromXml(
    QString xml)
{
    AdvancedSettingsXmlReader reader;
    reader.parseSettings(xml);
    return {reader.m_settings, reader.m_messages};
}

mediaelch::FilePath AdvancedSettingsXmlReader::getFilePath()
{
    QFile file(Settings::applicationDir() + "/advancedsettings.xml");
    if (!file.exists()) {
        file.setFileName(
            QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/advancedsettings.xml");
    }
    return mediaelch::FilePath(file.fileName());
}

QByteArray AdvancedSettingsXmlReader::getAdvancedSettingsXml(const mediaelch::FilePath& filepath)
{
    QByteArray xmlStr;
    QFile file{filepath.toString()};

    if (!file.exists()) {
        return xmlStr;
    }

    if (file.open(QIODevice::ReadOnly)) {
        xmlStr = file.readAll();
        file.close();
    } else {
        qCWarning(generic) << "[AdvancedSettings] Cannot open advancedsettings.xml in read-only mode";
        addError("advancedsettings.xml", ParseErrorType::FileIsReadOnly);
    }

    return xmlStr;
}

void AdvancedSettingsXmlReader::parseSettings(const QString& xmlSource)
{
    m_xml.clear();
    m_xml.addData(xmlSource);

    if (!m_xml.readNextStartElement() || m_xml.name().toString() != "advancedsettings") {
        qCWarning(generic) << "[AdvancedSettings] Couldn't find an <advancedsettings> tag! Error reported:"
                           << m_xml.errorString();
        addError("advancedsettings", ParseErrorType::NoMainTag);
        return;
    }

    while (m_xml.readNextStartElement()) {
        if (m_xml.hasError()) {
            qCWarning(generic) << "[AdvancedSettings] Error while parsing:" << m_xml.errorString();
            return;
        }
        if (m_xml.name() == QLatin1String("log")) {
            loadLog();

        } else if (m_xml.name() == QLatin1String("locale")) {
            m_settings.setLocale(m_xml.readElementText());

        } else if (m_xml.name() == QLatin1String("portableMode")) {
            expectBool(m_settings.m_portableMode);

        } else if (m_xml.name() == QLatin1String("gui")) {
            loadGui();

        } else if (m_xml.name() == QLatin1String("writeThumbUrlsToNfo")) {
            expectBool(m_settings.m_writeThumbUrlsToNfo);

        } else if (m_xml.name() == QLatin1String("episodeThumb")) {
            while (m_xml.readNextStartElement()) {
                if (m_xml.name() == QLatin1String("width")) {
                    // must be at least 100 pixel wide and should be small than ~4 Mio.
                    const auto inRange = [](int width) { return width >= 100 && width <= (2 << 22); };
                    expectIntChecked(m_settings.m_episodeThumbnailDimensions.width, inRange);

                } else if (m_xml.name() == QLatin1String("height")) {
                    // must be at least 100 pixel wide and should be small than ~4 Mio.
                    const auto inRange = [](int height) { return height >= 100 && height <= (2 << 22); };
                    expectIntChecked(m_settings.m_episodeThumbnailDimensions.height, inRange);

                } else {
                    skipUnsupportedTag();
                }
            }

        } else if (m_xml.name() == QLatin1String("bookletCut")) {
            expectInt(m_settings.m_bookletCut);

        } else if (m_xml.name() == QLatin1String("sorttokens")) {
            loadSortTokens();

        } else if (m_xml.name() == QLatin1String("genres")) {
            loadMappings(m_settings.m_genreMappings);

        } else if (m_xml.name() == QLatin1String("audioCodecs")) {
            loadMappings(m_settings.m_audioCodecMappings);

        } else if (m_xml.name() == QLatin1String("videoCodecs")) {
            loadMappings(m_settings.m_videoCodecMappings);

        } else if (m_xml.name() == QLatin1String("certifications")) {
            loadMappings(m_settings.m_certificationMappings);

        } else if (m_xml.name() == QLatin1String("studios")) {
            if (m_xml.attributes().hasAttribute("useFirstStudioOnly")) {
                const auto firstStudioOnly = m_xml.attributes().value("useFirstStudioOnly").trimmed();
                m_settings.m_useFirstStudioOnly = (firstStudioOnly == QLatin1String("true"));
            }
            loadMappings(m_settings.m_studioMappings);

        } else if (m_xml.name() == QLatin1String("countries")) {
            loadMappings(m_settings.m_countryMappings);

        } else if (m_xml.name() == QLatin1String("fileFilters")) {
            loadFilters();

        } else if (m_xml.name() == QLatin1String("exclude")) {
            loadExcludePatterns();

        } else {
            skipUnsupportedTag();
        }
    }

    // Proper setup of file filters: m_fileExcludes / m_folderExcludes may be
    // used generically, but in the end, we have different file filters.
    m_settings.m_movieFilters.fileExcludes = m_settings.m_fileExcludes;
    m_settings.m_concertFilters.fileExcludes = m_settings.m_fileExcludes;
    m_settings.m_tvShowFilters.fileExcludes = m_settings.m_fileExcludes;
    m_settings.m_musicFilters.fileExcludes = m_settings.m_fileExcludes;
    m_settings.m_subtitleFilters.fileExcludes = m_settings.m_fileExcludes;

    m_settings.m_movieFilters.folderExcludes = m_settings.m_folderExcludes;
    m_settings.m_concertFilters.folderExcludes = m_settings.m_folderExcludes;
    m_settings.m_tvShowFilters.folderExcludes = m_settings.m_folderExcludes;
    m_settings.m_musicFilters.folderExcludes = m_settings.m_folderExcludes;
    m_settings.m_subtitleFilters.folderExcludes = m_settings.m_folderExcludes;
}

void AdvancedSettingsXmlReader::loadLog()
{
    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("debug")) {
            expectBool(m_settings.m_debugLog);
        } else if (m_xml.name() == QLatin1String("file")) {
            m_settings.m_logFile = m_xml.readElementText().trimmed();
        } else {
            skipUnsupportedTag();
        }
    }
}

void AdvancedSettingsXmlReader::loadGui()
{
    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("forceCache")) {
            expectBool(m_settings.m_forceCache);
        } else if (m_xml.name() == QLatin1String("stylesheet")) {
            m_settings.m_customStylesheet = m_xml.readElementText().trimmed();
        } else {
            skipUnsupportedTag();
        }
    }
}

void AdvancedSettingsXmlReader::loadSortTokens()
{
    QStringList tokens;
    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("token")) {
            tokens << m_xml.readElementText().trimmed();
        } else {
            skipUnsupportedTag();
        }
    }

    if (!tokens.isEmpty()) {
        m_settings.m_sortTokens = tokens;
    }
}

void AdvancedSettingsXmlReader::loadFilters()
{
    // The current XML element's text is split by "," and all items
    // are appended to a cleared "list".
    const auto appendNextFiltersToList = [&](mediaelch::FileFilter& list) {
        QStringList newFilters;
        const auto filters = m_xml.readElementText().split(",", ElchSplitBehavior::SkipEmptyParts);
        for (const QString& filter : filters) {
            newFilters << filter.trimmed();
        }
        list = mediaelch::FileFilter(newFilters);
    };

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("movies")) {
            appendNextFiltersToList(m_settings.m_movieFilters);

        } else if (m_xml.name() == QLatin1String("concerts")) {
            appendNextFiltersToList(m_settings.m_concertFilters);

        } else if (m_xml.name() == QLatin1String("tvShows")) {
            appendNextFiltersToList(m_settings.m_tvShowFilters);

        } else if (m_xml.name() == QLatin1String("subtitle")) {
            appendNextFiltersToList(m_settings.m_subtitleFilters);

        } else {
            skipUnsupportedTag();
        }
    }
}

/// Load the next mappings inside <map> tags into "mappings".
/// \param mappings QHash table that will be cleared and to which the mappings will be appended.
void AdvancedSettingsXmlReader::loadMappings(QHash<QString, QString>& mappings)
{
    mappings.clear();
    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("map") && m_xml.attributes().hasAttribute("from")) {
            const auto from = m_xml.attributes().value("from").trimmed();
            const auto to = m_xml.attributes().value("to").trimmed();
            if (!from.isEmpty() && !to.isEmpty()) {
                mappings.insert(from.toString(), to.toString());
            }
            m_xml.readElementText();
        } else {
            skipUnsupportedTag();
        }
    }
}

void AdvancedSettingsXmlReader::loadExcludePatterns()
{
    QVector<QRegularExpression> folderExcludes;
    QVector<QRegularExpression> fileExcludes;

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("pattern")) {
            QString applyTo = m_xml.attributes().value("applyTo").toString();
            // TODO: Sanitize input string
            QRegularExpression pattern(m_xml.readElementText().trimmed());
            if (!pattern.isValid()) {
                qCCritical(generic) << "[AdvancedSettings] Invalid regular expression! Message:"
                                    << pattern.errorString();
                addError("pattern", ParseErrorType::InvalidValue);
                return;
            }
            pattern.optimize();

            if (applyTo == "filename") {
                fileExcludes << pattern;
            } else if (applyTo == "folders") {
                folderExcludes << pattern;
            } else {
                qCWarning(generic) << "[AdvancedSettings] Unknown value for 'applyTo' attribute of <pattern> element at"
                                   << currentLocation();
                addError("pattern", ParseErrorType::InvalidAttributeValue);
            }

        } else {
            skipUnsupportedTag();
        }
    }

    if (!fileExcludes.isEmpty()) {
        m_settings.m_fileExcludes = fileExcludes;
    }
    if (!folderExcludes.isEmpty()) {
        m_settings.m_folderExcludes = folderExcludes;
    }
}

void AdvancedSettingsXmlReader::addError(QString tag, ParseErrorType type)
{
    m_messages.push_back({type, tag});
}

void AdvancedSettingsXmlReader::addWarning(QString tag, ParseErrorType type)
{
    m_messages.push_back({type, tag});
}

void AdvancedSettingsXmlReader::skipUnsupportedTag()
{
    qCWarning(generic) << "[AdvancedSettings] Found unsupported xml tag" << m_xml.name() << "at" << currentLocation();
    addError(m_xml.name().toString(), ParseErrorType::UnsupportedTag);
    m_xml.skipCurrentElement();
}

void AdvancedSettingsXmlReader::invalidValue()
{
    qCWarning(generic) << "[AdvancedSettings] Invalid value for xml tag" << m_xml.name() << "at" << currentLocation();
    addError(m_xml.name().toString(), ParseErrorType::InvalidValue);
}

QString AdvancedSettingsXmlReader::currentLocation() const
{
    return QString("%1:%2").arg(m_xml.lineNumber()).arg(m_xml.columnNumber());
}

void AdvancedSettingsXmlReader::expectBool(bool& valueToSet)
{
    const QString val = m_xml.readElementText().trimmed().toLower();
    if (val == "true" || val == "1") {
        valueToSet = true;
    } else if (val == "false" || val == "0") {
        valueToSet = false;
    } else {
        invalidValue();
    }
}

void AdvancedSettingsXmlReader::expectInt(int& valueToSet)
{
    bool ok = false;
    const int val = m_xml.readElementText().trimmed().toInt(&ok);
    if (ok) {
        valueToSet = val;
    } else {
        invalidValue();
    }
}

template<typename Callback>
void AdvancedSettingsXmlReader::expectIntChecked(int& valueToSet, Callback fct)
{
    bool ok = false;
    const int val = m_xml.readElementText().trimmed().toInt(&ok);
    if (ok && fct(val)) {
        valueToSet = val;
    } else {
        invalidValue();
    }
}
