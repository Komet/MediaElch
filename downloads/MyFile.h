#ifndef MYFILE_H
#define MYFILE_H

#include <QFile>

class MyFile : public QFile
{
    Q_OBJECT
public:
    explicit MyFile(const QString &name);
    bool copy(const QString &newName);
};

#endif // MYFILE_H
