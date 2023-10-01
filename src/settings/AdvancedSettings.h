#pragma once

#include "data/ThumbnailDimensions.h"
#include "media/FileFilter.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QLocale>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QVector>

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
    const mediaelch::FileFilter& musicFilters() const;
    const mediaelch::FileFilter& subtitleFilters() const;

    QHash<QString, QString> audioCodecMappings() const;
    QHash<QString, QString> videoCodecMappings() const;
    QHash<QString, QString> certificationMappings() const;
    QHash<QString, QString> studioMappings() const;
    QHash<QString, QString> countryMappings() const;

    bool useFirstStudioOnly() const;
    bool portableMode() const;
    int bookletCut() const;
    bool writeThumbUrlsToNfo() const;
    mediaelch::ThumbnailDimensions episodeThumbnailDimensions() const;

    /// \brief Returns true if the user has provided a custom advancedsettings.xml
    ///        "false" if default values are used.
    bool isUserDefined() const;

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
    mediaelch::FileFilter m_musicFilters;
    QHash<QString, QString> m_audioCodecMappings;
    QHash<QString, QString> m_videoCodecMappings;
    QHash<QString, QString> m_certificationMappings;
    QHash<QString, QString> m_studioMappings;
    QHash<QString, QString> m_countryMappings;
    mediaelch::ThumbnailDimensions m_episodeThumbnailDimensions;
    QVector<QRegularExpression> m_fileExcludes;
    QVector<QRegularExpression> m_folderExcludes;
    int m_bookletCut = 2;
    bool m_portableMode = false;
    bool m_writeThumbUrlsToNfo = true;
    bool m_useFirstStudioOnly = false;
    bool m_userDefined = false;
};

QDebug operator<<(QDebug dbg, const AdvancedSettings& settings);
QDebug operator<<(QDebug dbg, const AdvancedSettings* settings);
