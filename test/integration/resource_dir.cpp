#include "test/integration/resource_dir.h"

#include <QTextStream>

static QDir s_resourceDir;
static QDir s_tempDir;

QDir resourceDir()
{
    return s_resourceDir;
}

void setResourceDir(QDir dir)
{
    s_resourceDir = dir;
}

QString getFileContent(QString filename)
{
    QString filepath = resourceDir().filePath(filename);
    QFile file(filepath);
    if (!file.exists()) {
        throw std::runtime_error(QString("File %1 does not exist! Abort.").arg(filepath).toStdString());
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error(QString("File %1 cab't be opened for reading! Abort.").arg(filepath).toStdString());
    }

    QTextStream in(&file);
    return in.readAll();
}

void writeTempFile(QString filename, QString content)
{
    QString filepath = tempDir().filePath(filename);
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error(QString("File %1 cab't be opened for writing! Abort.").arg(filepath).toStdString());
    }
    file.write(content.toUtf8());
}

QDir tempDir()
{
    return s_tempDir;
}

void setTempDir(QDir dir)
{
    s_tempDir = dir;
}
