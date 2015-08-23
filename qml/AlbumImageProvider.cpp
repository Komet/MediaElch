#include "AlbumImageProvider.h"

#include "globals/Manager.h"
#include <QDebug>

AlbumImageProvider::AlbumImageProvider() : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage AlbumImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QStringList parts = id.split("/");

    if (parts.count() == 4 && parts.at(0) == "booklet") {
        int artistNum = parts.at(1).toInt();
        int albumNum = parts.at(2).toInt();
        int imageId = parts.at(3).toInt();

        if (Manager::instance()->musicModel()->artists().count() <= artistNum)
            return QImage();

        Artist *artist = Manager::instance()->musicModel()->artists().at(artistNum);

        if (artist->albums().count() <= albumNum)
            return QImage();

        Album *album = artist->albums().at(albumNum);

        int row = album->bookletModel()->rowById(imageId);
        QImage img = QImage::fromData(album->bookletModel()->data(album->bookletModel()->index(row, 0), Qt::UserRole+4).toByteArray());

        if (size)
            *size = QSize(img.width(), img.height());
        if (requestedSize.width() > 0 || requestedSize.height() > 0)
            return img.scaled(requestedSize.width(), requestedSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        return img;
    }

    return QImage();
}
