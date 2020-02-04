#include "ImageDialog.h"
#include "ui_ImageDialog.h"

#include <QBuffer>
#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QMovie>
#include <QPainter>
#include <QSize>
#include <QStandardPaths>
#include <QTimer>
#include <QtCore/qmath.h>

#include "concerts/Concert.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/NameFormatter.h"
#include "movies/Movie.h"
#include "music/Album.h"
#include "music/Artist.h"
#include "scrapers/image/ImageProviderInterface.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"
#include "ui/small_widgets/ImageLabel.h"

ImageDialog::ImageDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ImageDialog)
{
    ui->setupUi(this);
    ui->searchTerm->setType(MyLineEdit::TypeLoading);
    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    ui->gallery->setAlignment(Qt::Horizontal);
    ui->gallery->setShowZoomAndResolution(false);

    resize(Settings::instance()->settings()->value("ImageDialog/Size").toSize());

    // clang-format off
    connect(ui->table,             &QTableWidget::cellClicked,       this, &ImageDialog::imageClicked);
    connect(ui->table,             &MyTableWidget::sigDroppedImage,  this, &ImageDialog::onImageDropped);
    connect(ui->buttonClose,       SIGNAL(clicked()),                this, SLOT(reject()));
    connect(ui->buttonChoose,      &QAbstractButton::clicked,        this, &ImageDialog::chooseLocalImage);
    connect(ui->previewSizeSlider, &QAbstractSlider::valueChanged,   this, &ImageDialog::onPreviewSizeChange);
    connect(ui->buttonZoomIn,      &QAbstractButton::clicked,        this, &ImageDialog::onZoomIn);
    connect(ui->buttonZoomOut,     &QAbstractButton::clicked,        this, &ImageDialog::onZoomOut);
    connect(ui->searchTerm,        &MyLineEdit::returnPressed,       this, &ImageDialog::onSearchWithAllResults);
    connect(ui->imageProvider,     SIGNAL(currentIndexChanged(int)), this, SLOT(onProviderChanged(int)));
    connect(ui->results,           &QTableWidget::itemClicked,       this, &ImageDialog::onResultClicked);
    connect(ui->gallery,           SIGNAL(sigRemoveImage(QString)),  this, SLOT(onImageClosed(QString)));
    connect(ui->btnAcceptImages,   SIGNAL(clicked()),                this, SLOT(accept()));
    // clang-format on

    ui->btnAcceptImages->hide();

    QMovie* movie = new QMovie(":/img/spinner.gif", QByteArray(), this);
    movie->start();
    ui->labelSpinner->setMovie(movie);
    clearSearch();
    setImageType(ImageType::MoviePoster);
    m_currentDownloadReply = nullptr;
    m_multiSelection = false;

    QPixmap zoomOut(":/img/zoom_out.png");
    QPixmap zoomIn(":/img/zoom_in.png");
    QPainter p;
    p.begin(&zoomOut);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(zoomOut.rect(), QColor(0, 0, 0, 150));
    p.end();
    p.begin(&zoomIn);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(zoomIn.rect(), QColor(0, 0, 0, 150));
    p.end();
    ui->buttonZoomOut->setIcon(QIcon(zoomOut));
    ui->buttonZoomIn->setIcon(QIcon(zoomIn));

    for (ImageProviderInterface* provider : Manager::instance()->imageProviders()) {
        connect(provider, &ImageProviderInterface::sigSearchDone, this, &ImageDialog::onSearchFinished);
        connect(provider, &ImageProviderInterface::sigImagesLoaded, this, &ImageDialog::onProviderImagesLoaded);
    }
}

/**
 * @brief ImageDialog::~ImageDialog
 */
ImageDialog::~ImageDialog()
{
    delete ui;
}

int ImageDialog::exec()
{
    return 0;
}

/**
 * @brief Executes the dialog and returns the result of QDialog::exec
 * @param type Type of the images (ImageDialogType)
 * @return Result of QDialog::exec
 */
