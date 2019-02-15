#pragma once

#include <QObject>
#include <QQuickImageProvider>

class AlbumImageProvider : public QQuickImageProvider
{
public:
    explicit AlbumImageProvider();
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);
};
