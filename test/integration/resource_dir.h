#pragma once

#include <QDir>
#include <QString>
#include <memory>

QDir resourceDir();
void setResourceDir(QDir dir);
/// Reads the contents of the given file inside the
/// resource directory. Throws if the file is not found
/// or not readable.
QString getFileContent(QString filepath);
/// Reads the contents of the given file inside the
/// temp directory. Throws if the file is not found
/// or not readable.
QString getTempFileContent(QString filepath);
/// Get a temporary directory (usually the build directory) to write files to.
/// The given subdirectory (e.g. "export/simple") will be created if it does
/// not exist.
QDir tempDir(QString subDir = "");
void setTempDir(QDir dir);
/// Creates a temporary file in the build directory. If the filepath
/// contains slashes ("/") all parts prior to it will be recognized
/// as subdirectories and will be created if no existent.
/// Useful when you compare NFO files and want the resutls.
void writeTempFile(QString filepath, QString content);
