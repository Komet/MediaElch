#pragma once

#include "downloads/MyFile.h"

#include <QMap>
#include <QObject>

class FileWorker : public QObject
{
    Q_OBJECT
public:
    explicit FileWorker(QObject* parent = nullptr);
    void setFiles(QMap<QString, QString> files);
    QMap<QString, QString> files();

public slots:
    void copyFiles();
    void moveFiles();

signals:
    void sigFinished();

private:
    QMap<QString, QString> m_files;
};
