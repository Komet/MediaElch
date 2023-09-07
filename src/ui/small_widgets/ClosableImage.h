#pragma once

#include "globals/Globals.h"
#include "media/ImageCache.h"
#include "media/Path.h"

#include <QLabel>
#include <QMouseEvent>
#include <QMovie>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QWidget>

class ClosableImage final : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int mySize READ mySize WRITE setMySize USER true)
    Q_PROPERTY(bool clickable READ clickable WRITE setClickable DESIGNABLE true)
    Q_PROPERTY(int myFixedHeight READ myFixedHeight WRITE setMyFixedHeight DESIGNABLE true)

public:
    explicit ClosableImage(QWidget* parent = nullptr);
    void setMyData(const QVariant& myData);
    ELCH_NODISCARD QVariant myData() const;
    void setImage(const QByteArray& image);
    void setImageFromPath(const mediaelch::FilePath& image);
    ELCH_NODISCARD QByteArray image();
    int mySize() const;
    void setMySize(const int& size);
    void setShowZoomAndResolution(bool show);
    void setFixedSize(Qt::Orientation scaleTo, int size);
    void setMyFixedHeight(const int& height);
    int myFixedHeight() const;
    void setDefaultPixmap(QPixmap pixmap, bool useDefaultBorder = true);
    void clear();
    void setClickable(bool clickable);
    bool clickable() const;
    void setLoading(bool loading);
    void setImageType(ImageType type);
    ELCH_NODISCARD ImageType imageType() const;

    bool showCapture() const;
    void setShowCapture(bool showCapture);

signals:
    void sigClose();
    void sigCapture(ImageType type);
    void clicked();
    void sigImageDropped(ImageType, QUrl);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void closed();
    void onAsyncImageLoaded();
    void onCloseImageRequest();

private:
    void updateSize(int imageWidth, int imageHeight);
    ELCH_NODISCARD int imageWith() const;
    ELCH_NODISCARD QRect imgRect();
    ELCH_NODISCARD QRect closeRect();
    ELCH_NODISCARD QRect zoomRect();
    ELCH_NODISCARD QRect captureRect();
    ELCH_NODISCARD bool confirmDeleteImage();
    ELCH_NODISCARD bool hasImage() const
    {
        return !m_image.isNull() || (m_imageFromPath != nullptr && !m_imageFromPath->isNull());
    }

private:
    QVariant m_myData;
    QByteArray m_image;
    std::unique_ptr<mediaelch::AsyncImage> m_imageFromPath{nullptr};
    QPixmap m_pixmapForCloseAnimation;
    QPixmap m_defaultPixmap;
    QFont m_font;
    QPixmap m_zoomIn;
    QPixmap m_capture;
    QMovie* m_loadingMovie{nullptr};
    QPointer<QPropertyAnimation> m_anim;
    ImageType m_imageType = ImageType::None;
    int m_mySize{0};
    int m_fixedSize{180};
    int m_fixedHeight{0};
    int m_scaleTo{Qt::Horizontal};
    bool m_clickable{false};
    bool m_isLoading{false};
    bool m_showZoomAndResolution{true};
    bool m_showCapture{false};
};
