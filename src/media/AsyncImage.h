#pragma once

#include "media/Path.h"

#include <QFutureWatcher>
#include <QImage>
#include <QObject>
#include <QSize>

namespace mediaelch {

namespace impl {

struct ResizedImage
{
    QImage image;
    QSize originalSize;
    QSize resizedSize;
};

} // namespace impl

/// \brief Asynchronously load a QImage, resize it to a preferred size, and cache it on disk.
/// \details
///   You can use AsyncCachedImage to load an image asynchronously and get notified
///   when to image is loaded.  This class must only be used from a single thread,
///   i.e. its accessor-functions are not thread safe.
///
///   Caches the image if it is requested in a certain size, as the resize operation
///   can heavily decrease the file size and subsequent loads can be faster (due to
///   loading the cached image instead of the original).
class AsyncImage : public QObject
{
    Q_OBJECT

public:
    static std::unique_ptr<AsyncImage> fromPath(mediaelch::FilePath path);
    static std::unique_ptr<AsyncImage>
    fromPathCached(mediaelch::DirectoryPath cacheDir, mediaelch::FilePath path, QSize targetSize);

    /// \brief Returns a reference to the image, possibly 0.
    ELCH_NODISCARD QImage& image() { return m_img.image; }
    ELCH_NODISCARD QSize originalSize() const { return m_img.originalSize; }
    ELCH_NODISCARD bool isNull() const { return m_img.image.isNull(); }
    ELCH_NODISCARD bool isLoaded() const { return m_ready; }
    ELCH_NODISCARD const mediaelch::FilePath& path() const { return m_path; }

signals:
    void sigLoaded();

private:
    explicit AsyncImage(QObject* parent = nullptr) : QObject(parent) {}

private slots:
    /// Fills the image variable.  Must always be called on the same thread
    /// as the owner of this AsyncCachedImage.
    void onLoaded();

private:
    QFutureWatcher<impl::ResizedImage> m_watcher;
    impl::ResizedImage m_img;
    mediaelch::FilePath m_path;
    bool m_ready{false};
};

} // namespace mediaelch
