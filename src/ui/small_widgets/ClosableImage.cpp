#include "ClosableImage.h"

#include <QApplication>
#include <QBuffer>
#include <QCheckBox>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOption>
#include <QToolTip>
#include <qmath.h>

#include "data/ImageCache.h"
#include "globals/Helper.h"
#include "globals/ImagePreviewDialog.h"
#include "settings/Settings.h"

ClosableImage::ClosableImage(QWidget* parent) : QLabel(parent)
{
    setMouseTracking(true);
    m_showZoomAndResolution = true;
    m_showCapture = false;
    m_scaleTo = Qt::Horizontal;
    m_fixedSize = 180;
    m_fixedHeight = 0;
    m_clickable = false;
    m_loading = false;
    m_font = QApplication::font();
#ifdef Q_OS_WIN
    m_font.setPointSize(m_font.pointSize() - 1);
#else
    m_font.setPointSize(m_font.pointSize() - 2);
#endif
    m_font.setFamily("Helvetica Neue");

    m_loadingMovie = new QMovie(":/img/spinner.gif", QByteArray(), this);
    m_loadingMovie->start();

    m_zoomIn = QPixmap(":/img/zoom_in.png");
    helper::setDevicePixelRatio(m_zoomIn, helper::devicePixelRatio(this));
    QPainter p;
    p.begin(&m_zoomIn);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(m_zoomIn.rect(), QColor(0, 0, 0, 150));
    p.end();
    const int width = static_cast<int>(16 * helper::devicePixelRatio(this));
    m_zoomIn = m_zoomIn.scaledToWidth(width, Qt::SmoothTransformation);

    m_capture = QPixmap(":/img/photo.png");
    helper::setDevicePixelRatio(m_capture, helper::devicePixelRatio(this));
    p.begin(&m_capture);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(m_capture.rect(), QColor(0, 0, 0, 150));
    p.end();
    m_capture = m_capture.scaledToWidth(width, Qt::SmoothTransformation);

    setAcceptDrops(true);
}

void ClosableImage::mousePressEvent(QMouseEvent* ev)
{
    if (m_loading || ev->button() != Qt::LeftButton || !m_pixmap.isNull()) {
        return;
    }

    if ((!m_image.isNull() || !m_imagePath.isEmpty()) && closeRect().contains(ev->pos())) {
        if (!confirmDeleteImage()) {
            return;
        }
        m_pixmap = grab();
        helper::setDevicePixelRatio(m_pixmap, helper::devicePixelRatio(this));
        m_anim = new QPropertyAnimation(this);
        m_anim->setEasingCurve(QEasingCurve::InQuad);
        m_anim->setTargetObject(this);
        m_anim->setStartValue(0);
        m_anim->setEndValue(width() / 2);
        m_anim->setPropertyName("mySize");
        m_anim->setDuration(400);
        m_anim->start(QPropertyAnimation::DeleteWhenStopped);
        connect(m_anim.data(), &QAbstractAnimation::finished, this, &ClosableImage::sigClose);
        connect(m_anim.data(), &QAbstractAnimation::finished, this, &ClosableImage::closed, Qt::QueuedConnection);

    } else if ((!m_image.isNull() || !m_imagePath.isEmpty()) && m_showZoomAndResolution
               && zoomRect().contains(ev->pos())) {
        if (!m_image.isNull()) {
            auto* dialog = new ImagePreviewDialog(this);
            dialog->setImage(QPixmap::fromImage(QImage::fromData(m_image)));
            dialog->exec();
            dialog->deleteLater();

        } else if (!m_imagePath.isEmpty()) {
            auto* dialog = new ImagePreviewDialog(this);
            dialog->setImage(QPixmap::fromImage(QImage(m_imagePath)));
            dialog->exec();
            dialog->deleteLater();
        }

    } else if (m_showCapture && captureRect().contains(ev->pos())) {
        emit sigCapture(m_imageType);

    } else if (m_clickable && imgRect().contains(ev->pos())) {
        emit clicked();
    }
}

void ClosableImage::mouseMoveEvent(QMouseEvent* ev)
{
    if (!m_loading) {
        if ((!m_image.isNull() || !m_imagePath.isEmpty()) && closeRect().contains(ev->pos())) {
            setCursor(Qt::PointingHandCursor);
            setToolTip(tr("Delete Image"));
            return;
        }

        if ((!m_image.isNull() || !m_imagePath.isEmpty()) && m_showZoomAndResolution
            && zoomRect().contains(ev->pos())) {
            setCursor(Qt::PointingHandCursor);
            setToolTip(tr("Zoom Image"));
            return;
        }

        if (m_showCapture && captureRect().contains(ev->pos())) {
            setCursor(Qt::PointingHandCursor);
            setToolTip(tr("Capture random screenshot"));
            return;
        }

        if (m_clickable && imgRect().contains(ev->pos())) {
            setCursor(Qt::PointingHandCursor);
            setToolTip(tr("Select another image"));
            return;
        }
    }
    setCursor(Qt::ArrowCursor);
    setToolTip("");
}