int ImageDialog::exec(ImageType type)
{
    m_type = type;

    auto* settings = Settings::instance()->settings();

    // set slider value
    ui->previewSizeSlider->setValue(
        settings->value(QString("ImageDialog/PreviewSize_%1").arg(static_cast<int>(m_type)), 8).toInt());

    const QSize savedSize = settings->value("ImageDialog/Size").toSize();
    const QPoint savedPos = settings->value("ImageDialog/Pos").toPoint();

#ifdef Q_OS_MAC
    constexpr bool isMac = true;
#else
    constexpr bool isMac = false;
#endif

    if (savedSize.isValid() && !savedSize.isNull() && !isMac) {
        resize(savedSize);
    } else {
        // resize
        QSize newSize;
        newSize.setHeight(parentWidget()->size().height() - 50);
        newSize.setWidth(qMin(1200, parentWidget()->size().width() - 100));
        resize(newSize);
    }

    if (!savedPos.isNull() && !isMac) {
        move(savedPos);
    } else {
        // move to center
        const int xMove = (parentWidget()->size().width() - size().width()) / 2;
        QPoint globalPos = parentWidget()->mapToGlobal(parentWidget()->pos());
        move(globalPos.x() + xMove, qMax(0, globalPos.y() - 100));
    }

    // get image providers and setup combo box
    m_providers = Manager::instance()->imageProviders(type);

    ui->imageProvider->blockSignals(true);
    ui->imageProvider->clear();

    if (hasDefaultImages() || !hasImageProvider()) {
        ui->imageProvider->addItem(tr("Default"));
        ui->imageProvider->setItemData(0, true, DataRole::isDefaultProvider);
    }

    for (ImageProviderInterface* provider : m_providers) {
        int row = ui->imageProvider->count();
        ui->imageProvider->addItem(provider->name());
        ui->imageProvider->setItemData(row, QVariant::fromValue(provider), DataRole::providerPointer);
        ui->imageProvider->setItemData(row, false, DataRole::isDefaultProvider);
    }
    ui->imageProvider->blockSignals(false);
    updateSourceLink();

    ui->searchTerm->setLoading(false);
    ui->searchTerm->setReadOnly(!hasImageProvider());
    ui->searchTerm->setEnabled(hasImageProvider());
    ui->imageProvider->setEnabled(hasImageProvider());

    // show image widget
    ui->stackedWidget->setCurrentIndex(1);

    if (m_itemType == ItemType::Movie) {
        ui->searchTerm->setText(formatSearchText(m_movie->name()));
    } else if (m_itemType == ItemType::Concert) {
        ui->searchTerm->setText(formatSearchText(m_concert->name()));
    } else if (m_itemType == ItemType::TvShow) {
        ui->searchTerm->setText(formatSearchText(m_tvShow->name()));
    } else if (m_itemType == ItemType::TvShowEpisode) {
        ui->searchTerm->setText(formatSearchText(m_tvShowEpisode->tvShow()->name()));
    } else if (m_itemType == ItemType::Album) {
        ui->searchTerm->setText(formatSearchText(m_album->title()));
    } else if (m_itemType == ItemType::Artist) {
        ui->searchTerm->setText(formatSearchText(m_artist->name()));
    } else {
        ui->searchTerm->clear();
    }

    if (hasImageProvider()) {
        onSearch(true);
    }

    if (!hasDefaultImages() && !hasImageProvider()) {
        qInfo() << "[ImageDialog] No provider available nor a default image";
        showError(tr(
            "Neither a image provider nor previously scraped image URLs are available for the requested image type."));
    }

    QDialog::show();
    renderTable();

    return QDialog::exec();
}

/**
 * @brief Accepts the dialog and saves the size of the preview images
 */
void ImageDialog::accept()
{
    qDebug() << "Entered";
    cancelDownloads();
#ifndef Q_OS_MAC
    Settings::instance()->settings()->setValue("ImageDialog/Size", size());
    Settings::instance()->settings()->setValue("ImageDialog/Pos", pos());
    Settings::instance()->settings()->sync();
#endif
    QDialog::accept();
}

/**
 * @brief Rejects the dialog and saves the size of the preview images
 */
void ImageDialog::reject()
{
    qDebug() << "Entered";
    cancelDownloads();
#ifndef Q_OS_MAC
    Settings::instance()->settings()->setValue("ImageDialog/Size", size());
    Settings::instance()->settings()->setValue("ImageDialog/Pos", pos());
    Settings::instance()->settings()->sync();
#endif
    QDialog::reject();
}

/**
 * @brief Returns an instance of ImageDialog
 * @param parent Parent widget (used the first time for constructing)
 * @return Instance of ImageDialog
 */
ImageDialog* ImageDialog::instance(QWidget* parent)
{
    static ImageDialog* s_instance = new ImageDialog(parent);
    return s_instance;
}

