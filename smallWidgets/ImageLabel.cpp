#include "ImageLabel.h"
#include "ui_ImageLabel.h"

#include "Globals.h"

/**
 * @brief ImageLabel::ImageLabel
 * @param parent
 */
ImageLabel::ImageLabel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageLabel)
{
    ui->setupUi(this);
    ui->resolution->setText("");
    QFont font = ui->resolution->font();
    font.setPointSize(font.pointSize()-2);
    ui->resolution->setFont(font);
}

/**
 * @brief ImageLabel::~ImageLabel
 */
ImageLabel::~ImageLabel()
{
    delete ui;
}

/**
 * @brief Sets the image to show
 * @param pixmap QPixmap of the image
 */
void ImageLabel::setImage(QPixmap pixmap)
{
    ui->image->setPixmap(pixmap);
}

/**
 * @brief Sets the resolution text
 * @param resolution Resolution string to set
 */
void ImageLabel::setResolution(QString resolution)
{
    ui->resolution->setText(resolution);
}

/**
 * @brief Sets the resolution text
 * @param resolution Resolution (QSize) to set
 */
void ImageLabel::setResolution(QSize resolution)
{
    if (resolution.isNull() || resolution.isEmpty() || !resolution.isValid()) {
        ui->resolution->setText("");
        return;
    }

    ui->resolution->setText(QString("%1 x %2").arg(resolution.width()).arg(resolution.height()));
}
