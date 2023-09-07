#include "media/ImageCache.h"

#include "log/Log.h"
#include "media/ImageUtils.h"
#include "settings/Settings.h"

#include <QDir>
#include <QFileInfo>
#include <QtConcurrent>

ImageCache::ImageCache(QObject* parent) : QObject(parent)
{
    mediaelch::DirectoryPath location = Settings::instance()->imageCacheDir();
    QDir dir = location.dir();
    if (!dir.exists()) {
        dir.mkdir(location.toString());
    }
    location = location.subDir("images");
    dir.setPath(location.toString());
    bool exists = dir.exists();
    if (!exists) {
        exists = dir.mkdir(location.toString());
    }
    if (exists) {
        m_cacheDir = location;
    }
    qCDebug(generic) << "[ImageCache] Using cache directory:" << m_cacheDir;
}

ImageCache* ImageCache::instance()
{
    static ImageCache s_instance;
    return &s_instance;
}

void ImageCache::invalidateImages(const mediaelch::FilePath& path)
{
    if (!m_cacheDir.isValid()) {
        return;
    }

    QDir dir = m_cacheDir.dir();
    const QStringList entries = dir.entryList({mediaelch::pathHash(path) + "*"});
    for (const QString& file : entries) {
        QFile f(dir.absolutePath() + "/" + file);
        f.remove();
    }
}

void ImageCache::clearCache()
{
    if (!m_cacheDir.isValid()) {
        return;
    }
    const auto entries = m_cacheDir.dir().entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& file : entries) {
        QFile(file.absoluteFilePath()).remove();
    }
}

std::unique_ptr<mediaelch::AsyncImage> ImageCache::loadImageAsync(const mediaelch::FilePath& path, QSize targetSize)
{
    return mediaelch::AsyncImage::fromPathCached(m_cacheDir, path, targetSize);
}
