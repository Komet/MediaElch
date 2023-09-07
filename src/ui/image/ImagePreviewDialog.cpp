#include "ImagePreviewDialog.h"
#include "ui_ImagePreviewDialog.h"

#include "log/Log.h"
#include "media/AsyncImage.h"
#include "ui/main/MainWindow.h"

#include <QScrollBar>

ImagePreviewDialog::ImagePreviewDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ImagePreviewDialog)
{
    ui->setupUi(this);

    connect(ui->buttonClose, &QAbstractButton::clicked, this, &QDialog::accept);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    m_loadingMovie = new QMovie(":/img/spinner.gif", QByteArray(), this);
}

ImagePreviewDialog::~ImagePreviewDialog()
{
    delete ui;
}

void ImagePreviewDialog::setImage(QPixmap img)
{
    img.setDevicePixelRatio(devicePixelRatioF());
    ui->image->setPixmap(img);
}

void ImagePreviewDialog::setImageFromPath(const mediaelch::FilePath& path)
{
    MediaElch_Expects(!m_isLoading); // should only be called once

    setLoading(true);
    m_asyncImage = mediaelch::AsyncImage::fromPath(path);
    connect(m_asyncImage.get(), &mediaelch::AsyncImage::sigLoaded, this, [this]() { //
        setLoading(false);
        setImage(QPixmap::fromImage(m_asyncImage->image()));
    });
}

void ImagePreviewDialog::setLoading(bool loading)
{
    m_isLoading = loading;
    if (loading) {
        m_loadingMovie->start();
        ui->image->setMovie(m_loadingMovie);
        update();
    } else {
        m_loadingMovie->stop();
        ui->image->setMovie(nullptr);
    }
}

int ImagePreviewDialog::exec()
{
    qCDebug(generic) << "[ImagePreviewDialog] Open image preview dialog";

    ui->scrollArea->verticalScrollBar()->setValue(0);
    ui->scrollArea->horizontalScrollBar()->setValue(0);

    QWidget* window = MainWindow::instance();

    QSize newSize;
    newSize.setHeight(window->size().height() - 50);
    newSize.setWidth(qMin(1200, window->size().width() - 100));
    resize(newSize);

    const int xMove = (window->size().width() - size().width()) / 2;
    move(window->x() + xMove, window->y());

    return QDialog::exec();
}
