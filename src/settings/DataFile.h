#pragma once

#include "data/tv_show/SeasonNumber.h"
#include "globals/Globals.h"

#include <QString>
#include <QStringList>
#include <QVector>

/// Options for looking up existing sidecars (NFO/art) next to a video file.
struct SidecarLookupOptions
{
    /// Also try saveFileName(..., stacked=false) when stacked was true.
    bool tryNonStacked = false;
    /// Apply patterns using the parent folder name as basename (Folder.nfo / Folder-poster.jpg).
    bool tryFolderBasename = false;
    /// Also try literal "movie.nfo" (Kodi folder convention). Prefer after folder basename.
    bool tryGenericMovieNfo = false;

    static SidecarLookupOptions strict() { return {}; }

    static SidecarLookupOptions forMovieLoad()
    {
        SidecarLookupOptions o;
        o.tryNonStacked = true;
        o.tryFolderBasename = true;
        o.tryGenericMovieNfo = true;
        return o;
    }

    static SidecarLookupOptions forConcertLoad()
    {
        SidecarLookupOptions o;
        o.tryNonStacked = true;
        o.tryFolderBasename = true;
        return o;
    }

    static SidecarLookupOptions forEpisodeLoad()
    {
        SidecarLookupOptions o;
        o.tryNonStacked = true;
        return o;
    }

    static SidecarLookupOptions forImageLoad()
    {
        SidecarLookupOptions o;
        o.tryNonStacked = true;
        o.tryFolderBasename = true;
        return o;
    }
};

class DataFile
{
public:
    DataFile() = default;
    DataFile(DataFileType type, QString fileName, int pos);
    DataFileType type() const;
    QString fileName() const;
    int pos() const;
    QString saveFileName(const QString& fileName,
        SeasonNumber season = SeasonNumber::NoSeason,
        bool stacked = false) const;
    static bool lessThan(DataFile a, DataFile b);
    void setFileName(QString fileName);

    static DataFileType dataFileTypeForImageType(ImageType imageType);

    /// Ordered sidecar file-name candidates (not full paths) for lookup next to a video.
    /// Order: primary (stacked) → optional non-stacked → folder basename patterns → movie.nfo.
    static QStringList sidecarFileNameCandidates(const QVector<DataFile>& dataFiles,
        const QString& videoFileName,
        bool stacked,
        const QString& folderName,
        SidecarLookupOptions options);

private:
    QString m_fileName;
    int m_pos = 0;
    DataFileType m_type = DataFileType::NoType;
};