/**
 * @brief Clears the dialogs contents and cancels outstanding downloads
 */
void ImageDialog::clear()
{
    m_imageUrls.clear();
    setMultiSelection(false);
    ui->gallery->clear();
    ui->btnAcceptImages->hide();
    clearSearch();
}

void ImageDialog::clearSearch()
{
    cancelDownloads();
    m_elements.clear();
    ui->table->clearContents();
    ui->table->setRowCount(0);
}

/**
 * @brief Return the url of the last clicked image
 * @return URL of the last image clicked
 * @see ImageDialog::imageClicked
 */
QUrl ImageDialog::imageUrl()
{
    qDebug() << "Entered, returning" << m_imageUrl;
    return m_imageUrl;
}

/**
 * @brief Renders the table when the size of the dialog changes
 */
void ImageDialog::resizeEvent(QResizeEvent* event)
{
    if (calcColumnCount() != ui->table->columnCount()) {
        renderTable();
    }
    QDialog::resizeEvent(event);
}

/**
 * @brief Sets a list of images to be downloaded and shown
 * @param downloads List of images (downloads)
 * @param initial If true saves downloads as defaults
 */
void ImageDialog::setDownloads(QVector<Poster> downloads, bool initial)
{
    qDebug() << "Entered";
    ui->stackedWidget->setCurrentIndex(1);
    if (initial) {
        m_defaultElements = downloads;
    }
    for (const Poster& poster : downloads) {
        DownloadElement d;
        d.originalUrl = poster.originalUrl;
        d.thumbUrl = poster.thumbUrl;
        d.downloaded = false;
        d.resolution = poster.originalSize;
        d.hint = poster.hint;
        if (!poster.language.isEmpty()) {
            d.hint.append(" (" + poster.language + ")");
        }
        m_elements.append(d);
    }
    ui->labelLoading->setVisible(true);
    ui->labelSpinner->setVisible(true);
    startNextDownload();
    renderTable();
    if (downloads.count() == 0) {
        ui->stackedWidget->setCurrentIndex(2);
    }
}

/**
 * @brief Returns an instance of a network access manager
 * @return Instance of a network access manager
 */
QNetworkAccessManager* ImageDialog::qnam()
{
    return &m_qnam;
}

/**
 * @brief Starts the next download if there is one
 */
void ImageDialog::startNextDownload()
{
    qDebug() << "[ImageDialog] Start next download";

    int nextIndex = -1;
    for (int i = 0, n = m_elements.size(); i < n; i++) {
        if (!m_elements[i].downloaded) {
            nextIndex = i;
            break;
        }
    }

    if (nextIndex == -1) {
        ui->labelLoading->setVisible(false);
        ui->labelSpinner->setVisible(false);
        return;
    }
    m_currentDownloadIndex = nextIndex;
    m_currentDownloadReply = qnam()->get(QNetworkRequest(m_elements[nextIndex].thumbUrl));
    connect(m_currentDownloadReply, &QNetworkReply::finished, this, &ImageDialog::downloadFinished);
}

/**
 * @brief Called when a download has finished
 * Renders the table and displays the downloaded image and starts the next download
 */
void ImageDialog::downloadFinished()
{
    qDebug() << "Entered";

    if (m_currentDownloadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302
        || m_currentDownloadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301) {
        m_currentDownloadReply->deleteLater();
        m_currentDownloadReply = qnam()->get(
            QNetworkRequest(m_currentDownloadReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl()));
        connect(m_currentDownloadReply, &QNetworkReply::finished, this, &ImageDialog::downloadFinished);
        return;
    }

    if (m_currentDownloadReply->error() == QNetworkReply::NoError) {
        m_elements[m_currentDownloadIndex].pixmap.loadFromData(m_currentDownloadReply->readAll());
        helper::setDevicePixelRatio(m_elements[m_currentDownloadIndex].pixmap, helper::devicePixelRatio(this));

        if (!m_elements[m_currentDownloadIndex].pixmap.isNull()) {
            const int width = static_cast<int>((getColumnWidth() - 10) * helper::devicePixelRatio(this));
            m_elements[m_currentDownloadIndex].scaledPixmap =
                m_elements[m_currentDownloadIndex].pixmap.scaledToWidth(width, Qt::SmoothTransformation);
            helper::setDevicePixelRatio(
                m_elements[m_currentDownloadIndex].scaledPixmap, helper::devicePixelRatio(this));
            m_elements[m_currentDownloadIndex].cellWidget->setImage(m_elements[m_currentDownloadIndex].scaledPixmap);
            m_elements[m_currentDownloadIndex].cellWidget->setHint(
                m_elements[m_currentDownloadIndex].resolution, m_elements[m_currentDownloadIndex].hint);
        }
        ui->table->resizeRowsToContents();

    } else {
        showError(tr("Error while downloading one or more images: %1").arg(m_currentDownloadReply->errorString()));
        qWarning() << "Network Error: " << m_currentDownloadReply->errorString() << " | "
                   << m_currentDownloadReply->url();
    }

    // It is possible that m_elements has been cleared by aborting all downloads
    if (m_currentDownloadIndex < m_elements.size()) {
        // Mark item as downloaded even if there was an error to avoid an infinite loop.
        m_elements[m_currentDownloadIndex].downloaded = true;
    }
    m_currentDownloadReply->deleteLater();
    startNextDownload();
}

