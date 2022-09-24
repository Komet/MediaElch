#pragma once

#include "utils/Meta.h"

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

namespace mediaelch {

/// \brief Deprecated version of MovieFileSearcher
/// Deprecated Use MovieFileSearcher instead.
class MovieDirScan : public QObject
{
    Q_OBJECT
public:
    MovieDirScan(QObject* parent = nullptr) : QObject(parent) {}
    ~MovieDirScan() override = default;

    /// \brief Scans the given path for movie files.
    ///
    /// Results are in a list which contains a QStringList for every movie.
    ///
    /// \param startPath Scanning started at this path
    /// \param path Path to scan
    /// \param contents List of contents
    /// \param separateFolders Are concerts in separate folders
    /// \param firstScan When this is true, subfolders are scanned, regardless of separateFolders
    /// \deprecated Use MovieFileSearcher::reload() instead
    /// \note Only used in MovieFilesOrganizer
    ELCH_DEPRECATED void scanDir(QString startPath,
        QString path,
        QVector<QStringList>& contents,
        bool separateFolders = false,
        bool firstScan = false);

public slots:
    void abort();

signals:
    void currentDir(QString);

private:
    /// Get a list of files in a directory
    /// \deprecated Remove with scanDir
    ELCH_DEPRECATED QStringList getFiles(QString path);

private:
    QHash<QString, QDateTime> m_lastModifications;
    bool m_aborted = false;
};

} // namespace mediaelch
