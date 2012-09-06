#ifndef IMAGEPREVIEWDIALOG_H
#define IMAGEPREVIEWDIALOG_H

#include <QDialog>

namespace Ui {
class ImagePreviewDialog;
}

/**
 * @brief This dialog shows a full size preview of images
 */
class ImagePreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImagePreviewDialog(QWidget *parent = 0);
    ~ImagePreviewDialog();
    static ImagePreviewDialog *instance(QWidget *parent = 0);
    void setImage(QPixmap img);

public slots:
    int exec();

private:
    Ui::ImagePreviewDialog *ui;
};

#endif // IMAGEPREVIEWDIALOG_H