/**
 * @brief Renders the table
 */
void ImageDialog::renderTable()
{
    const int cols = calcColumnCount();
    ui->table->setColumnCount(cols);
    ui->table->setRowCount(0);
    ui->table->clearContents();

    for (int i = 0, n = ui->table->columnCount(); i < n; i++) {
        ui->table->setColumnWidth(i, getColumnWidth());
    }

    for (int i = 0, n = m_elements.size(); i < n; i++) {
        int row = (i - (i % cols)) / cols;
        if (i % cols == 0) {
            ui->table->insertRow(row);
        }
        auto item = new QTableWidgetItem;
        item->setData(Qt::UserRole, m_elements[i].originalUrl);
        auto label = new ImageLabel(ui->table);
        if (!m_elements[i].pixmap.isNull()) {
            const int width = static_cast<int>((getColumnWidth() - 10) * helper::devicePixelRatio(this));
            QPixmap pixmap = m_elements[i].pixmap.scaledToWidth(width, Qt::SmoothTransformation);
            helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
            label->setImage(pixmap);
            label->setHint(m_elements[i].resolution, m_elements[i].hint);
        }
        m_elements[i].cellWidget = label;
        ui->table->setItem(row, i % cols, item);
        ui->table->setCellWidget(row, i % cols, label);
        ui->table->resizeRowToContents(row);
    }
}

/**
 * @brief Calculates the number of columns that can be displayed
 * @return Number of columns that fit in the layout
 */
int ImageDialog::calcColumnCount()
{
    auto tableWidth = static_cast<qreal>(ui->table->size().width());
    auto columnWidth = static_cast<qreal>(getColumnWidth() + 4);
    return qFloor(tableWidth / columnWidth);
}

/**
 * @brief Returns the list of one column (based on the value of the slider)
 * @return Width of one column
 */
int ImageDialog::getColumnWidth()
{
    return ui->previewSizeSlider->value() * 16;
}

/**
 * @brief Called when an image was clicked
 * Saves the URL of the image and accepts the dialog
 * @param row Row of the image
 * @param col Column of the image
 */
void ImageDialog::imageClicked(int row, int col)
{
    if (ui->table->item(row, col) == nullptr) {
        qDebug() << "Invalid item";
        return;
    }
    QUrl url = ui->table->item(row, col)->data(Qt::UserRole).toUrl();
    m_imageUrl = url;
    if (m_multiSelection) {
        if ((ui->table->cellWidget(row, col) != nullptr) && !m_imageUrls.contains(url)) {
            m_imageUrls.append(url);
            auto* imgLabel = dynamic_cast<ImageLabel*>(ui->table->cellWidget(row, col));
            if (imgLabel != nullptr) {
                QByteArray ba;
                QBuffer buffer(&ba);
                QImage img = imgLabel->image();
                img.save(&buffer, "jpg", 100);
                ui->gallery->addImage(ba, url.toString());
            }
        }
    } else {
        accept();
    }
}

/**
 * @brief Sets the type of images
 * @param type Type of images
 */
void ImageDialog::setImageType(ImageType type)
{
    m_imageType = type;
}

/**
 * @brief Sets the current movie
 */
void ImageDialog::setMovie(Movie* movie)
{
    m_movie = movie;
    m_itemType = ItemType::Movie;
}

/**
 * @brief Sets the current concert
 */
