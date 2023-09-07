#pragma once

#include "media/AsyncImage.h"
#include "media/Path.h"

#include <QSize>
#include <QString>

class ImageCache : public QObject
{
    Q_OBJECT
public:
    explicit ImageCache(QObject* parent = nullptr);
    static ImageCache* instance();

    void invalidateImages(const mediaelch::FilePath& path);
    void clearCache();

    /// \brief Asynchronously load the given image and resize it. Caches the resized result on disk.
    /// \details
    ///   If either width or height of the targetSize is 0, the aspect ratio is respected and the
    ///   not-null value used. See mediaelch::scaledImage() for details.  The resized result
    ///   is cached on disk and loaded instead of the original on subsequent requests.
    ///   No in-memory caching is performed.
    std::unique_ptr<mediaelch::AsyncImage> loadImageAsync(const mediaelch::FilePath& path, QSize targetSize);

private:
    mediaelch::DirectoryPath m_cacheDir;
};
