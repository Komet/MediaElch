#include "ui/small_widgets/WebImageLabel.h"
#include "ui_WebImageLabel.h"

#include "network/NetworkRequest.h"
#include "utils/Meta.h"

#include <QMovie>

WebImageLabel::WebImageLabel(QWidget* parent) : QWidget(parent), ui(new Ui::WebImageLabel)
{
    ui->setupUi(this);
    ui->resolution->hide();

    m_placeholder = scaleToFit(QPixmap{":/img/placeholders/poster.png"});
    m_loadingSpinner = new QMovie(":/img/spinner.gif", QByteArray(), this);

    showPlaceholder();
}

WebImageLabel::~WebImageLabel()
{
    // Current network download is freed via NetworkManager destructor.
    m_currentPosterDownload = nullptr;
    // Spinner deleted via parent->child relationship.s
    m_loadingSpinner = nullptr;
    delete ui;
}

void WebImageLabel::showImageFrom(QUrl url)
{
    showLoading(true);
    if (url.isValid()) {
        m_currentPosterDownload = network.getWithWatcher(mediaelch::network::requestWithDefaults(url));
        connect(m_currentPosterDownload, &QNetworkReply::finished, this, &WebImageLabel::onImageDownloaded);
    } else {
        showPlaceholder();
    }
}

void WebImageLabel::onImageDownloaded()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    MediaElch_Expects(reply != nullptr);
    auto dls = makeDeleteLaterScope(reply);

    if (reply->error()) {
        showPlaceholder();
        return;
    }

    QPixmap pixmap;
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    pixmap.loadFromData(reply->readAll());

    setImage(pixmap);
}

void WebImageLabel::clearAndAbortDownload()
{
    if (m_currentPosterDownload != nullptr && m_currentPosterDownload->isRunning()) {
        m_currentPosterDownload->abort();
    }
    m_currentPosterDownload = nullptr;
    showPlaceholder();
}

void WebImageLabel::setImage(QPixmap img)
{
    showLoading(false);
    ui->image->setPixmap(scaleToFit(img));
}

void WebImageLabel::showPlaceholder()
{
    setImage(m_placeholder);
}

void WebImageLabel::startLoadingSpinner()
{
    showLoading(true);
}

void WebImageLabel::showLoading(bool isLoading)
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

QPixmap WebImageLabel::scaleToFit(const QPixmap& img)
{
    // TODO: Is there an easier way to use the correct size? QImage?
    const int width = static_cast<int>(ui->image->width() * devicePixelRatioF());
    QPixmap scaledImg = img.scaledToWidth(width, Qt::SmoothTransformation);
    scaledImg.setDevicePixelRatio(devicePixelRatioF());
    return scaledImg;
}
