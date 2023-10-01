#pragma once

#include "media/Path.h"

#include <QHash>
#include <QImage>
#include <QSize>
#include <QString>
#include <QVector>

class ImageCache : public QObject
{
    Q_OBJECT
public:
    explicit ImageCache(QObject* parent = nullptr);
    static ImageCache* instance(QObject* parent = nullptr);
    QImage image(mediaelch::FilePath path, int width, int height, int& origWidth, int& origHeight);
    QSize imageSize(mediaelch::FilePath path);
    void invalidateImages(mediaelch::FilePath path);
    void clearCache();

private:
    mediaelch::DirectoryPath m_cacheDir;
    QHash<mediaelch::FilePath, QVector<qint64>> m_lastModifiedTimes;
    QImage scaledImage(QImage img, int width, int height);
    qint64 getLastModified(const mediaelch::FilePath& fileName);
};
