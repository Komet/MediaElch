#include "ImageCache.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "settings/Settings.h"

ImageCache::ImageCache(QObject* parent) : QObject(parent)
{
    mediaelch::DirectoryPath location = Settings::instance()->imageCacheDir();
    QDir dir(location.dir());
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
    qDebug() << "[ImageCache] Using cache dir:" << m_cacheDir;

    m_forceCache = Settings::instance()->advanced()->forceCache();
}

ImageCache* ImageCache::instance(QObject* parent)
{
    static auto* s_instance = new ImageCache(parent);
    return s_instance;
}

QImage ImageCache::image(mediaelch::FilePath path, int width, int height, int& origWidth, int& origHeight)
{
    if (!m_cacheDir.isValid()) {
        return scaledImage(helper::getImage(path), width, height);
    }

    QString md5 = QCryptographicHash::hash(path.toString().toUtf8(), QCryptographicHash::Md5).toHex();
    QString baseName = QString("%1_%2_%3_").arg(md5).arg(width).arg(height);

    bool update = true;
    QDir dir = m_cacheDir.dir();
    QStringList files = dir.entryList(QStringList() << baseName + "*");
    if (!files.isEmpty()) {
        QString fileName = files.first();
        QStringList parts = fileName.split("_");
        if (parts.count() > 6) {
            origWidth = parts.at(3).toInt();
            origHeight = parts.at(4).toInt();
            if (m_forceCache || (parts.at(5).toInt() > 0 && parts.at(5).toUInt() == getLastModified(path))) {
                update = false;
            }
        }
    }

    if (update) {
        QImage origImg = helper::getImage(path);
        origWidth = origImg.width();
        origHeight = origImg.height();
        QImage img = scaledImage(origImg, width, height);
        QString fileName = QString("%1_%2_%3_%4_%5_%6_.png")
                               .arg(md5)
                               .arg(width)
                               .arg(height)
                               .arg(origWidth)
                               .arg(origHeight)
                               .arg(getLastModified(path));
        img.save(m_cacheDir.filePath(fileName), "png", -1);
        return img;
    }

    return helper::getImage(mediaelch::FilePath(m_cacheDir.filePath(files.first())));
}

QImage ImageCache::scaledImage(QImage img, int width, int height)
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

void ImageCache::invalidateImages(mediaelch::FilePath path)
{
    if (!m_cacheDir.isValid()) {
        return;
    }

    QString md5 = QCryptographicHash::hash(path.toString().toUtf8(), QCryptographicHash::Md5).toHex();
    QDir dir = m_cacheDir.dir();
    const QStringList entries = dir.entryList(QStringList() << md5 + "*");
    for (const QString& file : entries) {
        QFile f(dir.absolutePath() + "/" + file);
        f.remove();
    }
}

QSize ImageCache::imageSize(mediaelch::FilePath path)
{
    if (!m_cacheDir.isValid()) {
        return helper::getImage(path).size();
    }

    QString md5 = QCryptographicHash::hash(path.toString().toUtf8(), QCryptographicHash::Md5).toHex();
    QString baseName = QString("%1_").arg(md5);
    QDir dir = m_cacheDir.dir();
    QStringList files = dir.entryList(QStringList() << baseName + "*");
    if (files.isEmpty() || files.first().split("_").count() < 7) {
        return helper::getImage(path).size();
    }

    QStringList parts = files.first().split("_");
    if (!m_forceCache && parts.at(5).toInt() > 0 && getLastModified(path) != parts.at(5).toUInt()) {
        return helper::getImage(path).size();
    }

    return {parts.at(3).toInt(), parts.at(4).toInt()};
}

unsigned ImageCache::getLastModified(const mediaelch::FilePath& fileName)
{
    unsigned now = QDateTime::currentDateTime().toTime_t();
    if (!m_lastModifiedTimes.contains(fileName) || m_lastModifiedTimes.value(fileName).first() < now - 10) {
        unsigned lastMod = QFileInfo(fileName.toString()).lastModified().toTime_t();
        m_lastModifiedTimes.insert(fileName, {now, lastMod});
    }
    return m_lastModifiedTimes.value(fileName).last();
}

void ImageCache::clearCache()
{
    if (!m_cacheDir.isValid() || !Settings::instance()->advanced()->forceCache()) {
        return;
    }
    const auto entries = m_cacheDir.dir().entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& file : entries) {
        QFile(file.absoluteFilePath()).remove();
    }
}
