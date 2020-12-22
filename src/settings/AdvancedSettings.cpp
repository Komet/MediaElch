#include "AdvancedSettings.h"

#include <QDebug>
#include <QTextStream>

#include "Settings.h"

AdvancedSettings::AdvancedSettings()
{
    m_locale = QLocale::system();
    m_sortTokens = QStringList{"Der", "Die", "Das", "The", "Le", "La", "Les", "Un", "Une", "Des"};

    m_audioCodecMappings.insert("mpa1l2", "mp2");
    m_audioCodecMappings.insert("mpa1l3", "mp3");
    m_audioCodecMappings.insert("aac lc", "aac");
    m_videoCodecMappings.insert("v_mpeg4/iso/avc", "h264"); // older MediaInfo versions (v0.7)
    m_videoCodecMappings.insert("avc", "h264");             // newer MediaInfo versions (v17.12)

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
        "*.webm",
        "index.bdmv",
        "*.wtv"});

    // Assign video filters.
    m_movieFilters = videoFiles;
    m_tvShowFilters = videoFiles;
    m_concertFilters = videoFiles;

    m_subtitleFilters = mediaelch::FileFilter({"*.idx", "*.sub", "*.srr", "*.srt", "*.ass", "*.ttml"});
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
    QLocale::setDefault(m_locale);
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

QString AdvancedSettings::customStylesheet() const
{
    return m_customStylesheet;
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

bool AdvancedSettings::isFileExcluded(QString file) const
{
    for (const auto& pattern : m_excludePatterns) {
        if (pattern.matchFilename(file)) {
            return true;
        }
    }
    return false;
}

bool AdvancedSettings::isFolderExcluded(QString dir) const
{
    for (const auto& pattern : m_excludePatterns) {
        if (pattern.matchFoldername(dir)) {
            return true;
        }
    }
    return false;
}

bool AdvancedSettings::useFirstStudioOnly() const
{
    return m_useFirstStudioOnly;
}

QDebug operator<<(QDebug dbg, const AdvancedSettings& settings)
{
    QString nl = "\n";
    QString s;
    QTextStream out(&s);

    const auto printMap = [&nl](QTextStream& stream, const QHash<QString, QString>& map) {
        QHashIterator<QString, QString> i(map);
        while (i.hasNext()) {
            i.next();
            stream << "        " << i.key() << ": " << i.value() << nl;
        }
    };
    const auto printExcludePatterns = [&nl, &out](const QVector<FileSearchExclude>& patterns) {
        for (const auto& pattern : patterns) {
            out << "        - " << pattern.toString() << nl;
        }
    };

    out << "Advanced settings:" << nl;
    out << "    locale:                  " << QLocale::languageToString(settings.m_locale.language()) << " ("
        << QLocale::countryToString(settings.m_locale.country()) << ")" << nl;
    out << "    debugLog:                " << (settings.m_debugLog ? "true" : "false") << nl;
    out << "    logFile:                 " << settings.m_logFile << nl;
    out << "    forceCache:              " << (settings.m_forceCache ? "true" : "false") << nl;
    out << "    stylesheet:              "
        << (settings.m_customStylesheet.isEmpty() ? "<bundled>" : settings.m_customStylesheet) << nl;
    out << "    sortTokens:              " << settings.m_sortTokens.join(", ") << nl;
    out << "    movieFilters:            " << settings.m_movieFilters.filters().join(", ") << nl;
    out << "    concertFilters:          " << settings.m_concertFilters.filters().join(", ") << nl;
    out << "    tvShowFilters:           " << settings.m_tvShowFilters.filters().join(", ") << nl;
    out << "    subtitleFilters:         " << settings.m_subtitleFilters.filters().join(", ") << nl;
    out << "    genreMappings:           " << nl;
    printMap(out, settings.m_genreMappings);

    out << "    audioCodecMappings:      " << nl;
    printMap(out, settings.m_audioCodecMappings);

    out << "    videoCodecMappings:      " << nl;
    printMap(out, settings.m_videoCodecMappings);

    out << "    certificationMappings:   " << nl;
    printMap(out, settings.m_certificationMappings);

    out << "    studioMappings:          " << nl;
    printMap(out, settings.m_studioMappings);

    out << "    countryMappings:         " << nl;
    printMap(out, settings.m_countryMappings);

    out << "    writeThumbUrlsToNfo:     " << (settings.m_writeThumbUrlsToNfo ? "true" : "false") << nl;
    out << "    episodeThumb dimensions: " << nl;
    out << "        width:               " << settings.m_episodeThumbnailDimensions.width << nl;
    out << "        height:              " << settings.m_episodeThumbnailDimensions.height << nl;
    out << "    bookletCut:              " << settings.m_bookletCut << nl;
    out << "    useFirstStudioOnly:      " << (settings.m_useFirstStudioOnly ? "true" : "false") << nl;
    out << "    exclude patterns:        " << nl;
    printExcludePatterns(settings.m_excludePatterns);

    dbg.nospace().noquote() << *out.string();
    return dbg.maybeSpace().maybeQuote();
}

QDebug operator<<(QDebug dbg, const AdvancedSettings* settings)
{
    return dbg << *settings;
}
