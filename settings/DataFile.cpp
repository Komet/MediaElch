#include "DataFile.h"
#include <QDebug>
#include <QFileInfo>
#include <QRegExp>
#include <QStringList>

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
QString DataFile::saveFileName(QString fileName, int season, bool stacked)
{
    QFileInfo fi(fileName);
    QString newFileName = m_fileName;

    QString baseName = fi.completeBaseName();
    if (stacked)
        baseName = stackedBaseName(fileName);
    newFileName.replace("<baseFileName>", baseName);

    if (season != -1) {
        QString s = QString("%1").arg(season);
        if (season < 10)
            s.prepend("0");
        newFileName.replace("<seasonNumber>", s);
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

QString DataFile::stackedBaseName(QString fileName)
{
    QString baseName = fileName;
    QRegExp rx1a("(.*)([ _\\.-]*(?:cd|dvd|p(?:ar)?t|dis[ck])[ _\\.-]*[0-9]+)(.*)(\\.[^.]+)$", Qt::CaseInsensitive);
    QRegExp rx1b("(.*)([ _\\.-]+)$");
    QRegExp rx2a("(.*)([ _\\.-]*(?:cd|dvd|p(?:ar)?t|dis[ck])[ _.-]*[a-d])(.*)(\\.[^.]+)$", Qt::CaseInsensitive);
    QRegExp rx2b("(.*)([ _\\.-]+)$");

    QList<QList<QRegExp> > regex;
    regex << (QList<QRegExp>() << rx1a << rx1b);
    regex << (QList<QRegExp>() << rx2a << rx2b);

    foreach (QList<QRegExp> rx, regex) {
        if (rx.at(0).indexIn(fileName) != -1) {
            QString title = rx.at(0).cap(1);
            QString volume = rx.at(0).cap(2);
            /*QString ignore = rx.at(0).cap(3);
            QString extension = rx.at(0).cap(4);*/
            while (rx.at(1).indexIn(title) != -1) {
                title = rx.at(1).capturedTexts().at(1);
                volume.prepend(rx.at(1).capturedTexts().at(2));
            }
            baseName = title;
        }
    }

    return baseName;
}
