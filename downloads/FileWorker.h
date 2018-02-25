#ifndef FILEWORKER_H
#define FILEWORKER_H

#include <QMap>
#include <QObject>
#include "downloads/MyFile.h"

class FileWorker : public QObject
{
    Q_OBJECT
public:
    explicit FileWorker(QObject *parent = nullptr);
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

#endif // FILEWORKER_H
