#pragma once

#include "globals/Globals.h"
#include "ui/small_widgets/ClosableImage.h"

#include <QResizeEvent>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>

class Image;

class ImageGallery : public QWidget
{
    Q_OBJECT
public:
    explicit ImageGallery(QWidget* parent = nullptr);
    void clear();
    void setImages(QVector<ExtraFanart> images);
    void setImages(QList<Image*> images);
    void addImage(const QByteArray& img, const QString& url = QString());
    void setLoading(const bool& loading);
    void setAlignment(const int& alignment);
    void setShowZoomAndResolution(const bool& show);

signals:
    void sigRemoveImage(QByteArray);
    void sigRemoveImage(QString);
    void sigImagesDropped(QVector<QUrl> imageUrls);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onCloseImage();
    void onVerticalScrollBarMoved(const int& value);
    void onHorizontalScrollBarMoved(const int& value);
    void onButtonLeft();
    void onButtonRight();
    void onButtonTop();
    void onButtonBottom();
    void onVerticalScrollBarRangeChanged(int min, int max);
    void onHorizontalScrollBarRangeChanged(int min, int max);
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QLabel* m_loadingLabel;
    QVector<ClosableImage*> m_imageLabels;
    QWidget* m_imagesWidget;
    QScrollArea* m_scrollArea;
    QToolButton* m_buttonLeft;
    QToolButton* m_buttonRight;
    QToolButton* m_buttonTop;
    QToolButton* m_buttonBottom;
    int m_imageWidth;
    int m_imageHeight;
    int m_horizontalSpace;
    int m_verticalSpace;
    int m_alignment;
    int m_scrollValue;
    bool m_showZoomAndResolution;

    void positionImages();
};
