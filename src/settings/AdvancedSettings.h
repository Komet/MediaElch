#pragma once

#include "file/FileFilter.h"
#include "globals/Globals.h"
#include "image/ThumbnailDimensions.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QVector>

class FileSearchExclude
{
public:
    static FileSearchExclude excludeFilePattern(QRegularExpression regex)
    {
        return FileSearchExclude(ExcludeType::File, regex);
    }
    static FileSearchExclude excludeFolderPattern(QRegularExpression regex)
    {
        return FileSearchExclude(ExcludeType::Folder, regex);
    }

    FileSearchExclude() = default; // required for QVector

    bool matchFilename(QString filename) const
    {
        if (m_type == ExcludeType::File) {
            return m_regex.isValid() && m_regex.match(filename).hasMatch();
        }
        return false;
    }

    bool matchFoldername(QString folder) const
    {
        if (m_type == ExcludeType::Folder) {
            return m_regex.isValid() && m_regex.match(folder).hasMatch();
        }
        return false;
    }

    QString toString() const { return excludeTypeToString(m_type) + ": " + m_regex.pattern(); }

private:
    enum class ExcludeType
    {
        File,
        Folder
    };

    QString excludeTypeToString(ExcludeType type) const
    {
        switch (type) {
        case ExcludeType::File: return "file";
        case ExcludeType::Folder: return "folder";
        }
        qCritical() << "[ExcludeType] Unhandled switch/case";
        return "";
    }

    FileSearchExclude(ExcludeType type, QRegularExpression regex) : m_type{type}, m_regex{regex} {}

private:
    ExcludeType m_type = ExcludeType::File;
    QRegularExpression m_regex;
};

class AdvancedSettings
{
public:
    AdvancedSettings();

    bool debugLog() const;
    QString logFile() const;
    QLocale locale() const;
    QStringList sortTokens() const;
    QString customStylesheet() const;
    QHash<QString, QString> genreMappings() const;

    const mediaelch::FileFilter& movieFilters() const;
    const mediaelch::FileFilter& concertFilters() const;
    const mediaelch::FileFilter& tvShowFilters() const;
    const mediaelch::FileFilter& subtitleFilters() const;

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
    mediaelch::ThumbnailDimensions episodeThumbnailDimensions() const;

    bool isFileExcluded(QString file) const;
    bool isFolderExcluded(QString dir) const;

    friend class AdvancedSettingsXmlReader;
    friend QDebug operator<<(QDebug dbg, const AdvancedSettings& settings);

private:
    void setLocale(QString locale);

private:
    bool m_debugLog = false;
    QString m_logFile;
    QLocale m_locale;
    QStringList m_sortTokens;
    QString m_customStylesheet;
    QHash<QString, QString> m_genreMappings;
    mediaelch::FileFilter m_movieFilters;
    mediaelch::FileFilter m_concertFilters;
    mediaelch::FileFilter m_tvShowFilters;
    mediaelch::FileFilter m_subtitleFilters;
    QHash<QString, QString> m_audioCodecMappings;
    QHash<QString, QString> m_videoCodecMappings;
    QHash<QString, QString> m_certificationMappings;
    QHash<QString, QString> m_studioMappings;
    QHash<QString, QString> m_countryMappings;
    mediaelch::ThumbnailDimensions m_episodeThumbnailDimensions;
    QVector<FileSearchExclude> m_excludePatterns;
    bool m_forceCache = false;
    bool m_portableMode = false;
    int m_bookletCut = 2;
    bool m_writeThumbUrlsToNfo = true;
    bool m_useFirstStudioOnly = false;
};

QDebug operator<<(QDebug dbg, const AdvancedSettings& settings);
QDebug operator<<(QDebug dbg, const AdvancedSettings* settings);
