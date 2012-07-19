#include "ImageLabel.h"
#include "ui_ImageLabel.h"

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

ImageLabel::~ImageLabel()
{
    delete ui;
}

void ImageLabel::setImage(QPixmap pixmap)
{
    ui->image->setPixmap(pixmap);
}

void ImageLabel::setResolution(QString resolution)
{
    ui->resolution->setText(resolution);
}

void ImageLabel::setResolution(QSize resolution)
{
    if (resolution.isNull() || resolution.isEmpty() || !resolution.isValid()) {
        ui->resolution->setText("");
        return;
    }

    ui->resolution->setText(QString("%1 x %2").arg(resolution.width()).arg(resolution.height()));
}
