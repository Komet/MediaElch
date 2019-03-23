#pragma once

#include "globals/Globals.h"
#include "tvShows/SeasonNumber.h"

#include <QString>

/**
 * @brief The DataFile class
 */
class DataFile
{
public:
    DataFile() = default;
    DataFile(DataFileType type, QString fileName, int pos);
    DataFileType type() const;
    QString fileName() const;
    int pos() const;
    QString saveFileName(const QString& fileName, SeasonNumber season = SeasonNumber::NoSeason, bool stacked = false);
    static bool lessThan(DataFile a, DataFile b);
    void setFileName(QString fileName);

    static DataFileType dataFileTypeForImageType(ImageType imageType);

private:
    QString m_fileName;
    int m_pos = 0;
    DataFileType m_type;
};
