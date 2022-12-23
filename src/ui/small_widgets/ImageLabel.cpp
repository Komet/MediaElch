#include "ImageLabel.h"
#include "ui_ImageLabel.h"

#include <QMovie>

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

    m_loadingSpinner = new QMovie(":/img/spinner.gif", QByteArray(), this);
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

void ImageLabel::setLoading(bool isLoading)
{
    ui->image->clear();
    if (isLoading) {
        ui->image->setMovie(m_loadingSpinner);
        m_loadingSpinner->start();
    } else {
        ui->image->setMovie(nullptr);
        m_loadingSpinner->stop();
    }
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
