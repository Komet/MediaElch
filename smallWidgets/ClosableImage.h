#ifndef CLOSABLEIMAGE_H
#define CLOSABLEIMAGE_H

#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWidget>

class ClosableImage : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int mySize READ mySize WRITE setMySize USER true)

public:
    explicit ClosableImage(QWidget *parent = 0);
    void setMyData(const QVariant &data);
    QVariant myData() const;
    void setImage(const QImage &image);
    QImage image() const;
    int mySize() const;
    void setMySize(const int &size);
    void setShowZoomAndResolution(const bool &show);
    void setFixedSize(const int &scaleTo, const int &size);

signals:
    void sigClose();
    void sigZoom(QImage);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

private:
    QVariant m_myData;
    QImage m_image;
    QPixmap m_pixmap;
    int m_mySize;
    QFont m_font;
    QPixmap m_zoomIn;
    bool m_showZoomAndResolution;
    int m_fixedSize;
    int m_scaleTo;
};

#endif // CLOSABLEIMAGE_H
