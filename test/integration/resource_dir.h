#pragma once

#include <QDir>
#include <memory>

QDir resourceDir();
void setResourceDir(QDir dir);
/// Reads the contents of the given file inside the
/// resource directory. Throws if the file is not found
/// or not readable.
QString getFileContent(QString filename);
