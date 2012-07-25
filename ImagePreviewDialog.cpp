#include "ImagePreviewDialog.h"
#include "ui_ImagePreviewDialog.h"
#include <QScrollBar>

ImagePreviewDialog::ImagePreviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImagePreviewDialog)
{
    ui->setupUi(this);
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(accept()));
#ifdef Q_WS_MAC
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

ImagePreviewDialog *ImagePreviewDialog::instance(QWidget *parent)
{
    static ImagePreviewDialog *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new ImagePreviewDialog(parent);
    }
    return m_instance;
}

void ImagePreviewDialog::setImage(QPixmap img)
{
    ui->image->setPixmap(img);
}

int ImagePreviewDialog::exec()
{
    ui->scrollArea->verticalScrollBar()->setValue(0);
    ui->scrollArea->horizontalScrollBar()->setValue(0);
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-100);
    newSize.setWidth(qMin(1000, parentWidget()->size().width()-200));
    resize(newSize);

    int xMove = (parentWidget()->size().width()-size().width())/2;
    QPoint globalPos = parentWidget()->mapToGlobal(parentWidget()->pos());
    move(globalPos.x()+xMove, globalPos.y());

    return QDialog::exec();
}
