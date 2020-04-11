#include "ImagePreviewDialog.h"
#include "ui_ImagePreviewDialog.h"

#include "globals/Helper.h"
#include "ui/main/MainWindow.h"

#include <QDebug>
#include <QScrollBar>

ImagePreviewDialog::ImagePreviewDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ImagePreviewDialog)
{
    ui->setupUi(this);

    connect(ui->buttonClose, &QAbstractButton::clicked, this, &QDialog::accept);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #ImagePreviewDialog { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif
}

ImagePreviewDialog::~ImagePreviewDialog()
{
    delete ui;
}

void ImagePreviewDialog::setImage(QPixmap img)
{
    helper::setDevicePixelRatio(img, helper::devicePixelRatio(this));
    ui->image->setPixmap(img);
}

int ImagePreviewDialog::exec()
{
    qDebug() << "[ImagePreviewDialog] Open image preview dialog";

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
