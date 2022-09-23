#pragma once
#include <QFile>

class MyFile : public QFile
{
    Q_OBJECT
public:
    explicit MyFile(const QString& name);
    bool copy(const QString& newName);
};