void ImageDialog::setConcert(Concert* concert)
{
    m_concert = concert;
    m_itemType = ItemType::Concert;
}

/**
 * @brief Sets the current TV show
 */
void ImageDialog::setTvShow(TvShow* show)
{
    m_tvShow = show;
    m_itemType = ItemType::TvShow;
}

/**
 * @brief Set season number
 */
void ImageDialog::setSeason(SeasonNumber season)
{
    m_season = season;
}

/**
 * @brief Sets the current TV show episode
 */
void ImageDialog::setTvShowEpisode(TvShowEpisode* episode)
{
    m_tvShowEpisode = episode;
    m_itemType = ItemType::TvShowEpisode;
}

void ImageDialog::setArtist(Artist* artist)
{
    m_artist = artist;
    m_itemType = ItemType::Artist;
}

void ImageDialog::setAlbum(Album* album)
{
    m_album = album;
    m_itemType = ItemType::Album;
}

/**
 * @brief Cancels the current download and clears the download queue
 */
void ImageDialog::cancelDownloads()
{
    qDebug() << "Entered";
    ui->labelLoading->setVisible(false);
    ui->labelSpinner->setVisible(false);
    bool running = false;
    for (const DownloadElement& d : m_elements) {
        if (!d.downloaded) {
            running = true;
            break;
        }
    }
    m_elements.clear();
    if (running) {
        m_currentDownloadReply->abort();
    }
}

/**
 * @brief Called when a local image should be chosen
 */
void ImageDialog::chooseLocalImage()
{
    QString path = Settings::instance()->lastImagePath();

    QString fileName =
        QFileDialog::getOpenFileName(parentWidget(), tr("Choose Image"), path, tr("Images (*.jpg *.jpeg *.png)"));

    if (fileName.isEmpty()) {
        return;
    }

    qWarning() << fileName;

    QFileInfo fi(fileName);
    Settings::instance()->setLastImagePath(fi.absoluteDir().canonicalPath());
    const int index = m_elements.size();

    DownloadElement d;
    d.originalUrl = fileName;
    d.thumbUrl = fileName;
    d.downloaded = false;
    m_elements.append(d);

    renderTable();
    m_elements[index].pixmap = QPixmap(fileName);
    m_elements[index].pixmap = m_elements[index].pixmap.scaledToWidth(getColumnWidth() - 10, Qt::SmoothTransformation);
    m_elements[index].cellWidget->setImage(m_elements[index].pixmap);
    m_elements[index].cellWidget->setHint(m_elements[index].pixmap.size());
    ui->table->resizeRowsToContents();
    m_elements[index].downloaded = true;
    if (m_multiSelection) {
        QByteArray ba;
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            ba = file.readAll();
            file.close();
        }
        ui->gallery->addImage(ba, fileName);

        m_imageUrls.append(QUrl::fromLocalFile(fileName));
    } else {
        m_imageUrl = QUrl::fromLocalFile(fileName);
        accept();
    }
}

/**
 * @brief Called when an image was dropped
 * @param url URL of the dropped image
 */
void ImageDialog::onImageDropped(QUrl url)
{
    qDebug() << "[ImageDialog] Dropped Image with url:" << url;

    const int index = m_elements.size();

    DownloadElement d;
    d.originalUrl = url;
    d.thumbUrl = url;
    d.downloaded = false;
    m_elements.append(d);

    renderTable();

    if (url.toString().startsWith("file://")) {
        m_elements[index].pixmap = QPixmap(url.toLocalFile());
        m_elements[index].pixmap =
            m_elements[index].pixmap.scaledToWidth(getColumnWidth() - 10, Qt::SmoothTransformation);
        m_elements[index].cellWidget->setImage(m_elements[index].pixmap);
        m_elements[index].cellWidget->setHint(m_elements[index].pixmap.size());
    }
    ui->table->resizeRowsToContents();
    m_elements[index].downloaded = true;
    if (m_multiSelection) {
        QByteArray ba;
        QFile file(url.toLocalFile());
        if (file.open(QIODevice::ReadOnly)) {
            ba = file.readAll();
            file.close();
        }
        ui->gallery->addImage(ba, url.toLocalFile());
        m_imageUrls.append(url);
    } else {
        m_imageUrl = url;
        accept();
    }
}

/**
 * @brief Called when the preview size slider was moved
 * @param value Current value of the slider
 */
