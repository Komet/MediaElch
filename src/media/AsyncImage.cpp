#include "media/AsyncImage.h"

#include "log/Log.h"
#include "media/ImageUtils.h"

#include <QFuture>
#include <QtConcurrent>
#include <chrono>

namespace {

std::chrono::seconds getLastModified(const mediaelch::FilePath& fileName)
{
    using namespace std::chrono_literals;
#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
    return std::chrono::seconds{QFileInfo(fileName.toString()).lastModified().toMSecsSinceEpoch() / 1000ll};
#else
    return std::chrono::seconds{QFileInfo(fileName.toString()).lastModified().toSecsSinceEpoch()};
#endif
}

bool isLastModifiedTheSame(const mediaelch::FilePath& fileName, std::chrono::seconds cachedSecSinceEpoch)
{
    // +/- 10s difference allowed
    using namespace std::chrono_literals;
    const auto actualLastModified = getLastModified(fileName);
    return cachedSecSinceEpoch <= actualLastModified + 10s && //
           cachedSecSinceEpoch >= actualLastModified - 10s;
}

mediaelch::impl::ResizedImage readImageSync(mediaelch::FilePath path)
{
    mediaelch::impl::ResizedImage img;
    QFile file(path.toString());
    if (file.open(QIODevice::ReadOnly)) {
        img.image = QImage::fromData(file.readAll());
        file.close();
        img.originalSize = img.image.size();
        img.resizedSize = img.originalSize;
    } else {
        qCWarning(generic) << "[AsyncImage] Could not load image from:" << path.toNativePathString();
    }
    return img;
}

mediaelch::impl::ResizedImage readAndResizeImageSync(mediaelch::FilePath path, QSize targetSize)
{
    mediaelch::impl::ResizedImage img = readImageSync(std::move(path));
    img.resizedSize = targetSize;
    if (!img.image.isNull()) {
        img.image = mediaelch::scaledImage(img.image, targetSize);
    }
    return img;
}

mediaelch::impl::ResizedImage
readAndResizeAndCacheImageSync(mediaelch::DirectoryPath cacheDir, mediaelch::FilePath imgPath, QSize targetSize)
{
    if (!cacheDir.isValid()) {
        return readAndResizeImageSync(imgPath, targetSize);
    }

    QString hash = mediaelch::pathHash(imgPath);
    QString nameFilter = QStringLiteral("%1_%2_%3_*") //
                             .arg(hash)
                             .arg(targetSize.width())
                             .arg(targetSize.height());

    QDir dir = cacheDir.dir();
    QStringList files = dir.entryList({nameFilter});

    // Check the cache directory: If the first matched file is usable, i.e. has a proper
    // modification date and filename format, use it.
    QSize cachedOriginalSize;
    bool cacheNeedsUpdate = true;
    if (!files.isEmpty()) {
        const QString fileName = files.first();
        const QStringList parts = fileName.split("_");
        if (parts.count() > 5) {
            cachedOriginalSize.setWidth(parts.at(3).toInt());
            cachedOriginalSize.setHeight(parts.at(4).toInt());
        }
        bool ok = false;
        auto lastModified = std::chrono::seconds{parts.at(5).toLongLong(&ok, 10)};
        cacheNeedsUpdate = !(ok && lastModified.count() > 0 && isLastModifiedTheSame(imgPath, lastModified));
    }

    if (cacheNeedsUpdate) {
        // It could happen that the to-be-cached image appears lexicographically after the other files,
        // e.g. due to a different/changed size/date.  In that case, the first, outdated cache would
        // always be loaded and result in `cacheNeedsUpdate` always being true.
        for (const QString& file : files) {
            QFile::remove(cacheDir.filePath(file));
        }

        auto origImg = readAndResizeImageSync(imgPath, targetSize);
        QString cacheFileName = QStringLiteral("%1_%2_%3_%4_%5_%6_.png")
                                    .arg(hash)
                                    .arg(targetSize.width())
                                    .arg(targetSize.height())
                                    .arg(origImg.originalSize.width())
                                    .arg(origImg.originalSize.height())
                                    .arg(getLastModified(imgPath).count());
        origImg.image.save(cacheDir.filePath(cacheFileName), "png", -1);
        return origImg;

    } else {
        mediaelch::impl::ResizedImage img;
        QString cachedImagePath = cacheDir.filePath(files.first());

        QFile file(cachedImagePath);
        if (file.open(QIODevice::ReadOnly)) {
            img.image = QImage::fromData(file.readAll());
            file.close();
            img.originalSize = cachedOriginalSize;
            img.resizedSize = targetSize;

            img.image = mediaelch::scaledImage(img.image, targetSize);
        } else {
            qCWarning(generic) << "[Image] Couldn't load cached image of" << imgPath.toNativePathString() << "at"
                               << QDir::toNativeSeparators(cachedImagePath);
        }
        return img;
    }
}

} // namespace

namespace mediaelch {

std::unique_ptr<AsyncImage> AsyncImage::fromPath(mediaelch::FilePath path)
{
    // std::make_unique() can't access private constructor; and we are neither exception safe
    // to begin with nor do we try to reduce allocations, so no big deal.
    auto img = std::unique_ptr<AsyncImage>(new AsyncImage());
    img->m_path = path;
    connect(&img->m_watcher, &QFutureWatcher<QImage>::finished, img.get(), &AsyncImage::onLoaded);
    img->m_watcher.setFuture(QtConcurrent::run(readImageSync, std::move(path)));
    return img;
}

std::unique_ptr<AsyncImage>
AsyncImage::fromPathCached(mediaelch::DirectoryPath cacheDir, mediaelch::FilePath path, QSize targetSize)
{
    // std::make_unique() can't access private constructor; and we are neither exception safe
    // to begin with nor do we try to reduce allocations, so no big deal.
    auto img = std::unique_ptr<AsyncImage>(new AsyncImage());
    img->m_path = path;
    connect(&img->m_watcher, &QFutureWatcher<QImage>::finished, img.get(), &AsyncImage::onLoaded);
    img->m_watcher.setFuture(
        QtConcurrent::run(readAndResizeAndCacheImageSync, std::move(cacheDir), std::move(path), std::move(targetSize)));
    return img;
}

void AsyncImage::onLoaded()
{
    m_img = m_watcher.result();
    // m_watcher.setFuture({}); // TODO
    emit sigLoaded();
}

} // namespace mediaelch
