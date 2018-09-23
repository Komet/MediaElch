#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QList>
#include <QUrl>
#include <QVariantList>
#include <QWidget>

namespace Ui {
class ImageWidget;
}

class Album;

class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWidget(QWidget *parent = nullptr);
    ~ImageWidget() override;
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