void ImageDialog::onPreviewSizeChange(int value)
{
    ui->buttonZoomOut->setDisabled(value == ui->previewSizeSlider->minimum());
    ui->buttonZoomIn->setDisabled(value == ui->previewSizeSlider->maximum());
    Settings::instance()->settings()->setValue(
        QString("ImageDialog/PreviewSize_%1").arg(static_cast<int>(m_type)), value);
    renderTable();
}

/**
 * @brief Increases the value of the preview size slider
 */
void ImageDialog::onZoomIn()
{
    ui->previewSizeSlider->setValue(ui->previewSizeSlider->value() + 1);
}

/**
 * @brief Decreases the value of the preview size slider
 */
void ImageDialog::onZoomOut()
{
    ui->previewSizeSlider->setValue(ui->previewSizeSlider->value() - 1);
}

/**
 * @brief If default provider is chosen, sets default downloads and disabled the search input
 * @param index Current provider index
 */
void ImageDialog::onProviderChanged(int index)
{
    if (index < 0 || index >= ui->imageProvider->count()) {
        return;
    }

    updateSourceLink();

    const bool isDefaultProvider = ui->imageProvider->itemData(index, DataRole::isDefaultProvider).toBool();

    ui->searchTerm->setReadOnly(isDefaultProvider);
    ui->searchTerm->setEnabled(!isDefaultProvider);

    if (isDefaultProvider) {
        // this is the default provider
        ui->stackedWidget->setCurrentIndex(1);
        ui->searchTerm->setLoading(false);
        clearSearch();
        setDownloads(m_defaultElements);
    } else {
        ui->searchTerm->setFocus();
        onSearch();
    }
}

void ImageDialog::updateSourceLink()
{
    int index = ui->imageProvider->currentIndex();
    if (index < 0 || index >= ui->imageProvider->count()) {
        return;
    }

    ui->lblErrorMessage->setVisible(false);
    ui->lblSuccessMessage->setVisible(false);

    if (ui->imageProvider->itemData(index, DataRole::isDefaultProvider).toBool()) {
        ui->imageSource->setVisible(false);
        ui->noResultsLabel->setText(tr("No images found"));

    } else {
        auto p = ui->imageProvider->itemData(ui->imageProvider->currentIndex(), DataRole::providerPointer)
                     .value<ImageProviderInterface*>();
        ui->imageSource->setText(tr("Images provided by <a href=\"%1\">%1</a>").arg(p->siteUrl().toString()));
        ui->imageSource->setVisible(true);
        ui->noResultsLabel->setText(
            tr("No images found") + "<br />"
            + tr("Contribute by uploading images to <a href=\"%1\">%1</a>").arg(p->siteUrl().toString()));
    }
}

/// Tells the current provider to search
/// @param onlyFirstResult If true, the results are limited to one
void ImageDialog::onSearch(bool onlyFirstResult)
{
    QString searchTerm = ui->searchTerm->text();
    if (searchTerm.startsWith("http://") || searchTerm.startsWith("https://")) {
        clearSearch();
        m_imageUrl = searchTerm;
        Poster poster;
        poster.originalUrl = searchTerm;
        poster.thumbUrl = searchTerm;
        onProviderImagesLoaded({poster}, {});
        return;
    }

    if (!hasImageProvider()) {
        return;
    }

    if (hasDefaultImages() && ui->imageProvider->currentIndex() == 0) {
        // "Default" selected
        return;
    }

    ui->stackedWidget->setCurrentIndex(1);
    QString initialSearchTerm;
    QString id;
    if (m_itemType == ItemType::Movie) {
        initialSearchTerm = m_movie->name();
        id = m_movie->tmdbId().toString();
    } else if (m_itemType == ItemType::Concert) {
        initialSearchTerm = m_concert->name();
        id = m_concert->tmdbId().toString();
    } else if (m_itemType == ItemType::TvShow) {
        initialSearchTerm = m_tvShow->name();
        id = m_tvShow->tvdbId().toString();
    } else if (m_itemType == ItemType::TvShowEpisode) {
        initialSearchTerm = m_tvShowEpisode->tvShow()->name();
        id = m_tvShowEpisode->tvShow()->tvdbId().toString();
    } else if (m_itemType == ItemType::Album) {
        initialSearchTerm = m_album->title();
        id = m_album->mbReleaseGroupId();
    } else if (m_itemType == ItemType::Artist) {
        initialSearchTerm = m_artist->name();
        id = m_artist->mbId();
    }

    clearSearch();
    ui->searchTerm->setLoading(true);
    m_currentProvider = ui->imageProvider->itemData(ui->imageProvider->currentIndex(), DataRole::providerPointer)
                            .value<ImageProviderInterface*>();

    if (!initialSearchTerm.isEmpty() && searchTerm == initialSearchTerm && !id.isEmpty()) {
        // search term was not changed and we have an id
        // -> trigger loading of images and show image widget
        ui->searchTerm->setLoading(false);
        loadImagesFromProvider(id);

    } else {
        // manual search term change or id is empty
        // -> trigger searching for item and show search result widget
        ui->results->clearContents();
        ui->results->setRowCount(0);
        int limit = (onlyFirstResult) ? 1 : 0;
        if (m_itemType == ItemType::Movie) {
            m_currentProvider->searchMovie(searchTerm, limit);
        } else if (m_itemType == ItemType::Concert) {
            m_currentProvider->searchConcert(searchTerm, limit);
        } else if (m_itemType == ItemType::TvShow || m_itemType == ItemType::TvShowEpisode) {
            m_currentProvider->searchTvShow(searchTerm, limit);
        } else if (m_itemType == ItemType::Artist) {
            m_currentProvider->searchArtist(searchTerm, limit);
        } else if (m_itemType == ItemType::Album) {
            m_currentProvider->searchAlbum(m_album->artist(), searchTerm, limit);
        }
    }
}

