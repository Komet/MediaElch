#include "test/integration/resource_dir.h"

#include <QTextStream>

static QDir s_resourceDir;

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
