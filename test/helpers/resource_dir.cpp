#include "test/helpers/resource_dir.h"

#include <QDebug>
#include <QTextStream>
#include <utility>

static QDir s_resourceDir;
static QDir s_tempDir;

namespace test {

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

QString readResourceFile(const QString& filename)
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

void writeResourceFile(const QString& filename, const QString& content)
{
    QString filepath = resourceDir().absoluteFilePath(filename);
    QFile file(filepath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error(QString("File '%1' can't be opened for writing! Abort.").arg(filepath).toStdString());
    }
    file.write(content.toUtf8());

    // Use trailing newline
    if (!content.endsWith('\n')) {
        file.write(QStringLiteral("\n").toUtf8());
    }
    file.close();
}

QString readTempFile(const QString& filepath)
{
    QString path = makeTempDir().filePath(filepath);
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

void writeTempFile(const QString& filepath, const QString& content)
{
    QStringList fileparts = filepath.split('/');
    QString filename = fileparts.last();
    fileparts.pop_back();

    QString filetemppath = makeTempDir(fileparts.join('/')).filePath(filename);
    QFile file(filetemppath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error(
            QString("File %1 can't be opened for writing! Abort.").arg(filetemppath).toStdString());
    }
    file.write(content.toUtf8());
    if (!content.endsWith('\n')) { // Use trailing newline
        file.write(QStringLiteral("\n").toUtf8());
    }
    file.close();
}

QDir makeTempDir(QString subDir)
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

void setTempRootDir(QDir dir)
{
    s_tempDir = std::move(dir);
}

} // namespace test
