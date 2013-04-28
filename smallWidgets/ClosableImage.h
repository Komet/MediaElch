#ifndef CLOSABLEIMAGE_H
#define CLOSABLEIMAGE_H

#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QMovie>
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
    explicit ClosableImage(QWidget *parent = 0);
    void setMyData(const QVariant &data);
    QVariant myData() const;
    void setImage(const QByteArray &image);
    void setImage(const QString &image);
    QByteArray image();
    int mySize() const;
    void setMySize(const int &size);
    void setShowZoomAndResolution(const bool &show);
    void setFixedSize(const int &scaleTo, const int &size);
    void setMyFixedHeight(const int &height);
    int myFixedHeight() const;
    void setDefaultPixmap(QPixmap pixmap);
    void clear();
    void setClickable(const bool &clickable);
    bool clickable() const;
    void setLoading(const bool &loading);
    void setTitle(const QString &text);
    QString title() const;

signals:
    void sigClose();
    void clicked();

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

private slots:
    void closed();

private:
    QVariant m_myData;
    QByteArray m_image;
    QString m_imagePath;
    QPixmap m_pixmap;
    QPixmap m_defaultPixmap;
    int m_mySize;
    QString m_title;
    QFont m_font;
    QPixmap m_zoomIn;
    bool m_showZoomAndResolution;
    int m_fixedSize;
    int m_scaleTo;
    bool m_clickable;
    bool m_loading;
    int m_fixedHeight;
    QMovie *m_loadingMovie;
    QPointer<QPropertyAnimation> m_anim;
    void updateSize(int imageWidth, int imageHeight);
    QRect imgRect();
    QRect closeRect();
    QRect zoomRect();
    bool confirmDeleteImage();
    void drawTitle(QPainter &p);
};

#endif // CLOSABLEIMAGE_H
