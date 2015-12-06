#include "ImageCache.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "settings/Settings.h"

ImageCache::ImageCache(QObject *parent) :
    QObject(parent)
{
    QString location = Settings::instance()->imageCacheDir();
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

    m_forceCache = Settings::instance()->advanced()->forceCache();
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
        return scaledImage(Helper::instance()->getImage(path), width, height);

    QString md5 = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Md5).toHex();
    QString baseName = QString("%1_%2_%3_").arg(md5).arg(width).arg(height);

    bool update = true;
    QDir dir(m_cacheDir);
    QStringList files = dir.entryList(QStringList() << baseName + "*");
    if (!files.isEmpty()) {
        QString fileName = files.first();
        QStringList parts = fileName.split("_");
        if (parts.count() > 6) {
            origWidth = parts.at(3).toInt();
            origHeight = parts.at(4).toInt();
            if (m_forceCache || parts.at(5).toInt() == getLastModified(path))
                update = false;
        }
    }

    if (update) {
        QImage origImg = Helper::instance()->getImage(path);
        origWidth = origImg.width();
        origHeight = origImg.height();
        QImage img = scaledImage(origImg, width, height);
        img.save(m_cacheDir + "/" + QString("%1_%2_%3_%4_%5_%6_.png").arg(md5).arg(width).arg(height).arg(origWidth).arg(origHeight).arg(getLastModified(path)), "png", -1);
        return img;
    }

    return Helper::instance()->getImage(m_cacheDir + "/" + files.first());
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
        return Helper::instance()->getImage(path).size();

    QString md5 = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Md5).toHex();
    QString baseName = QString("%1_").arg(md5);
    QDir dir(m_cacheDir);
    QStringList files = dir.entryList(QStringList() << baseName + "*");
    if (files.isEmpty() || files.first().split("_").count() < 7)
        return Helper::instance()->getImage(path).size();

    QStringList parts = files.first().split("_");
    if (!m_forceCache && getLastModified(path) != parts.at(5).toInt())
        return Helper::instance()->getImage(path).size();

    return QSize(parts.at(3).toInt(), parts.at(4).toInt());
}

int ImageCache::getLastModified(const QString &fileName)
{
    int now = QDateTime::currentDateTime().toTime_t();
    if (!m_lastModifiedTimes.contains(fileName) || m_lastModifiedTimes.value(fileName).first() < now-10) {
        int lastMod = QFileInfo(fileName).lastModified().toTime_t();
        m_lastModifiedTimes.insert(fileName, QList<int>() << now << lastMod);
    }
    return m_lastModifiedTimes.value(fileName).last();
}

void ImageCache::clearCache()
{
    if (m_cacheDir.isEmpty() || !Settings::instance()->advanced()->forceCache())
        return;
    foreach (const QFileInfo &file, QDir(m_cacheDir).entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
        QFile(file.absoluteFilePath()).remove();
}
