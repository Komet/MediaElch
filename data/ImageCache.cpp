#include "ImageCache.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include "globals/Globals.h"

ImageCache::ImageCache(QObject *parent) :
    QObject(parent)
{
    QString location = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    QDir dir(location);
    if (!dir.exists())
        dir.mkdir(location);
    location = location + "/images";
    dir.setPath(location);
    bool exists = dir.exists();
    if (!exists)
        exists = dir.mkdir(location);
    if (exists)
        m_cacheDir = location;
    qDebug() << "Cache dir" << m_cacheDir;
}

ImageCache *ImageCache::instance(QObject *parent)
{
    static ImageCache *m_instance = 0;
    if (!m_instance)
        m_instance = new ImageCache(parent);
    return m_instance;
}

QImage ImageCache::image(QString path, int width, int height, int &origWidth, int &origHeight)
{
    if (m_cacheDir.isEmpty())
        return scaledImage(QImage(path), width, height);

    QString md5 = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Md5).toHex();
    QString baseName = QString("%1_%2_%3_").arg(md5).arg(width).arg(height);
    int lastMod = QFileInfo(path).lastModified().toTime_t();

    bool update = true;
    QDir dir(m_cacheDir);
    QStringList files = dir.entryList(QStringList() << baseName + "*");
    if (!files.isEmpty()) {
        QString fileName = files.first();
        QStringList parts = fileName.split("_");
        if (parts.count() > 6) {
            origWidth = parts.at(3).toInt();
            origHeight = parts.at(4).toInt();
            if (parts.at(5).toInt() == lastMod)
                update = false;
        }
    }

    if (update) {
        QImage origImg(path);
        origWidth = origImg.width();
        origHeight = origImg.height();
        QImage img = scaledImage(origImg, width, height);
        img.save(m_cacheDir + "/" + QString("%1_%2_%3_%4_%5_%6_.png").arg(md5).arg(width).arg(height).arg(origWidth).arg(origHeight).arg(lastMod), "png", -1);
        return img;
    }

    return QImage(m_cacheDir + "/" + files.first());
}

QImage ImageCache::scaledImage(QImage img, int width, int height)
{
    if (width != 0 && height != 0)
        return img.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    else if (width != 0)
        return img.scaledToWidth(width, Qt::SmoothTransformation);
    else if (height != 0)
        return img.scaledToHeight(height, Qt::SmoothTransformation);
    else
        return img;
}

void ImageCache::invalidateImages(QString path)
{
    if (m_cacheDir.isEmpty())
        return;

    QString md5 = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Md5).toHex();
    QDir dir(m_cacheDir);
    foreach (const QString &file, dir.entryList(QStringList() << md5 + "*")) {
        QFile f(dir.absolutePath() + "/" + file);
        f.remove();
    }
}

QSize ImageCache::imageSize(QString path)
{
    if (m_cacheDir.isEmpty())
        return QImage(path).size();

    QString md5 = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Md5).toHex();
    QString baseName = QString("%1_").arg(md5);
    QDir dir(m_cacheDir);
    QStringList files = dir.entryList(QStringList() << baseName + "*");
    if (files.isEmpty() || files.first().split("_").count() < 7)
        return QImage(path).size();

    QStringList parts = files.first().split("_");
    int lastMod = QFileInfo(path).lastModified().toTime_t();
    if (lastMod != parts.at(5).toInt())
        return QImage(path).size();

    return QSize(parts.at(3).toInt(), parts.at(4).toInt());
}
