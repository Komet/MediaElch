#include "AdvancedSettings.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>

#include "Settings.h"

AdvancedSettings::AdvancedSettings(QObject* parent) : QObject(parent)
{
    m_portableMode = false;
    m_bookletCut = 2;
    loadSettings();
}

void AdvancedSettings::reset()
{
    m_debugLog = false;
    m_locale = QLocale::system();
    m_forceCache = false;
    m_bookletCut = 2;
    m_logFile = "";
    m_sortTokens = QStringList{"Der", "Die", "Das", "The", "Le", "La", "Les", "Un", "Une", "Des"};
    m_genreMappings.clear();
    m_audioCodecMappings.clear();
    m_videoCodecMappings.clear();
    m_certificationMappings.clear();
    m_studioMappings.clear();
    m_countryMappings.clear();
    m_writeThumbUrlsToNfo = true;
    m_useFirstStudioOnly = false;

    m_audioCodecMappings.insert("mpa1l2", "mp2");
    m_audioCodecMappings.insert("mpa1l3", "mp3");
    m_audioCodecMappings.insert("aac lc", "aac");
    m_videoCodecMappings.insert("v_mpeg4/iso/avc", "h264");

    const auto videoFiles = mediaelch::FileFilter({"*.mkv",
        "*.mk3d",
        "*.avi",
        "*.mpg",
        "*.mpeg",
        "*.mp4",
        "*.m2ts",
        "*.disc",
        "*.m4v",
        "*.strm",
        "*.dat",
        "*.flv",
        "*.vob",
        "*.ts",
        "*.iso",
        "*.ogg",
        "*.ogm",
        "*.rmvb",
        "*.img",
        "*.wmv",
        "*.mov",
        "*.divx",
        "VIDEO_TS.IFO",
        "index.bdmv",
        "*.wtv"});

    // Assign video filters.
    m_movieFilters = videoFiles;
    m_tvShowFilters = videoFiles;
    m_concertFilters = videoFiles;

    m_subtitleFilters = mediaelch::FileFilter({"*.idx", "*.sub", "*.srr", "*.srt", "*.ass", "*.ttml"});
}

