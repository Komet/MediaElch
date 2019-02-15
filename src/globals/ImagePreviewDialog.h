#pragma once

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
    explicit ImagePreviewDialog(QWidget* parent = nullptr);
    ~ImagePreviewDialog() override;
    static ImagePreviewDialog* instance(QWidget* parent = nullptr);
    void setImage(QPixmap img);

public slots:
    int exec() override;

private:
    Ui::ImagePreviewDialog* ui;
};
