#pragma once

#include <QUrl>
#include <QVariantList>
#include <QVector>
#include <QWidget>

namespace Ui {
class ImageWidget;
}

class Album;

class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWidget(QWidget* parent = nullptr);
    ~ImageWidget() override;
    void setAlbum(Album* album);

public slots:
    void zoomImage(int artistIndex, int albumIndex, int imageId);
    void cutImage(int artistIndex, int albumIndex, int imageId);
    void imagesDropped(QVariantList urls);
    void setLoading(bool loading);

signals:
    void sigImageDropped(QVector<QUrl> urls);

private:
    Ui::ImageWidget* ui;
};
