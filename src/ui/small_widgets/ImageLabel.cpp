#include "ImageLabel.h"
#include "ui_ImageLabel.h"

ImageLabel::ImageLabel(QWidget* parent) : QWidget(parent), ui(new Ui::ImageLabel)
{
    ui->setupUi(this);
    ui->resolution->setText("");
    QFont font = ui->resolution->font();
#ifdef Q_OS_WIN
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->resolution->setFont(font);
}

ImageLabel::~ImageLabel()
{
    delete ui;
}

/**
 * \brief Sets the image to show
 * \param img QPixmap of the image
 */
void ImageLabel::setImage(QPixmap img)
{
    ui->image->setPixmap(img);
}

/**
 * \brief Sets the resolution text
 * \param resolution Resolution (QSize) to set
 * \param hint Hint
 */
void ImageLabel::setHint(QSize resolution, QString hint)
{
    QString text;
    if (!resolution.isNull() && !resolution.isEmpty() && resolution.isValid()) {
        text = QString("%1 x %2").arg(resolution.width()).arg(resolution.height());
    }
    text.append(hint);
    ui->resolution->setText(text);
}

QImage ImageLabel::image() const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return ui->image->pixmap()->toImage();
#else
    return ui->image->pixmap(Qt::ReturnByValue).toImage();
#endif
}
