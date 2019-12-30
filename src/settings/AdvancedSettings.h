#pragma once

#include "file/FileFilter.h"
#include "globals/Globals.h"
#include "image/ThumbnailDimensions.h"

#include <QHash>
#include <QString>
#include <QStringList>

class AdvancedSettings
{
public:
    AdvancedSettings();

    bool debugLog() const;
    QString logFile() const;
    QLocale locale() const;
    QStringList sortTokens() const;
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

    friend class AdvancedSettingsXmlReader;
    friend QDebug operator<<(QDebug dbg, const AdvancedSettings& settings);

private:
    void setLocale(QString locale);

private:
    bool m_debugLog = false;
    QString m_logFile;
    QLocale m_locale;
    QStringList m_sortTokens;
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
    bool m_forceCache = false;
    bool m_portableMode = false;
    int m_bookletCut = 2;
    bool m_writeThumbUrlsToNfo = true;
    bool m_useFirstStudioOnly = false;
};

QDebug operator<<(QDebug dbg, const AdvancedSettings& movie);
QDebug operator<<(QDebug dbg, const AdvancedSettings* movie);
