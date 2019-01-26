#pragma once

#include <QHash>
#include <QImage>
#include <QList>
#include <QSize>
#include <QString>

class ImageCache : public QObject
{
    Q_OBJECT
public:
    explicit ImageCache(QObject *parent = nullptr);
    static ImageCache *instance(QObject *parent = nullptr);
    QImage image(QString path, int width, int height, int &origWidth, int &origHeight);
    QSize imageSize(QString path);
    void invalidateImages(QString path);
    void clearCache();

private:
    QString m_cacheDir;
    QHash<QString, QList<int>> m_lastModifiedTimes;
    QImage scaledImage(QImage img, int width, int height);
    int getLastModified(const QString &fileName);
    bool m_forceCache;
};
