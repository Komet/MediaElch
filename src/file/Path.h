#pragma once

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>

#include <memory>

namespace mediaelch {

/// \brief Represents the path to a directory.
///
/// DirectoryPath is a wrapper around QDir and works with "normalized" paths,
/// i.e. on Windows the forward slash is used as a separator as well.
/// A path is always stored as an absolute path. Relative paths are not used.
/// A default-constructed DirectoryPath is not valid. This behaviour differs
/// from QDir.
class DirectoryPath
{
public:
    DirectoryPath() = default;
    /* implicit */ DirectoryPath(const QString& path) : m_isValid{!path.isEmpty()}, m_dir(QDir(path)) {}
    /* implicit */ DirectoryPath(QDir path) : m_isValid{true}, m_dir(std::move(path)) {}

    bool isValid() const { return m_isValid; }

    /// \brief Returns the string representation of the normalized absolute path.
    QString toString() const { return m_dir.absolutePath(); }
    QString toNativePathString() const;

    /// \brief Returns true if the path is a parent folder of child or is the same directory.
    bool isParentFolderOf(const DirectoryPath& child);

    QString dirName() const { return m_dir.dirName(); }
    /// \brief Returns the directory as a QDir.
    QDir dir() const { return m_dir; }

    QString filePath(const QString& fileName) const;

    DirectoryPath subDir(const QString& dirName) const;


private:
    bool m_isValid = false;
    QDir m_dir;
};

bool operator==(const DirectoryPath& lhs, const DirectoryPath& rhs);
bool operator!=(const DirectoryPath& lhs, const DirectoryPath& rhs);

QDebug operator<<(QDebug debug, const DirectoryPath& dir);

inline uint qHash(const DirectoryPath& key, uint seed)
{
    return qHash(key.toString(), seed);
}


/// \brief A file path represents the location of a file in the file system.
///
/// FilePath is a wrapper around QDir with changes to support files as well.
/// A path is always stored as an absolute path. Relative paths are not used.
/// A default-constructed FilePath is not valid. This behaviour differs from
/// QDir.
class FilePath
{
public:
    FilePath() = default;
    /* implicit */ FilePath(const QString& path) : m_isValid{!path.isEmpty()}, m_fileInfo(path) {}
    explicit FilePath(QFileInfo filePath) : m_isValid{true}, m_fileInfo(std::move(filePath)) {}

    bool isValid() const { return m_isValid; }

    /// \brief Returns the string representation of the normalized absolute path.
    QString toString() const { return m_fileInfo.absoluteFilePath(); }

    QString toNativePathString() const;

    DirectoryPath dir() const;

private:
    bool m_isValid = false;
    QFileInfo m_fileInfo;
};

bool operator==(const FilePath& lhs, const FilePath& rhs);
bool operator!=(const FilePath& lhs, const FilePath& rhs);

QDebug operator<<(QDebug debug, const FilePath& dir);

inline uint qHash(const FilePath& key, uint seed)
{
    return qHash(key.toString(), seed);
}


} // namespace mediaelch
