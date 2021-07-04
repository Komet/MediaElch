#pragma once

#include "globals/Meta.h"

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
    explicit DirectoryPath(const QString& path) : m_isValid{!path.isEmpty()}, m_dir(QDir(path)) {}
    explicit DirectoryPath(QDir path) : m_isValid{true}, m_dir(std::move(path)) {}

    bool isValid() const { return m_isValid; }

    /// \brief Returns the string representation of the normalized absolute path.
    QString toString() const { return m_dir.absolutePath(); }
    QString toNativePathString() const;

    /// \brief Returns true if the path is a parent folder of child or is the same directory.
    bool isParentFolderOf(const DirectoryPath& child) const;

    QString dirName() const { return QDir(m_dir.absolutePath()).dirName(); }
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

inline ELCH_QHASH_RETURN_TYPE qHash(const DirectoryPath& key, uint seed)
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
    explicit FilePath(const QString& path) : m_isValid{!path.isEmpty()}, m_fileInfo(path) {}
    explicit FilePath(QFileInfo filePath) : m_isValid{true}, m_fileInfo(std::move(filePath)) {}

    bool isValid() const { return m_isValid; }

    /// \brief Returns the string representation of the normalized absolute path.
    QString toString() const { return m_fileInfo.absoluteFilePath(); }

    QString toNativePathString() const;

    DirectoryPath dir() const;
    QString fileName() const { return m_fileInfo.fileName(); }
    QString fileSuffix() const { return m_fileInfo.suffix(); }

private:
    bool m_isValid = false;
    QFileInfo m_fileInfo;
};

bool operator==(const FilePath& lhs, const FilePath& rhs);
bool operator!=(const FilePath& lhs, const FilePath& rhs);

QDebug operator<<(QDebug debug, const FilePath& dir);

inline ELCH_QHASH_RETURN_TYPE qHash(const FilePath& key, uint seed)
{
    return qHash(key.toString(), seed);
}

/// \brief Convenience class for storing multiple file paths.
class FileList
{
public:
    FileList() = default;
    explicit FileList(std::initializer_list<FilePath> files) : m_files{std::move(files)} {}
    explicit FileList(QVector<FilePath> files) : m_files{std::move(files)} {}
    /// \brief Convenience constructor to create a file list from a QStringList.
    /// \todo Remove this constructor when all paths are transformed to FilePaths.
    /* implicit */ FileList(const QStringList& files)
    {
        for (const QString& file : files) {
            m_files.push_back(mediaelch::FilePath(file));
        }
    }

    qsizetype size() const { return m_files.size(); }
    qsizetype count() const { return m_files.count(); }
    bool isEmpty() const { return m_files.isEmpty(); }

    void clear() { m_files.clear(); }

    FilePath& first() { return m_files.first(); }
    const FilePath& first() const { return m_files.first(); }

    FilePath& last() { return m_files.last(); }
    const FilePath& last() const { return m_files.last(); }

    const FilePath& at(int index) const { return m_files.at(index); }
    const FilePath& operator[](int index) const { return m_files[index]; }

    QStringList toStringList() const
    {
        QStringList paths;
        for (const auto& file : m_files) {
            paths.push_back(file.toString());
        }
        return paths;
    }

    QStringList toNativeStringList() const
    {
        QStringList paths;
        for (const auto& file : m_files) {
            paths.push_back(file.toNativePathString());
        }
        return paths;
    }

    void push_back(const FilePath& file) { return m_files.push_back(file); }
    void push_back(FilePath&& file) { return m_files.push_back(std::forward<FilePath>(file)); }

    auto begin() { return m_files.begin(); }
    auto begin() const { return m_files.begin(); }
    auto end() { return m_files.end(); }
    auto end() const { return m_files.end(); }

private:
    QVector<FilePath> m_files;
};

bool operator==(const FileList& lhs, const FileList& rhs);
bool operator!=(const FileList& lhs, const FileList& rhs);

void operator<<(FileList& list, const FilePath& file);


} // namespace mediaelch
