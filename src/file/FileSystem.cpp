#include "file/FileSystem.h"

#include "globals/Meta.h"
#include "log/Log.h"

#include <QDir>

std::shared_ptr<mediaelch::FileSystemLog> fileLog = nullptr;

static void logFileWrite(const QString& filepath, QIODevice::OpenMode mode, QFileDevice::FileError error)
{
    if (fileLog) {
        fileLog->logFileWrite(filepath, mode, error);
    }
}

static QFileDevice::FileError writeToFile(const QString& filepath, QIODevice::OpenMode mode, const QByteArray& content)
{
    QFile file(filepath);
    if (!file.open(mode | QIODevice::WriteOnly)) {
        logFileWrite(filepath, mode, file.error());
        return file.error();
    }
    qint64 bytesWritten = file.write(content);
    if (bytesWritten < 0) {
        logFileWrite(filepath, mode, file.error());
        return file.error();
    }
    file.close();
    auto status = file.error();
    logFileWrite(filepath, mode, status);
    MediaElch_Ensures(status == QFileDevice::NoError);
    return status;
}


namespace mediaelch {

QFileDevice::FileError writeTextToFile(const QString& filepath, const QByteArray& content)
{
    return writeToFile(filepath, QIODevice::Text, content);
}

QFileDevice::FileError writeTextToFile(const QString& filepath, const QString& content)
{
    return writeToFile(filepath, QIODevice::Text, content.toUtf8());
}

QFileDevice::FileError writeBinaryToFile(const QString& filepath, const QByteArray& content)
{
    return writeToFile(filepath, {}, content);
}

void FileSystemLog::logFileWrite(const QString& filename, QIODevice::OpenMode mode, QFileDevice::FileError error)
{
    const char* modeStr = mode.testFlag(QIODevice::Text) ? "Text" : "Binray";
    switch (error) {
    case QFileDevice::NoError: //
        qCDebug(generic) << "[File]" << modeStr << "file written to:" << filename;
        break;
    case QFileDevice::OpenError: //
        qCDebug(generic) << "[File]" << modeStr << "file could not be opened:" << filename;
        break;
    case QFileDevice::WriteError:
    default:
        qCDebug(generic) << "[File] Error while writing to file:" //
                         << filename << "| code:"                 //
                         << static_cast<int>(error);
    }
}

void installFileSystemLog(std::shared_ptr<FileSystemLog> log)
{
    fileLog = log;
}

} // namespace mediaelch
