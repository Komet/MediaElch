#include "DataFile.h"
#include <QDebug>
#include <QFileInfo>
#include <QRegExp>
#include <QStringList>
#include "globals/Helper.h"

/**
 * @brief DataFile::DataFile
 * @param type Type of this File (DataFileType)
 * @param fileName Name of this file
 * @param pos Position
 */
DataFile::DataFile(int type, QString fileName, int pos)
{
    m_type = type;
    m_fileName = fileName;
    m_pos = pos;
}

/**
 * @brief Filename
 * @return Filename
 */
QString DataFile::fileName() const
{
    return m_fileName;
}

void DataFile::setFileName(QString fileName)
{
    m_fileName = fileName;
}

/**
 * @brief Position of this file
 * @return
 */
int DataFile::pos() const
{
    return m_pos;
}

/**
 * @brief Constructs a filename to save
 * @param fileName File name
 * @param season Season number
 * @return
 */
QString DataFile::saveFileName(const QString &fileName, int season, bool stacked)
{
    QFileInfo fi(fileName);
    QString newFileName = m_fileName;

    QString baseName = fi.completeBaseName();
    if (stacked) {
        QString f = fileName;
        baseName = Helper::stackedBaseName(f);
    }
    newFileName.replace("<baseFileName>", baseName);

    if (season != -1) {
        if (season == 0) {
            newFileName.replace("<seasonNumber>", "-specials");
        } else {
            QString s = QString("%1").arg(season);
            if (season < 10)
                s.prepend("0");
            newFileName.replace("<seasonNumber>", s);
        }
    }

    return newFileName;
}

/**
 * @brief Returns the type of this file
 * @return Type
 */
int DataFile::type() const
{
    return m_type;
}

/**
 * @brief Comparator
 * @param a
 * @param b
 * @return a lessThan b
 */
bool DataFile::lessThan(DataFile a, DataFile b)
{
    return a.pos() < b.pos();
}
