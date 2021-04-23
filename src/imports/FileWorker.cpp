#include "FileWorker.h"

#include "imports/MyFile.h"
#include "log/Log.h"

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
