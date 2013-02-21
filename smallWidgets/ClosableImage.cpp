#include "ClosableImage.h"

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPropertyAnimation>
#include <QToolTip>
#include <qmath.h>

ClosableImage::ClosableImage(QWidget *parent) :
    QLabel(parent)
{
    setMouseTracking(true);
    m_showZoomAndResolution = true;
    m_fixedSize = Qt::Horizontal;
    m_font = QApplication::font();
    #ifdef Q_OS_WIN32
    m_font.setPointSize(m_font.pointSize()-1);
    #else
    m_font.setPointSize(m_font.pointSize()-2);
    #endif

    m_zoomIn = QPixmap(":/img/zoom_in.png");
    QPainter p;
    p.begin(&m_zoomIn);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(m_zoomIn.rect(), QColor(0, 0, 0, 150));
    p.end();
    m_zoomIn = m_zoomIn.scaledToWidth(16, Qt::SmoothTransformation);
}

void ClosableImage::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() != Qt::LeftButton)
        return;
    QRect closeRect(width()-25, 0, 24, 24);
    QRect zoomRect(0, height()-16, 16, 16);
    if (closeRect.contains(ev->pos()) && m_pixmap.isNull()) {
        m_pixmap = QPixmap::grabWidget(this);
        QPropertyAnimation *anim = new QPropertyAnimation(this);
        anim->setEasingCurve(QEasingCurve::InQuad);
        anim->setTargetObject(this);
        anim->setStartValue(0);
        anim->setEndValue(width()/2);
        anim->setPropertyName("mySize");
        anim->setDuration(400);
        anim->start(QPropertyAnimation::DeleteWhenStopped);
        connect(anim, SIGNAL(finished()), this, SIGNAL(sigClose()));
    } else if (m_showZoomAndResolution && zoomRect.contains(ev->pos()) && m_pixmap.isNull()) {
        emit sigZoom(QImage::fromData(m_image));
    }
}

void ClosableImage::mouseMoveEvent(QMouseEvent *ev)
{
    if ((ev->pos().x() > width()-25 && ev->pos().y() < 25) ||
            (m_showZoomAndResolution && ev->pos().x() < 16 && ev->pos().y() > height()-16)) {
        setCursor(Qt::PointingHandCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void ClosableImage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    if (!m_pixmap.isNull()) {
        int h = height()*(width()-2*m_mySize)/width();
        p.drawPixmap(m_mySize, (height()-h)/2, m_pixmap.scaledToWidth(width()-2*m_mySize));
        return;
    }
    QImage img = QImage::fromData(m_image);
    QRect r = rect();
    p.drawImage(0, 7, img.scaledToWidth(width()-9, Qt::SmoothTransformation));
    p.drawImage(r.width()-25, 0, QImage(":/img/closeImage.png"));

    if (m_showZoomAndResolution) {
        p.setFont(m_font);
        p.drawText(0, height()-20, width()-9, 20, Qt::AlignRight | Qt::AlignBottom, QString("%1x%2").arg(img.width()).arg(img.height()));
        p.drawPixmap(0, height()-16, 16, 16, m_zoomIn);
    }
}

QVariant ClosableImage::myData() const
{
    return m_myData;
}

void ClosableImage::setMyData(const QVariant &data)
{
    m_myData = data;
}

void ClosableImage::setImage(const QByteArray &image)
{
    QImage img = QImage::fromData(image);
    m_image = image;
    int zoomSpace = (m_showZoomAndResolution) ? 20 : 0;
    if (m_scaleTo == Qt::Horizontal) {
        // scale to width
        setFixedWidth(m_fixedSize);
        setFixedHeight(qCeil((((qreal)width()-9)/img.width())*img.height()+7+zoomSpace));
    } else {
        // scale to height
        setFixedHeight(m_fixedSize);
        setFixedWidth(qCeil((((qreal)height()-7-zoomSpace)/img.height())*img.width()+9));
    }
}

QByteArray ClosableImage::image() const
{
    return m_image;
}

int ClosableImage::mySize()const
{
    return m_mySize;
}

void ClosableImage::setMySize(const int &size)
{
    m_mySize = size;
    update();
}

void ClosableImage::setShowZoomAndResolution(const bool &show)
{
    m_showZoomAndResolution = show;
}

void ClosableImage::setFixedSize(const int &scaleTo, const int &size)
{
    m_scaleTo = scaleTo;
    m_fixedSize = size;
}
