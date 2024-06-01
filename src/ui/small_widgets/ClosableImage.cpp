#include "ui/small_widgets/ClosableImage.h"

#include "settings/Settings.h"
#include "ui/image/ImagePreviewDialog.h"
#include "ui/main/MainWindow.h"

#include <QApplication>
#include <QBuffer>
#include <QCheckBox>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOption>
#include <QToolTip>
#include <qmath.h>

ClosableImage::ClosableImage(QWidget* parent) : QLabel(parent)
{
    setMouseTracking(true);
    m_font = QApplication::font();
#ifdef Q_OS_WIN
    m_font.setPointSize(m_font.pointSize() - 1);
#else
    m_font.setPointSize(m_font.pointSize() - 2);
#endif
    m_font.setFamily("Helvetica Neue");

    m_loadingMovie = new QMovie(":/img/spinner.gif", QByteArray(), this);

    m_zoomIn = QPixmap(":/img/zoom_in.png");
    m_zoomIn.setDevicePixelRatio(devicePixelRatioF());
    QPainter p;
    p.begin(&m_zoomIn);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(m_zoomIn.rect(), QColor(0, 0, 0, 150));
    p.end();
    const int width = static_cast<int>(16.0 * devicePixelRatioF());
    m_zoomIn = m_zoomIn.scaledToWidth(width, Qt::SmoothTransformation);

    m_capture = QPixmap(":/img/photo.png");
    m_capture.setDevicePixelRatio(devicePixelRatioF());
    p.begin(&m_capture);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(m_capture.rect(), QColor(0, 0, 0, 150));
    p.end();
    m_capture = m_capture.scaledToWidth(width, Qt::SmoothTransformation);

    setAcceptDrops(true);
}

void ClosableImage::mousePressEvent(QMouseEvent* ev)
{
    if (m_isLoading || ev->button() != Qt::LeftButton || !m_pixmapForCloseAnimation.isNull()) {
        return;
    }

    if (hasImage() && closeRect().contains(ev->pos())) {
        onCloseImageRequest();

    } else if (m_showZoomAndResolution && hasImage() && zoomRect().contains(ev->pos())) {
        // TODO: Don't use "this", because we don't want to inherit the stylesheet,
        //       but we can't pass "nullptr", because otherwise there won't be a modal.
        auto* dialog = new ImagePreviewDialog(MainWindow::instance());
        if (!m_image.isNull()) {
            dialog->setImage(QPixmap::fromImage(QImage::fromData(m_image)));
        } else {
            dialog->setImageFromPath(m_imageFromPath->path());
        }
        dialog->exec();
        dialog->deleteLater();

    } else if (m_showCapture && captureRect().contains(ev->pos())) {
        emit sigCapture(m_imageType);

    } else if (m_clickable && imgRect().contains(ev->pos())) {
        emit clicked();
    }
}

