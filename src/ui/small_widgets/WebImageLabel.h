#pragma once

#include "network/NetworkManager.h"

#include <QPixmap>
#include <QPointer>
#include <QWidget>

namespace Ui {
class WebImageLabel;
}

class QMovie;

/// \brief   A widget with a weg-image that is downloaded and a text below
/// \details This class can be used to download an image from the web and
///          show it, with an optional label.  It has a default image and
///          shows a spinner while loading the image.
///          Has its own NetworkManager, so don't use it in a gallery with
///          many images at once.
class WebImageLabel : public QWidget
{
    Q_OBJECT

public:
    explicit WebImageLabel(QWidget* parent = nullptr);
    ~WebImageLabel() override;

    /// \brief Set the to-be-downloaded image.
    void showImageFrom(QUrl url);
    /// \brief Clears the image label (shows a placeholder) and aborts all downloads.
    void clearAndAbortDownload();

    /// \brief Start a loading icon (animated spinner).
    void startLoadingSpinner();
    /// \brief Show a placeholder image.
    void showPlaceholder();

private slots:
    /// \brief Called when the image set via showImageFrom() was downloaded.
    void onImageDownloaded();

private:
    /// \brief Set the currently shown image. Scales the image to fit.
    void setImage(QPixmap img);
    /// \brief Show a loading icon (animated spinner).
    void showLoading(bool isLoading);
    /// \brief Scale the pixmap according to current device pixel ratio and label size.
    QPixmap scaleToFit(const QPixmap& img);

private:
    Ui::WebImageLabel* ui;
    QPixmap m_placeholder;
    mediaelch::network::NetworkManager network;
    QPointer<QNetworkReply> m_currentPosterDownload = nullptr;
    QMovie* m_loadingSpinner = nullptr;
};
