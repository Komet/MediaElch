#ifndef ALBUMIMAGEPROVIDER_H
#define ALBUMIMAGEPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>

class AlbumImageProvider : public QQuickImageProvider
{
public:
    explicit AlbumImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
};

#endif // ALBUMIMAGEPROVIDER_H
