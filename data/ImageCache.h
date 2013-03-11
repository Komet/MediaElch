#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include <QImage>
#include <QObject>

class ImageCache : public QObject
{
    Q_OBJECT
public:
    explicit ImageCache(QObject *parent = 0);
    static ImageCache *instance(QObject *parent = 0);
    QImage image(QString path, int width, int height, int &origWidth, int &origHeight);
    QSize imageSize(QString path);
    void invalidateImages(QString path);

private:
    QString m_cacheDir;
    QImage scaledImage(QImage img, int width, int height);
};

#endif // IMAGECACHE_H
