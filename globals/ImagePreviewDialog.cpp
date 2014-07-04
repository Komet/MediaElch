#include "ImagePreviewDialog.h"
#include "ui_ImagePreviewDialog.h"

#include <QDebug>
#include <QScrollBar>

#include "globals/Globals.h"
#include "globals/Helper.h"

/**
 * @brief ImagePreviewDialog::ImagePreviewDialog
 * @param parent
 */
ImagePreviewDialog::ImagePreviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImagePreviewDialog)
{
    ui->setupUi(this);
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(accept()));
#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #ImagePreviewDialog { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif
}

/**
 * @brief ImagePreviewDialog::~ImagePreviewDialog
 */
ImagePreviewDialog::~ImagePreviewDialog()
{
    delete ui;
}

/**
 * @brief Returns an instance of the dialog
 * @param parent Parent widget
 * @return Instance of ImagePreviewDialog
 */
ImagePreviewDialog *ImagePreviewDialog::instance(QWidget *parent)
{
    static ImagePreviewDialog *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new ImagePreviewDialog(parent);
    }
    return m_instance;
}

/**
 * @brief Sets the image to be displayed
 * @param img Image
 */
void ImagePreviewDialog::setImage(QPixmap img)
{
    qDebug() << "Entered";
    Helper::instance()->setDevicePixelRatio(img, Helper::instance()->devicePixelRatio(this));
    ui->image->setPixmap(img);
}

/**
 * @brief Executes the dialog and adjusts its size
 * @return The QDialog::exec result
 */
int ImagePreviewDialog::exec()
{
    qDebug() << "Entered";
    ui->scrollArea->verticalScrollBar()->setValue(0);
    ui->scrollArea->horizontalScrollBar()->setValue(0);
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-50);
    newSize.setWidth(qMin(1200, parentWidget()->size().width()-100));
    resize(newSize);

    int xMove = (parentWidget()->size().width()-size().width())/2;
    QPoint globalPos = parentWidget()->mapToGlobal(parentWidget()->pos());
    move(globalPos.x()+xMove, globalPos.y());

    return QDialog::exec();
}
