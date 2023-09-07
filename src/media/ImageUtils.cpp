#include "media/ImageUtils.h"

#include <QBuffer>
#include <QFile>

namespace mediaelch {

void resizeBackdrop(QImage& image, bool& resized)
{
    resized = false;
    if ((image.width() != 1920 || image.height() != 1080) && image.width() > 1915 && image.width() < 1925
        && image.height() > 1075 && image.height() < 1085) {
        image = image.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        resized = true;
    }

    if ((image.width() != 1280 || image.height() != 720) && image.width() > 1275 && image.width() < 1285
        && image.height() > 715 && image.height() < 725) {
        image = image.scaled(1280, 720, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        resized = true;
    }
}

void resizeBackdrop(QByteArray& image)
{
    bool resized = false;
    QImage img = QImage::fromData(image);
    resizeBackdrop(img, resized);
    if (!resized) {
        return;
    }
    QBuffer buffer(&image);
    img.save(&buffer, "jpg", 100);
}


QImage getImage(mediaelch::FilePath path)
{
    QImage img;
    QFile file(path.toString());
    if (file.open(QIODevice::ReadOnly)) {
        img = QImage::fromData(file.readAll());
        file.close();
    }
    return img;
}


QImage scaledImage(const QImage& img, int width, int height)
{
    if (width != 0 && height != 0) {
        return img.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    if (width != 0) {
        return img.scaledToWidth(width, Qt::SmoothTransformation);
    }
    if (height != 0) {
        return img.scaledToHeight(height, Qt::SmoothTransformation);
    }
    return img;
}

QImage scaledImage(const QImage& img, QSize size)
{
    return scaledImage(img, size.width(), size.height());
}

} // namespace mediaelch