QByteArray AdvancedSettings::getAdvancedSettingsXml() const
{
    QByteArray xmlStr;

    QFile file(Settings::applicationDir() + "/advancedsettings.xml");
    if (!file.exists()) {
        file.setFileName(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/advancedsettings.xml");
    }

    if (!file.exists()) {
        return xmlStr;
    }

    if (file.open(QIODevice::ReadOnly)) {
        xmlStr = file.readAll();
        file.close();
    } else {
        qDebug() << "Cannot open advancedsettings.xml in ReadOnly mode.";
    }

    return xmlStr;
}

void AdvancedSettings::loadSettings()
{
    qDebug() << "Loading advanced settings...";
    reset();

    QXmlStreamReader xml{getAdvancedSettingsXml()};

    if (!xml.readNextStartElement() || xml.name().toString() != "advancedsettings") {
        qDebug() << "No advanced settings found!";
        return;
    }

    while (xml.readNextStartElement()) {
        if (xml.name() == "log") {
            loadLog(xml);

        } else if (xml.name() == "locale") {
            setLocale(xml.readElementText());

        } else if (xml.name() == "portableMode") {
            m_portableMode = (xml.readElementText() == "true");

        } else if (xml.name() == "gui") {
            loadGui(xml);

        } else if (xml.name() == "writeThumbUrlsToNfo") {
            m_writeThumbUrlsToNfo = (xml.readElementText() == "true");

        } else if (xml.name() == "episodeThumb") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "width") {
                    bool ok = false;
                    int width = xml.readElementText().trimmed().toInt(&ok);
                    if (ok) {
                        m_episodeThumbnailDimensions.width = width;
                    }

                } else if (xml.name() == "height") {
                    bool ok = false;
                    int height = xml.readElementText().trimmed().toInt(&ok);
                    if (ok) {
                        m_episodeThumbnailDimensions.height = height;
                    }
                } else {
                    qWarning() << "[AdvancedSettings] Found unsupported tag inside <episodeThumb>:" << xml.name();
                    break;
                }
            }

        } else if (xml.name() == "bookletCut") {
            m_bookletCut = xml.readElementText().toInt();

        } else if (xml.name() == "sorttokens") {
            loadSortTokens(xml);

        } else if (xml.name() == "genres") {
            loadMappings(xml, m_genreMappings);

        } else if (xml.name() == "audioCodecs") {
            loadMappings(xml, m_audioCodecMappings);

        } else if (xml.name() == "videoCodecs") {
            loadMappings(xml, m_videoCodecMappings);

        } else if (xml.name() == "certifications") {
            loadMappings(xml, m_certificationMappings);

        } else if (xml.name() == "studios") {
            if (xml.attributes().hasAttribute("useFirstStudioOnly")) {
                const auto firstStudioOnly = xml.attributes().value("useFirstStudioOnly").trimmed();
                m_useFirstStudioOnly = (firstStudioOnly == "true");
            }
            loadMappings(xml, m_studioMappings);

        } else if (xml.name() == "countries") {
            loadMappings(xml, m_countryMappings);

        } else if (xml.name() == "fileFilters") {
            loadFilters(xml);

        } else {
            xml.skipCurrentElement();
        }
    }

    qDebug() << "Advanced settings";
    qDebug() << "    locale                " << m_locale;
    qDebug() << "    debugLog              " << m_debugLog;
    qDebug() << "    logFile               " << m_logFile;
    qDebug() << "    forceCache            " << m_forceCache;
    qDebug() << "    sortTokens            " << m_sortTokens;
    qDebug() << "    movieFilters          " << m_movieFilters.filters();
    qDebug() << "    concertFilters        " << m_concertFilters.filters();
    qDebug() << "    tvShowFilters         " << m_tvShowFilters.filters();
    qDebug() << "    subtitleFilters       " << m_subtitleFilters.filters();
    qDebug() << "    genreMappings         " << m_genreMappings;
    qDebug() << "    audioCodecMappings    " << m_audioCodecMappings;
    qDebug() << "    videoCodecMappings    " << m_videoCodecMappings;
    qDebug() << "    certificationMappings " << m_certificationMappings;
    qDebug() << "    studioMappings        " << m_studioMappings;
    qDebug() << "    countryMappings       " << m_countryMappings;
    qDebug() << "    writeThumbUrlsToNfo   " << m_writeThumbUrlsToNfo;
    qDebug() << "    episodeThumb dimensions:";
    qDebug() << "      width:   " << m_episodeThumbnailDimensions.width;
    qDebug() << "      height:  " << m_episodeThumbnailDimensions.height;
    qDebug() << "    bookletCut            " << m_bookletCut;
    qDebug() << "    useFirstStudioOnly    " << m_useFirstStudioOnly;
}

void AdvancedSettings::setLocale(QString locale)
{
    locale = locale.trimmed();
    if (locale.toLower() == "system") {
        m_locale = QLocale::system();

    } else {
        m_locale = QLocale(locale);
        // If "locale" is not a valid locale, Qt uses the C locale.
        // Because `.name()` also returns "C" instead of the systems locale,
        // we have to check for it.
        if (m_locale.name() == "C") {
            m_locale = QLocale::system();
        }
    }
}

void AdvancedSettings::loadLog(QXmlStreamReader& xml)
{
    while (xml.readNextStartElement()) {
        if (xml.name() == "debug") {
            m_debugLog = (xml.readElementText().trimmed() == "true");
        } else if (xml.name() == "file") {
            m_logFile = xml.readElementText().trimmed();
        } else {
            xml.skipCurrentElement();
        }
    }
}

void AdvancedSettings::loadGui(QXmlStreamReader& xml)
{
    while (xml.readNextStartElement()) {
        if (xml.name() == "forceCache") {
            m_forceCache = (xml.readElementText().trimmed() == "true");
        } else {
            xml.skipCurrentElement();
        }
    }
}

