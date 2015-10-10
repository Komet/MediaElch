#include "Image.h"

#include <QFile>

int Image::m_idCounter = 0;

Image::Image(QObject *parent) : QObject(parent)
{
    m_imageId = ++Image::m_idCounter;
    m_deletion = false;
}

QString Image::fileName() const
{
    return m_fileName;
}

void Image::setFileName(const QString &fileName)
{
    if (fileName == m_fileName)
        return;
    m_fileName = fileName;
    emit fileNameChanged();
}

bool Image::deletion() const
{
    return m_deletion;
}

void Image::setDeletion(bool deletion)
{
    if (deletion == m_deletion)
        return;
    m_deletion = deletion;
    emit deletionChanged();
}

QByteArray Image::rawData() const
{
    return m_rawData;
}

void Image::setRawData(const QByteArray &rawData)
{
    if (rawData == m_rawData)
        return;
    m_rawData = rawData;
    emit rawDataChanged();
}

int Image::imageId()
{
    return m_imageId;
}

void Image::load()
{
    if (!m_rawData.isEmpty())
        return;

    QFile f(fileName());
    if (!f.open(QIODevice::ReadOnly))
        return;
    m_rawData = f.readAll();
    f.close();
}

void Image::resetIdCounter()
{
    m_imageId = ++Image::m_idCounter;
}
