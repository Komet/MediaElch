#pragma once

#include <QDialog>

namespace Ui {
class ImagePreviewDialog;
}

/// \brief This dialog shows a full size preview of images, e.g. a movie's poster.
class ImagePreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImagePreviewDialog(QWidget* parent = nullptr);
    ~ImagePreviewDialog() override;

    /// \brief Sets the image to be displayed
    /// \param img Image in high definition to be shown to the user.
    void setImage(QPixmap img);

public slots:
    /// \brief Executes the dialog and adjusts its size.
    /// \return The QDialog::exec result
    int exec() override;

private:
    Ui::ImagePreviewDialog* ui;
};
