#pragma once

#include "globals/Globals.h"

#include <QLabel>
#include <QMouseEvent>
#include <QMovie>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QWidget>

class ClosableImage : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int mySize READ mySize WRITE setMySize USER true)
    Q_PROPERTY(bool clickable READ clickable WRITE setClickable DESIGNABLE true)
    Q_PROPERTY(int myFixedHeight READ myFixedHeight WRITE setMyFixedHeight DESIGNABLE true)
    Q_PROPERTY(QString title READ title WRITE setTitle DESIGNABLE true)

public:
    explicit ClosableImage(QWidget* parent = nullptr);
    void setMyData(const QVariant& myData);
    QVariant myData() const;
    void setImage(const QByteArray& image);
    void setImage(const QString& image);
    void setImageByPath(const QString& image);
    QByteArray image();
    int mySize() const;
    void setMySize(const int& size);
    void setShowZoomAndResolution(const bool& show);
    void setFixedSize(const int& scaleTo, const int& size);
    void setMyFixedHeight(const int& height);
    int myFixedHeight() const;
    void setDefaultPixmap(QPixmap pixmap);
    void clear();
    void setClickable(const bool& clickable);
    bool clickable() const;
    void setLoading(const bool& loading);
    void setTitle(const QString& text);
    QString title() const;
    void setImageType(ImageType type);
    ImageType imageType() const;

    bool showCapture() const;
    void setShowCapture(bool showCapture);

signals:
    void sigClose();
    void sigCapture();
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

private:
    QVariant m_myData;
    QByteArray m_image;
    QString m_imagePath;
    QPixmap m_pixmap;
    QPixmap m_defaultPixmap;
    int m_mySize = 0;
    QString m_title;
    QFont m_font;
    QPixmap m_zoomIn;
    QPixmap m_capture;
    bool m_showZoomAndResolution = false;
    bool m_showCapture = false;
    int m_fixedSize = 0;
    int m_scaleTo = 0;
    bool m_clickable = false;
    bool m_loading = false;
    int m_fixedHeight = 0;
    QMovie* m_loadingMovie = nullptr;
    QPointer<QPropertyAnimation> m_anim;
    ImageType m_imageType = ImageType::None;
    QPixmap m_emptyPixmap;

    void updateSize(int imageWidth, int imageHeight);
    QRect imgRect();
    QRect closeRect();
    QRect zoomRect();
    QRect captureRect();
    bool confirmDeleteImage();
    void drawTitle(QPainter& p);
};
