#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include "image/ImageModel.h"
#include "image/ImageProxyModel.h"
#include "music/Album.h"

namespace Ui {
class ImageWidget;
}

class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWidget(QWidget *parent = 0);
    ~ImageWidget();
    void setAlbum(Album *album);

public slots:
    void zoomImage(int artistIndex, int albumIndex, int imageId);
    void cutImage(int artistIndex, int albumIndex, int imageId);
    void imagesDropped(QVariantList urls);
    void setLoading(bool loading);

signals:
    void sigImageDropped(QList<QUrl> urls);

private:
    Ui::ImageWidget *ui;
};

#endif // IMAGEWIDGET_H
