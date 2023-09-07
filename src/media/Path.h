#pragma once

#include "utils/Meta.h"

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

    ELCH_NODISCARD bool isValid() const { return m_isValid; }

    /// \brief Returns the string representation of the normalized absolute path.
    ELCH_NODISCARD QString toString() const { return m_dir.absolutePath(); }
    ELCH_NODISCARD QString toNativePathString() const;

    /// \brief Returns true if the path is a parent folder of child or is the same directory.
    ELCH_NODISCARD bool isParentFolderOf(const DirectoryPath& child) const;

    ELCH_NODISCARD QString absolutePath() const { return m_dir.absolutePath(); }
    ELCH_NODISCARD QString path() const { return m_dir.path(); }
    void setPath(const QString& path) { m_dir.setPath(path); }
    void setPath(QDir path) { m_dir = std::move(path); }
    ELCH_NODISCARD QString dirName() const { return QDir(m_dir.absolutePath()).dirName(); }
    ELCH_NODISCARD bool isReadable() const { return m_dir.isReadable(); }

    /// \brief Returns the directory as a QDir.
    ELCH_NODISCARD QDir dir() const { return m_dir; }

    ELCH_NODISCARD QString filePath(const QString& fileName) const;

    ELCH_NODISCARD DirectoryPath subDir(const QString& dirName) const;


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

    ELCH_NODISCARD bool isValid() const { return m_isValid; }

    /// \brief Returns the string representation of the normalized absolute path.
    ELCH_NODISCARD QString toString() const { return m_fileInfo.absoluteFilePath(); }

    ELCH_NODISCARD QString toNativePathString() const;

    ELCH_NODISCARD DirectoryPath dir() const;
    ELCH_NODISCARD QString fileName() const { return m_fileInfo.fileName(); }
    ELCH_NODISCARD QString fileSuffix() const { return m_fileInfo.suffix(); }

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
    FileList(std::initializer_list<FilePath> files) : m_files{std::move(files)} {}
    explicit FileList(QVector<FilePath> files) : m_files{std::move(files)} {}
    /// \brief Convenience constructor to create a file list from a QStringList.
    /// \todo Remove this constructor when all paths are transformed to FilePaths.
    /* implicit */ FileList(const QStringList& files)
    {
        for (const QString& file : files) {
            m_files.push_back(mediaelch::FilePath(file));
        }
    }

    ELCH_NODISCARD elch_ssize_t size() const { return m_files.size(); }
    ELCH_NODISCARD elch_ssize_t count() const { return m_files.count(); }
    ELCH_NODISCARD bool isEmpty() const { return m_files.isEmpty(); }

    void clear() { m_files.clear(); }

    ELCH_NODISCARD FilePath& first() { return m_files.first(); }
    ELCH_NODISCARD const FilePath& first() const { return m_files.first(); }

    ELCH_NODISCARD FilePath& last() { return m_files.last(); }
    ELCH_NODISCARD const FilePath& last() const { return m_files.last(); }

    ELCH_NODISCARD const FilePath& at(int index) const { return m_files.at(index); }
    ELCH_NODISCARD const FilePath& operator[](int index) const { return m_files[index]; }

    ELCH_NODISCARD QStringList toStringList() const
    {
        QStringList paths;
        for (const auto& file : m_files) {
            paths.push_back(file.toString());
        }
        return paths;
    }

    ELCH_NODISCARD QStringList toNativeStringList() const
    {
        QStringList paths;
        for (const auto& file : m_files) {
            paths.push_back(file.toNativePathString());
        }
        return paths;
    }

    void push_back(const FilePath& file) { return m_files.push_back(file); }
    void push_back(FilePath&& file) { return m_files.push_back(std::forward<FilePath>(file)); }

    ELCH_NODISCARD auto begin() { return m_files.begin(); }
    ELCH_NODISCARD auto begin() const { return m_files.begin(); }
    ELCH_NODISCARD auto end() { return m_files.end(); }
    ELCH_NODISCARD auto end() const { return m_files.end(); }

private:
    QVector<FilePath> m_files;
};

bool operator==(const FileList& lhs, const FileList& rhs);
bool operator!=(const FileList& lhs, const FileList& rhs);

void operator<<(FileList& list, const FilePath& file);


QString pathHash(const mediaelch::FilePath& path);

} // namespace mediaelch
