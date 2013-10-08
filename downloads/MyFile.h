#ifndef MYFILE_H
#define MYFILE_H

#include <QFile>

class MyFile : public QFile
{
    Q_OBJECT
public:
    MyFile(const QString &name);
    bool copy(const QString &newName);
};

#endif // MYFILE_H
