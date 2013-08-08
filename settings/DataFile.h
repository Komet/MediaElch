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
    QString saveFileName(const QString &fileName, int season = -1, bool stacked = false);
    static bool lessThan(DataFile a, DataFile b);
    void setFileName(QString fileName);

    static int dataFileTypeForImageType(int imageType);

private:
    QString m_fileName;
    int m_pos;
    int m_type;
};

#endif // DATAFILE_H
