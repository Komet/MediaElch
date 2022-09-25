#pragma once

#include <QDir>
#include <QString>
#include <memory>

namespace test {

/// Resource directory used by all tests, as well as resource-dir related
/// functions such as writeResourceFile().
QDir resourceDir();

void setResourceDir(QDir dir);

/// Reads the contents of the given file inside the resource directory.
/// Throws if the file is not found or not readable.
QString readResourceFile(const QString& filename);

/// Writes the contents of the given file inside the
/// resource directory. Throws if the file is not not writable.
void writeResourceFile(const QString& filename, const QString& content);


/// Get a temporary directory (usually the build directory) to write files to.
/// The given subdirectory (e.g. "export/simple") will be created if it does
/// not exist.
QDir makeTempDir(QString subDir = "");

/// Set the root temporary directory in which all sub-directories
/// are created by makeTempDir().
void setTempRootDir(QDir dir);

/// Reads the contents of the given file inside the
/// temp directory. Throws if the file is not found
/// or not readable.
QString readTempFile(const QString& filepath);

/// Creates a temporary file in the build directory. If the filepath
/// contains slashes ("/") all parts prior to it will be recognized
/// as subdirectories and will be created if no existent.
/// Useful when you compare NFO files and want the results.
void writeTempFile(const QString& filepath, const QString& content);

} // namespace test
