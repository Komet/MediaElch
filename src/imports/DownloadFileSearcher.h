#pragma once

#include "settings/Settings.h"

#include <QDirIterator>
#include <QString>

namespace mediaelch {

/// \brief File searcher for importable/"downloadable" files.
class DownloadFileSearcher : public QObject
{
    Q_OBJECT

public:
    struct Package
    {
        QString baseName;
        QStringList files;
        /// Size in Bytes of this package.
        /// Not an int to allow sizes >4GB on 32bit systems
        double size;
    };

    struct Import
    {
        QString baseName;
        QStringList files;
        QStringList extraFiles;
        /// Size in Bytes of this import.
        /// Not an int to allow sizes >4GB on 32bit systems
        double size;
    };

public:
    DownloadFileSearcher(bool scanDownloads, bool scanImports, QObject* parent = nullptr) :
        QObject(parent), m_scanDownloads{scanDownloads}, m_scanImports{scanImports}
    {
    }
    ~DownloadFileSearcher() = default;

    /// \brief Scan the folders that are set in MediaElch's settings for downloads/imports.
    /// \see sigSearchFinished()
    void scan();

signals:
    void sigScanFinished(DownloadFileSearcher& searcher);

public:
    /// \brief Get pairs of (base) file names and packages.
    /// \see scan()
    QMap<QString, Package> packages() { return m_packages; }

    /// \brief Get pairs of (base) file names and imports.
    /// \see scan()
    QMap<QString, Import> imports() { return m_imports; }

private:
    /// \brief Extract the base file name of the given file, i.e. remove all part
    ///        data (e.g. "part1", ".r2") from the file name.
    QString baseName(const QFileInfo& fileInfo) const;

    /// \brief Check whether the given file is a package, e.g. a RAR archive.
    bool isPackage(const QFileInfo& file) const;

    /// \brief Check whether the given file is importable, i.e. matches the file
    ///        filters set by the user in MediaElch's settings.
    bool isImportable(const QFileInfo& file) const;

    /// \brief Check whether the given file matches the subtitle filter
    ///        set by the user in MediaElch's settings.
    bool isSubtitle(const QFileInfo& file) const;

private:
    QMap<QString, Package> m_packages;
    QMap<QString, Import> m_imports;

    bool m_scanDownloads = false;
    bool m_scanImports = false;
};

} // namespace mediaelch
