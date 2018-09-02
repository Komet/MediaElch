#ifndef DATAFILE_H
#define DATAFILE_H

#include "globals/Globals.h"

#include <QString>

/**
 * @brief The DataFile class
 */
class DataFile
{
public:
    DataFile(DataFileType type, QString fileName, int pos);
    DataFileType type() const;
    QString fileName() const;
    int pos() const;
    QString saveFileName(const QString &fileName, int season = -1, bool stacked = false);
    static bool lessThan(DataFile a, DataFile b);
    void setFileName(QString fileName);

    static DataFileType dataFileTypeForImageType(ImageType imageType);

private:
    QString m_fileName;
    int m_pos;
    DataFileType m_type;
};

#endif // DATAFILE_H
