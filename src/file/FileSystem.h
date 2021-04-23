#pragma once

#include "globals/Meta.h"

#include <QFile>
#include <QFileDevice>
#include <QObject>
#include <QString>
#include <memory>

namespace mediaelch {

/// \brief Write the given text to a file.
/// \details Also logs the results of the file access.
ELCH_NODISCARD auto writeTextToFile(const QString& filepath, const QByteArray& content) -> QFileDevice::FileError;
ELCH_NODISCARD auto writeTextToFile(const QString& filepath, const QString& content) -> QFileDevice::FileError;

/// \brief Write the given binary data to a file.
/// \details Also logs the results of the file access.
ELCH_NODISCARD auto writeBinaryToFile(const QString& filepath, const QByteArray& contents) -> QFileDevice::FileError;

/// \brief Base class for filesystem logging.
class FileSystemLog : public QObject
{
    Q_OBJECT
public:
    virtual void logFileWrite(const QString& filename, QIODevice::OpenMode mode, QFileDevice::FileError error);
};

void installFileSystemLog(std::shared_ptr<FileSystemLog> log);

} // namespace mediaelch
