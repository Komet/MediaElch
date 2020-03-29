#pragma once

#include <QDir>
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

private:
    bool m_isValid = false;
    QDir m_dir;
};

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
    explicit FilePath(const QString& filePath) : m_isValid{true}, m_dir(QDir(filePath)) {}
    explicit FilePath(QDir filePath) : m_isValid{true}, m_dir(std::move(filePath)) {}

    bool isValid() const { return m_isValid; }

private:
    bool m_isValid = false;
    QDir m_dir;
};


} // namespace mediaelch
