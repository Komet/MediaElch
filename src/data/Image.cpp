#include "data/Image.h"

#include <QFile>

static int s_idCounter = 0;

Image::Image(QObject* parent) : QObject(parent), m_deletion{false}, m_imageId(++s_idCounter)
{
}

mediaelch::FilePath Image::filePath() const
{
    return m_filePath;
}

void Image::setFilePath(const mediaelch::FilePath& filePath)
{
    if (filePath == m_filePath) {
        return;
    }
    m_filePath = filePath;
    emit fileNameChanged();
}

bool Image::deletion() const
{
    return m_deletion;
}

void Image::setDeletion(bool deletion)
{
    if (deletion == m_deletion) {
        return;
    }
    m_deletion = deletion;
    emit deletionChanged();
}

QByteArray Image::rawData() const
{
    return m_rawData;
}

void Image::setRawData(const QByteArray& rawData)
{
    if (rawData == m_rawData) {
        return;
    }
    m_rawData = rawData;
    emit rawDataChanged();
}

int Image::imageId() const
{
    return m_imageId;
}

void Image::load()
{
    if (!m_rawData.isEmpty()) {
        return;
    }

    QFile f(filePath().toString());
    if (!f.open(QIODevice::ReadOnly)) {
        return;
    }
    m_rawData = f.readAll();
    f.close();
}

void Image::resetIdCounter()
{
    m_imageId = ++s_idCounter;
}
