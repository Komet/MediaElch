#include "ImagePreviewDialog.h"
#include "ui_ImagePreviewDialog.h"

#include "globals/Helper.h"

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

ImagePreviewDialog* ImagePreviewDialog::instance(QWidget* parent)
{
    static ImagePreviewDialog* s_instance = nullptr;
    if (s_instance == nullptr) {
        s_instance = new ImagePreviewDialog(parent);
    }
    return s_instance;
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

    QSize newSize;
    newSize.setHeight(parentWidget()->size().height() - 50);
    newSize.setWidth(qMin(1200, parentWidget()->size().width() - 100));
    resize(newSize);

    // move assumes coordinates relative to the parent widget
    int xMove = (parentWidget()->size().width() - size().width()) / 2;
    move(parentWidget()->x() + xMove, parentWidget()->y());

    return QDialog::exec();
}
