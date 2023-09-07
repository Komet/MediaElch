#pragma once

#include "media/Path.h"

#include <QDialog>
#include <QMovie>
#include <memory>

namespace Ui {
class ImagePreviewDialog;
}

namespace mediaelch {
class AsyncImage;
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
    /// \brief Sets the image to be displayed. Loaded from disk.
    /// \param path Image to show in high resolution.
    void setImageFromPath(const mediaelch::FilePath& path);

public slots:
    /// \brief Executes the dialog and adjusts its size.
    /// \return The QDialog::exec result
    int exec() override;

private:
    void setLoading(bool loading);

private:
    Ui::ImagePreviewDialog* ui;
    std::unique_ptr<mediaelch::AsyncImage> m_asyncImage;
    QMovie* m_loadingMovie{nullptr};
    bool m_isLoading{false};
};
