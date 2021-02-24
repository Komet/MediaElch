#include "test/integration/resource_dir.h"

#include <QDebug>
#include <QTextStream>
#include <utility>

static QDir s_resourceDir;
static QDir s_tempDir;

QDir resourceDir()
{
    return s_resourceDir;
}

void setResourceDir(QDir dir)
{
    const bool success = dir.makeAbsolute();
    if (!success) {
        throw std::runtime_error(QString("Directory '%1' not found! Abort.").arg(dir.absolutePath()).toStdString());
    }
    s_resourceDir = std::move(dir);
}

QString getFileContent(QString filename)
{
    QString filepath = resourceDir().absoluteFilePath(filename);
    QFile file(filepath);
    if (!file.exists()) {
        throw std::runtime_error(QString("File '%1' does not exist! Abort.").arg(filepath).toStdString());
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error(QString("File '%1' can't be opened for reading! Abort.").arg(filepath).toStdString());
    }

    QTextStream in(&file);
    return in.readAll();
}

QString getTempFileContent(QString filepath)
{
    QString path = tempDir().filePath(filepath);
    QFile file(path);
    if (!file.exists()) {
        throw std::runtime_error(QString("File %1 does not exist! Abort.").arg(path).toStdString());
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error(QString("File %1 can't be opened for reading! Abort.").arg(path).toStdString());
    }

    QTextStream in(&file);
    return in.readAll();
}

void writeTempFile(QString filepath, QString content)
{
    QStringList fileparts = filepath.split('/');
    QString filename = fileparts.last();
    fileparts.pop_back();

    QString filetemppath = tempDir(fileparts.join('/')).filePath(filename);
    QFile file(filetemppath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error(
            QString("File %1 can't be opened for writing! Abort.").arg(filetemppath).toStdString());
    }
    // Use trailing newline
    if (!content.endsWith('\n')) {
        content.append('\n');
    }
    file.write(content.toUtf8());
}

QDir tempDir(QString subDir)
{
    QDir dir{s_tempDir.path() + '/' + subDir};
    if (dir.exists()) {
        return dir;
    }

    if (!dir.mkpath(".")) {
        qCritical() << "[Test] Can't create sub directory:" << subDir;
        throw std::runtime_error("Can't create sub directory");
    }

    return dir;
}

void setTempDir(QDir dir)
{
    s_tempDir = std::move(dir);
}
