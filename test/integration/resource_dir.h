#pragma once

#include <QDir>
#include <memory>

QDir resourceDir();
void setResourceDir(QDir dir);
/// Reads the contents of the given file inside the
/// resource directory. Throws if the file is not found
/// or not readable.
QString getFileContent(QString filename);

QDir tempDir();
void setTempDir(QDir dir);
/// Creates a temporary file in the build directory.
/// Useful when you compare NFO files and want the resutls.
void writeTempFile(QString filename, QString content);