void ClosableImage::paintEvent(QPaintEvent* event)
{
    QPainter p(this);

    if (m_loading) {
        QLabel::paintEvent(event);
        return;
    }

    if (!m_pixmap.isNull()) {
        const int h = height() * (width() - 2 * m_mySize) / width();
        const int w = static_cast<int>((width() - 2 * m_mySize) * helper::devicePixelRatio(this));
        p.drawPixmap(m_mySize, (height() - h) / 2, m_pixmap.scaledToWidth(w));
        return;
    }

    QImage img;
    int origWidth = 0;
    int origHeight = 0;
    const int w = static_cast<int>((width() - 9) * helper::devicePixelRatio(this));
    if (!m_image.isNull()) {
        img = QImage::fromData(m_image);
        origWidth = img.width();
        origHeight = img.height();
        img = img.scaledToWidth(w, Qt::SmoothTransformation);
    } else if (!m_imagePath.isEmpty()) {
        img = ImageCache::instance()->image(m_imagePath, w, 0, origWidth, origHeight);
    } else {
        const int x =
            static_cast<int>((width() - (m_defaultPixmap.width() / helper::devicePixelRatio(m_defaultPixmap))) / 2);
        const int y =
            static_cast<int>((height() - (m_defaultPixmap.height() / helper::devicePixelRatio(m_defaultPixmap))) / 2);
        p.drawPixmap(x, y, m_defaultPixmap);
        if (m_showCapture) {
            p.drawPixmap(captureRect(), m_capture);
        }
        drawTitle(p);
        return;
    }

    helper::setDevicePixelRatio(img, helper::devicePixelRatio(this));
    QRect r = rect();
    p.drawImage(0, 7, img);
    QImage closeImg =
        QImage(":/img/closeImage.png")
            .scaled(QSize(20, 20) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    helper::setDevicePixelRatio(closeImg, helper::devicePixelRatio(this));
    p.drawImage(r.width() - 21, 0, closeImg);
    if (m_showZoomAndResolution) {
        QString res = QString("%1x%2").arg(origWidth).arg(origHeight);
        QFontMetrics fm(m_font);

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        const int resWidth = fm.horizontalAdvance(res);
#else
        const int resWidth = fm.width(res);
#endif

        p.setFont(m_font);
        p.setPen(QColor(102, 102, 102));
        p.drawText(width() - resWidth - 9, height() - 20, resWidth, 20, Qt::AlignRight | Qt::AlignBottom, res);
        p.drawPixmap(zoomRect(), m_zoomIn);
        drawTitle(p);
    }

    if (m_showCapture) {
        p.drawPixmap(captureRect(), m_capture);
        drawTitle(p);
    }
}

/**
 * An alternative Option...
 */
void ClosableImage::drawTitle(QPainter& p)
{
    Q_UNUSED(p)
    /*
    if (m_title.isEmpty())
        return;

    QFont f = m_font;
    f.setBold(true);
    p.setFont(f);
    QFontMetrics fm(f);
    int width = fm.width(m_title);
    p.drawText(24, height()-20, width, 20, Qt::AlignLeft | Qt::AlignBottom, m_title);
    */
}

QVariant ClosableImage::myData() const
{
    return m_myData;
}

void ClosableImage::setMyData(const QVariant& myData)
{
    m_myData = myData;
}

void ClosableImage::setImage(const QByteArray& image)
{
    clear();
    QImage img = QImage::fromData(image);
    m_image = image;
    updateSize(img.width(), img.height());
}

void ClosableImage::setImage(const QString& image)
{
    setImageByPath(image);
}

void ClosableImage::setImageByPath(const QString& image)
{
    clear();
    m_imagePath = image;
    QSize size = ImageCache::instance()->imageSize(image);
    updateSize(size.width(), size.height());
}

void ClosableImage::updateSize(int imageWidth, int imageHeight)
{
    int zoomSpace = (m_showZoomAndResolution) ? 20 : 0;
    if (m_scaleTo == Qt::Horizontal) {
        // scale to width
        setFixedWidth(m_fixedSize);
        if (imageWidth == 0 || imageHeight == 0) {
            setFixedHeight((m_fixedHeight != 0) ? m_fixedHeight : 135);
        } else {
            int calcHeight = qCeil(((static_cast<qreal>(width()) - 9.0) / imageWidth) * imageHeight + 7.0 + zoomSpace);
            setFixedHeight((m_fixedHeight != 0 && calcHeight < m_fixedHeight) ? m_fixedHeight : calcHeight);
        }
    } else {
        // scale to height
        setFixedHeight(m_fixedSize);
        if (imageWidth == 0 || imageHeight == 0) {
            setFixedWidth(180);
        } else {
            setFixedWidth(qCeil(((static_cast<qreal>(height()) - 7.0 - zoomSpace) / imageHeight) * imageWidth + 9.0));
        }
    }
    update();
}

QByteArray ClosableImage::image()
{
    if (m_image.isNull() && !m_imagePath.isEmpty()) {
        QFile file(m_imagePath);
        if (file.open(QIODevice::ReadOnly)) {
            m_image = file.readAll();
            file.close();
        }
    }
    return m_image;
}

int ClosableImage::mySize() const
{
    return m_mySize;
}

void ClosableImage::setMySize(const int& size)
{
    m_mySize = size;
    update();
}

void ClosableImage::setShowZoomAndResolution(const bool& show)
{
    m_showZoomAndResolution = show;
}

void ClosableImage::setFixedSize(const int& scaleTo, const int& size)
{
    m_scaleTo = scaleTo;
    m_fixedSize = size;
}

void ClosableImage::setMyFixedHeight(const int& height)
{
    m_fixedHeight = height;
}

int ClosableImage::myFixedHeight() const
{
    return m_fixedHeight;
}

void ClosableImage::setDefaultPixmap(QPixmap pixmap)
{
    m_defaultPixmap = pixmap;
    const int w = static_cast<int>((width() - 60) * helper::devicePixelRatio(this));
    const int h = static_cast<int>((height() - 40) * helper::devicePixelRatio(this));
    m_defaultPixmap = m_defaultPixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    helper::setDevicePixelRatio(m_defaultPixmap, helper::devicePixelRatio(this));
}

void ClosableImage::setClickable(const bool& clickable)
{
    m_clickable = clickable;
}

bool ClosableImage::clickable() const
{
    return m_clickable;
}

void ClosableImage::setLoading(const bool& loading)
{
    m_loading = loading;
    if (loading) {
        setMovie(m_loadingMovie);
        m_image = QByteArray();
        m_imagePath.clear();
        update();
    } else {
        setMovie(nullptr);
    }
}

void ClosableImage::clear()
{
    if (m_anim != nullptr) {
        m_anim->stop();
    }
    m_imagePath.clear();
    m_image = QByteArray();
    m_pixmap = m_emptyPixmap;
    m_loading = false;
    setMovie(nullptr);
    update();
}

QRect ClosableImage::zoomRect()
{
    return {0, height() - 16, 16, 16};
}

QRect ClosableImage::captureRect()
{
    return {20, height() - 16, 16, 16};
}

QRect ClosableImage::closeRect()
{
    return {width() - 25, 0, 24, 24};
}

QRect ClosableImage::imgRect()
{
    return {0, 7, width() - 9, height() - 23};
}

void ClosableImage::closed()
{
    m_pixmap = QPixmap();
    m_image = QByteArray();
    m_imagePath.clear();
    update();
}

bool ClosableImage::showCapture() const
{
    return m_showCapture;
}

void ClosableImage::setShowCapture(bool showCapture)
{
    m_showCapture = showCapture;
}

bool ClosableImage::confirmDeleteImage()
{
    if (Settings::instance()->dontShowDeleteImageConfirm()) {
        return true;
    }

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(tr("Really delete image?"));
    msgBox.setText(tr("Are you sure you want to delete this image?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    QCheckBox dontPrompt(tr("Do not ask again"), &msgBox);
    dontPrompt.blockSignals(true);
    msgBox.addButton(&dontPrompt, QMessageBox::ActionRole);
    int ret = msgBox.exec();
    if (dontPrompt.checkState() == Qt::Checked && ret == QMessageBox::Yes) {
        Settings::instance()->setDontShowDeleteImageConfirm(true);
    }
    return (ret == QMessageBox::Yes);
}

void ClosableImage::setTitle(const QString& text)
{
    m_title = text;
}

QString ClosableImage::title() const
{
    return m_title;
}

void ClosableImage::setImageType(ImageType type)
{
    m_imageType = type;
}

ImageType ClosableImage::imageType() const
{
    return m_imageType;
}

void ClosableImage::dragMoveEvent(QDragMoveEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    QUrl url = mimeData->urls().at(0);
    QStringList filters = QStringList() << ".jpg"
                                        << ".jpeg"
                                        << ".png";
    for (const QString& filter : filters) {
        if (url.toString().endsWith(filter, Qt::CaseInsensitive)) {
            event->acceptProposedAction();
            return;
        }
    }
}

void ClosableImage::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    QUrl url = mimeData->urls().at(0);
    QStringList filters = QStringList() << ".jpg"
                                        << ".jpeg"
                                        << ".png";
    for (const QString& filter : filters) {
        if (url.toString().endsWith(filter, Qt::CaseInsensitive)) {
            event->acceptProposedAction();
            return;
        }
    }
}

void ClosableImage::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls() && !mimeData->urls().isEmpty()) {
        QUrl url = mimeData->urls().at(0);
        QStringList filters = QStringList() << ".jpg"
                                            << ".jpeg"
                                            << ".png";
        for (const QString& filter : filters) {
            if (url.toString().endsWith(filter, Qt::CaseInsensitive)) {
                emit sigImageDropped(m_imageType, url);
                return;
            }
        }
    }
}
