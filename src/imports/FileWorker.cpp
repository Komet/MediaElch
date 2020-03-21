#include "FileWorker.h"

#include <QDebug>
#include <QFile>

FileWorker::FileWorker(QObject* parent) : QObject(parent)
{
}

void FileWorker::setFiles(QMap<QString, QString> files)
{
    m_files = files;
}

QMap<QString, QString> FileWorker::files()
{
    return m_files;
}

void FileWorker::copyFiles()
{
    QMapIterator<QString, QString> it(files());
    while (it.hasNext()) {
        it.next();
        MyFile file(it.key());
        file.copy(it.value());
    }
    emit sigFinished();
}

void FileWorker::moveFiles()
{
    QMapIterator<QString, QString> it(files());
    while (it.hasNext()) {
        it.next();
        MyFile file(it.key());
        file.rename(it.value());
    }
    emit sigFinished();
}