/// Alias for onSearch(false). Used for signal/slot connection
void ImageDialog::onSearchWithAllResults()
{
    onSearch(false);
}

/**
 * @brief Fills the results table
 * @param results List of results
 */
void ImageDialog::onSearchFinished(QVector<ScraperSearchResult> results, ScraperSearchError error)
{
    ui->searchTerm->setLoading(false);

    if (error.hasError()) {
        showError(error.message);
    } else if (results.size() > 1) {
        // special case for 1 result  => load images automatically
        //                  0 results => message in center of dialog
        showSuccess(tr("Found %n results", "", results.size()));
    }

    for (const ScraperSearchResult& result : results) {
        QString name = result.name;
        if (result.released.isValid()) {
            name.append(QString(" (%1)").arg(result.released.toString("yyyy")));
        }

        auto item = new QTableWidgetItem(name);
        item->setData(Qt::UserRole, result.id);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }

    // if there is only one result, take it
    if (ui->results->rowCount() == 1) {
        onResultClicked(ui->results->item(0, 0));
    } else if (ui->results->rowCount() == 0) {
        ui->stackedWidget->setCurrentIndex(2);
    } else {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

/**
 * @brief Triggers loading of images from the current provider
 */
void ImageDialog::loadImagesFromProvider(QString id)
{
    ui->lblSuccessMessage->hide();
    ui->lblErrorMessage->hide();
    ui->labelLoading->setVisible(true);
    ui->labelSpinner->setVisible(true);
    if (m_itemType == ItemType::Movie) {
        TmdbId movieId = TmdbId(id);
        if (m_type == ImageType::MoviePoster) {
            m_currentProvider->moviePosters(movieId);
        } else if (m_type == ImageType::MovieBackdrop) {
            m_currentProvider->movieBackdrops(movieId);
        } else if (m_type == ImageType::MovieLogo) {
            m_currentProvider->movieLogos(movieId);
        } else if (m_type == ImageType::MovieBanner) {
            m_currentProvider->movieBanners(movieId);
        } else if (m_type == ImageType::MovieThumb) {
            m_currentProvider->movieThumbs(movieId);
        } else if (m_type == ImageType::MovieClearArt) {
            m_currentProvider->movieClearArts(movieId);
        } else if (m_type == ImageType::MovieCdArt) {
            m_currentProvider->movieCdArts(movieId);
        }
    } else if (m_itemType == ItemType::Concert) {
        TmdbId movieId = TmdbId(id);
        if (m_type == ImageType::ConcertBackdrop) {
            m_currentProvider->concertBackdrops(movieId);
        } else if (m_type == ImageType::ConcertPoster) {
            m_currentProvider->concertPosters(movieId);
        } else if (m_type == ImageType::ConcertLogo) {
            m_currentProvider->concertLogos(movieId);
        } else if (m_type == ImageType::ConcertClearArt) {
            m_currentProvider->concertClearArts(movieId);
        } else if (m_type == ImageType::ConcertCdArt) {
            m_currentProvider->concertCdArts(movieId);
        }
    } else if (m_itemType == ItemType::TvShow) {
        TvDbId showId = TvDbId(id);
        if (m_type == ImageType::TvShowBackdrop) {
            m_currentProvider->tvShowBackdrops(showId);
        } else if (m_type == ImageType::TvShowBanner) {
            m_currentProvider->tvShowBanners(showId);
        } else if (m_type == ImageType::TvShowCharacterArt) {
            m_currentProvider->tvShowCharacterArts(showId);
        } else if (m_type == ImageType::TvShowClearArt) {
            m_currentProvider->tvShowClearArts(showId);
        } else if (m_type == ImageType::TvShowLogos) {
            m_currentProvider->tvShowLogos(showId);
        } else if (m_type == ImageType::TvShowThumb) {
            m_currentProvider->tvShowThumbs(showId);
        } else if (m_type == ImageType::TvShowPoster) {
            m_currentProvider->tvShowPosters(showId);
        } else if (m_type == ImageType::TvShowSeasonPoster) {
            m_currentProvider->tvShowSeason(showId, m_season);
        } else if (m_type == ImageType::TvShowSeasonBanner) {
            m_currentProvider->tvShowSeasonBanners(showId, m_season);
        } else if (m_type == ImageType::TvShowSeasonThumb) {
            m_currentProvider->tvShowSeasonThumbs(showId, m_season);
        } else if (m_type == ImageType::TvShowSeasonBackdrop) {
            m_currentProvider->tvShowSeasonBackdrops(showId, m_season);
        }
    } else if (m_itemType == ItemType::TvShowEpisode) {
        TvDbId showId = TvDbId(id);
        if (m_type == ImageType::TvShowEpisodeThumb) {
            m_currentProvider->tvShowEpisodeThumb(showId, m_tvShowEpisode->season(), m_tvShowEpisode->episode());
        }
    } else if (m_itemType == ItemType::Artist) {
        if (m_type == ImageType::ArtistFanart) {
            m_currentProvider->artistFanarts(id);
        } else if (m_type == ImageType::ArtistLogo) {
            m_currentProvider->artistLogos(id);
        } else if (m_type == ImageType::ArtistThumb) {
            m_currentProvider->artistThumbs(id);
        }
    } else if (m_itemType == ItemType::Album) {
        if (m_type == ImageType::AlbumCdArt) {
            m_currentProvider->albumCdArts(id);
        } else if (m_type == ImageType::AlbumThumb) {
            m_currentProvider->albumThumbs(id);
        } else if (m_type == ImageType::AlbumBooklet) {
            m_currentProvider->albumBooklets(id);
        }
    }
}

/**
 * @brief Triggers loading of images
 */
void ImageDialog::onResultClicked(QTableWidgetItem* item)
{
    ui->stackedWidget->setCurrentIndex(1);
    loadImagesFromProvider(item->data(Qt::UserRole).toString());
}

/**
 * @brief Called when the image provider has finished loading
 * @param images List of images
 */
void ImageDialog::onProviderImagesLoaded(QVector<Poster> images, ScraperLoadError error)
{
    if (error.hasError()) {
        showError(tr("Error while querying image provider: %1").arg(error.message));
    }
    setDownloads(images, false);
}

void ImageDialog::setMultiSelection(const bool& enable)
{
    m_multiSelection = enable;
    ui->gallery->setVisible(enable);
    ui->btnAcceptImages->setVisible(enable);
}

QVector<QUrl> ImageDialog::imageUrls()
{
    return m_imageUrls;
}

void ImageDialog::onImageClosed(const QString& url)
{
    m_imageUrls.removeOne(url);
}

QString ImageDialog::formatSearchText(const QString& text)
{
    QString fText = text;
    fText.replace(" - ", " ");
    fText.replace("-", " ");
    fText = NameFormatter::instance()->formatName(fText);
    return fText;
}

void ImageDialog::showError(const QString& message)
{
    ui->lblSuccessMessage->setVisible(false);
    ui->lblErrorMessage->setVisible(true);
    ui->lblErrorMessage->setText(message);
}

void ImageDialog::showSuccess(const QString& message)
{
    ui->lblErrorMessage->setVisible(false);
    ui->lblSuccessMessage->setVisible(true);
    ui->lblSuccessMessage->setText(message);
}