void AdvancedSettings::loadSortTokens(QXmlStreamReader& xml)
{
    m_sortTokens.clear();
    while (xml.readNextStartElement()) {
        if (xml.name() == "token") {
            m_sortTokens << xml.readElementText().trimmed();
        } else {
            xml.skipCurrentElement();
        }
    }
}

void AdvancedSettings::loadFilters(QXmlStreamReader& xml)
{
    qDebug() << "loading filters";

    /**
     * The current XML element's text is split by "," and all items
     * are appended to a cleared "list".
     */
    const auto appendNextFiltersToList = [&xml](mediaelch::FileFilter& list) {
        QStringList newFilters;
        const auto filters = xml.readElementText().split(",", QString::SkipEmptyParts);
        for (const QString& filter : filters) {
            newFilters << filter.trimmed();
        }
        list = mediaelch::FileFilter(newFilters);
    };

    while (xml.readNextStartElement()) {
        if (xml.name() == "movies") {
            appendNextFiltersToList(m_movieFilters);

        } else if (xml.name() == "concerts") {
            appendNextFiltersToList(m_concertFilters);

        } else if (xml.name() == "tvShows") {
            appendNextFiltersToList(m_tvShowFilters);

        } else if (xml.name() == "subtitle") {
            appendNextFiltersToList(m_subtitleFilters);

        } else {
            xml.skipCurrentElement();
        }
    }
}

/**
 * @brief Load the next mappings inside <map> tags into "mappings".
 * @param xml XML stream with its position right before <map> elements.
 * @param mappings QHash table that will be cleared and to which the mappings will be appended.
 */
void AdvancedSettings::loadMappings(QXmlStreamReader& xml, QHash<QString, QString>& mappings)
{
    mappings.clear();
    while (xml.readNextStartElement()) {
        if (xml.name() == "map" && xml.attributes().hasAttribute("from")) {
            const auto from = xml.attributes().value("from").trimmed();
            const auto to = xml.attributes().value("to").trimmed();
            if (!from.isEmpty() && !to.isEmpty()) {
                mappings.insert(from.toString(), to.toString());
            }
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

QLocale AdvancedSettings::locale() const
{
    return m_locale;
}

QStringList AdvancedSettings::sortTokens() const
{
    return m_sortTokens;
}

QHash<QString, QString> AdvancedSettings::genreMappings() const
{
    return m_genreMappings;
}

const mediaelch::FileFilter& AdvancedSettings::movieFilters() const
{
    return m_movieFilters;
}

const mediaelch::FileFilter& AdvancedSettings::concertFilters() const
{
    return m_concertFilters;
}

const mediaelch::FileFilter& AdvancedSettings::tvShowFilters() const
{
    return m_tvShowFilters;
}

const mediaelch::FileFilter& AdvancedSettings::subtitleFilters() const
{
    return m_subtitleFilters;
}

QHash<QString, QString> AdvancedSettings::audioCodecMappings() const
{
    return m_audioCodecMappings;
}

QHash<QString, QString> AdvancedSettings::videoCodecMappings() const
{
    return m_videoCodecMappings;
}

QHash<QString, QString> AdvancedSettings::certificationMappings() const
{
    return m_certificationMappings;
}

QHash<QString, QString> AdvancedSettings::studioMappings() const
{
    return m_studioMappings;
}

QHash<QString, QString> AdvancedSettings::countryMappings() const
{
    return m_countryMappings;
}

bool AdvancedSettings::forceCache() const
{
    return m_forceCache;
}

bool AdvancedSettings::portableMode() const
{
#ifdef Q_OS_WIN
    return m_portableMode;
#else
    return false;
#endif
}

int AdvancedSettings::bookletCut() const
{
    return m_bookletCut;
}

bool AdvancedSettings::writeThumbUrlsToNfo() const
{
    return m_writeThumbUrlsToNfo;
}

mediaelch::ThumbnailDimensions AdvancedSettings::episodeThumbnailDimensions() const
{
    return m_episodeThumbnailDimensions;
}

bool AdvancedSettings::useFirstStudioOnly() const
{
    return m_useFirstStudioOnly;
}