void ClosableImage::mouseMoveEvent(QMouseEvent* ev)
{
    if (!m_isLoading) {
        if ((!m_image.isNull() || (m_imageFromPath != nullptr && !m_imageFromPath->isNull()))
            && closeRect().contains(ev->pos())) {
            setCursor(Qt::PointingHandCursor);
            setToolTip(tr("Delete Image"));
            return;
        }

        if ((!m_image.isNull() || (m_imageFromPath != nullptr && !m_imageFromPath->isNull())) && m_showZoomAndResolution
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

    if (m_isLoading) {
        QLabel::paintEvent(event);
        return;
    }

    if (!m_pixmapForCloseAnimation.isNull()) {
        const int h = height() * (width() - 2 * m_mySize) / width();
        const int w = static_cast<int>((width() - 2 * m_mySize) * devicePixelRatioF());
        p.drawPixmap(m_mySize, (height() - h) / 2, m_pixmapForCloseAnimation.scaledToWidth(w));
        return;
    }

    QImage img;
    int origWidth = 0;
    int origHeight = 0;
    const int w = imageWith();
    if (!m_image.isNull()) {
        img = QImage::fromData(m_image);
        origWidth = img.width();
        origHeight = img.height();
        img = img.scaledToWidth(w, Qt::SmoothTransformation);
    } else if (m_imageFromPath != nullptr && !m_imageFromPath->isNull()) {
        img = m_imageFromPath->image(); // ImageCache::instance()->image(mediaelch::FilePath(m_imagePath), w, 0,
                                        // origWidth, origHeight);
        origWidth = m_imageFromPath->originalSize().width();
        origHeight = m_imageFromPath->originalSize().height();
        img = img.scaledToWidth(w, Qt::SmoothTransformation);
    } else {
        const int x = static_cast<int>((width() - (m_defaultPixmap.width() / m_defaultPixmap.devicePixelRatioF())) / 2);
        const int y =
            static_cast<int>((height() - (m_defaultPixmap.height() / m_defaultPixmap.devicePixelRatioF())) / 2.0);
        p.drawPixmap(x, y, m_defaultPixmap);
        if (m_showCapture) {
            p.drawPixmap(captureRect(), m_capture);
        }
        return;
    }

    img.setDevicePixelRatio(devicePixelRatioF());
    QRect r = rect();
    p.drawImage(0, 7, img);
    QImage closeImg = QImage(":/img/closeImage.png")
                          .scaled(QSize(20, 20) * devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    closeImg.setDevicePixelRatio(devicePixelRatioF());
    p.drawImage(r.width() - 21, 0, closeImg);
    if (m_showZoomAndResolution) {
        QString res = QStringLiteral("%1x%2").arg(origWidth).arg(origHeight);
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
    }

    if (m_showCapture) {
        p.drawPixmap(captureRect(), m_capture);
    }
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

void ClosableImage::setImageFromPath(const mediaelch::FilePath& image)
{
    clear();
    setLoading(true);
    const int w = imageWith();
    m_imageFromPath = ImageCache::instance()->loadImageAsync(image, QSize{w, 0});
    connect(m_imageFromPath.get(),
        &mediaelch::AsyncImage::sigLoaded,
        this,
        &ClosableImage::onAsyncImageLoaded,
        Qt::DirectConnection);
}

void ClosableImage::onAsyncImageLoaded()
{
    // TODO: Handle the case that m_imageFromPath was changed before the signal was emitted.
    //       Can that happen? Should only happen for queued events, as otherwise the object was deleted.
    MediaElch_Expects(m_imageFromPath != nullptr);
    setLoading(false);
    QSize size = m_imageFromPath->image().size();
    updateSize(size.width(), size.height());
}


void ClosableImage::onCloseImageRequest()
{
    if (!confirmDeleteImage()) {
        return;
    }
    m_pixmapForCloseAnimation = grab();
    m_pixmapForCloseAnimation.setDevicePixelRatio(devicePixelRatioF());
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
}

int ClosableImage::imageWith() const
{
    return static_cast<int>((width() - 9) * devicePixelRatioF());
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

void ClosableImage::setShowZoomAndResolution(bool show)
{
    m_showZoomAndResolution = show;
}

void ClosableImage::setFixedSize(Qt::Orientation scaleTo, int size)
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

void ClosableImage::setDefaultPixmap(QPixmap pixmap, bool useDefaultBorder)
{
    const int borderWidth = useDefaultBorder ? 60 : 0;
    const int borderHeight = useDefaultBorder ? 40 : 0;
    const float ratio = devicePixelRatioF();
    const int w = static_cast<int>((width() - borderWidth) * ratio);
    const int h = static_cast<int>((height() - borderHeight) * ratio);
    pixmap = pixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(ratio);
    m_defaultPixmap = pixmap;
}

void ClosableImage::setClickable(bool clickable)
{
    m_clickable = clickable;
}

bool ClosableImage::clickable() const
{
    return m_clickable;
}

void ClosableImage::setLoading(bool loading)
{
    m_isLoading = loading;
    if (loading) {
        m_loadingMovie->start();
        setMovie(m_loadingMovie);
        m_image = {};
        m_imageFromPath = nullptr;
        update();
    } else {
        m_loadingMovie->stop();
        setMovie(nullptr);
    }
}

void ClosableImage::clear()
{
    if (m_anim != nullptr) {
        m_anim->stop();
    }
    m_image = {};
    m_imageFromPath = nullptr;
    m_pixmapForCloseAnimation = {};
    m_isLoading = false;
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
    m_pixmapForCloseAnimation = {};
    m_image = {};
    m_imageFromPath = nullptr;
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
