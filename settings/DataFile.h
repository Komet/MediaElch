#ifndef DATAFILE_H
#define DATAFILE_H

#include <QString>

/**
 * @brief The DataFile class
 */
class DataFile
{
public:
    DataFile(int type, QString fileName, int pos);
    int type() const;
    QString fileName() const;
    int pos() const;
    QString saveFileName(QString fileName, int season = -1, bool stacked = false);
    static bool lessThan(DataFile a, DataFile b);

private:
    QString m_fileName;
    int m_pos;
    int m_type;
    QString stackedBaseName(QString fileName);
};

#endif // DATAFILE_H
