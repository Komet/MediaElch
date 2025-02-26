#include "MyFile.h"

#include "log/Log.h"

#include <QFileInfo>

MyFile::MyFile(const QString& name) : QFile(name)
{
}

bool MyFile::copy(const QString& newName)
{
    if (fileName().isEmpty()) {
        qCWarning(generic) << "QFile::copy: Empty or null file name";
        return false;
    }
    if (QFile(newName).exists()) {
        return false;
    }
    unsetError();
    close();
    if (error() == QFile::NoError) {
        bool error = false;
        if (!open(QFile::ReadOnly)) {
            error = true;
        } else {
            QFile out(newName);
            if (!out.open(QIODevice::ReadWrite)) {
                error = true;
            }
            if (error) {
                out.close();
                close();
            } else {
                char block[4096]; // NOLINT: This file needs a refactoring nonetheless; allow c array for now
                qint64 totalRead = 0;
                while (!atEnd()) {
                    qint64 in = read(block, sizeof(block));
                    if (in <= 0) {
                        break;
                    }
                    totalRead += in;
                    if (in != out.write(block, in)) {
                        close();
                        error = true;
                        break;
                    }
                }

                if (totalRead != size()) {
                    // Unable to read from the source. The error string is
                    // already set from read().
                    error = true;
                }
                if (error) {
                    out.remove();
                }
            }
        }
        if (!error) {
            QFile::setPermissions(newName, permissions());
            close();
            unsetError();
            return true;
        }
    }
    return false;
}
