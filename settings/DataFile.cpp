#include "DataFile.h"
#include <QDebug>
#include <QFileInfo>

/**
 * @brief DataFile::DataFile
 * @param id Id of this file
 * @param name Display name of this file
 * @param fileName Name of this file
 * @param pos Position
 * @param enabled Enabled
 */
DataFile::DataFile(int id, QString name, QString fileName, int pos, bool enabled)
{
    m_id = id;
    m_name = name;
    m_fileName = fileName;
    m_pos = pos;
    m_enabled = enabled;
}

/**
 * @brief Unique id of this file
 * @return Id
 */
int DataFile::id()
{
    return m_id;
}

/**
 * @brief Display text for the settings page
 * @return Display text
 */
QString DataFile::name()
{
    return m_name;
}

/**
 * @brief Filename
 * @return Filename
 */
QString DataFile::fileName()
{
    return m_fileName;
}

/**
 * @brief Position of this file
 * @return Position
 */
int DataFile::pos() const
{
    return m_pos;
}

/**
 * @brief Status of this file
 * @return File is enabled
 */
bool DataFile::enabled()
{
    return m_enabled;
}

/**
 * @brief Sets the status of this file
 * @param enabled Enabled
 */
void DataFile::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

/**
 * @brief Sets the position
 * @param pos Position
 */
void DataFile::setPos(int pos)
{
    m_pos = pos;
}

QString DataFile::saveFileName(QString fileName)
{
    QFileInfo fi(fileName);
    QString newFileName = m_fileName;
    newFileName.replace("<moviefile>", fi.completeBaseName());
    return newFileName;
}

/**
 * @brief Comparator
 * @param a
 * @param b
 * @return a lessThan b
 */
bool DataFile::lessThan(const DataFile *a, const DataFile *b)
{
    return a->pos() < b->pos();
}
