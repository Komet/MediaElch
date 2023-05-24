#pragma once

#include <QDir>
#include <QString>
#include <memory>

namespace test {

/// Returns true if reference/resource files should be updated.
/// Used by XML integration tests and scraper tests.
/// Checks environment variable MEDIAELCH_UPDATE_REF_FILES
bool shouldUpdateResourceFiles();

/// Resource directory used by all tests, as well as resource-dir related
/// functions such as writeResourceFile().
QDir resourceDir();

/// Set the resource directory from which all resource files are loaded.
/// If the given directory does not exist, an exception will be thrown.
void setResourceDir(QDir dir);
void setResourceDir(const std::string& dir);

/// Reads the contents of the given file inside the resource directory.
/// Throws if the file is not found or not readable.
///
/// If MEDIAELCH_UPDATE_REF_FILES is set, returns an empty string
/// if the file is not readable.
QString readResourceFile(const QString& filename);

/// Writes the contents of the given file inside the
/// resource directory. Throws if the file is not not writable.
void writeResourceFile(const QString& filename, const QString& content);


/// Get a temporary directory (usually the build directory) to write files to.
/// The given subdirectory (e.g. "export/simple") will be created if it does
/// not exist.
QDir makeTempDir(QString subDir = "");

QDir tempRootDir();

/// Set the root temporary directory in which all sub-directories
/// are created by makeTempDir().  If the given directory does not
/// exist, it will be created on-demand.
void setTempRootDir(QDir dir);
void setTempRootDir(const std::string& dir);

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
