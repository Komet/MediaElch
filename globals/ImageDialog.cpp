#include "ImageDialog.h"
#include "ui_ImageDialog.h"

#include <QBuffer>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QtCore/qmath.h>
#include <QLabel>
#include <QMovie>
#include <QPainter>
#include <QSize>
#include <QTimer>
#include "data/ImageProviderInterface.h"
#include "globals/Manager.h"
#include "globals/NameFormatter.h"

/**
 * @brief ImageDialog::ImageDialog
 * @param parent
 */
ImageDialog::ImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageDialog)
{
    ui->setupUi(this);
    ui->searchTerm->setType(MyLineEdit::TypeLoading);
#if QT_VERSION >= 0x050000
    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    ui->results->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    ui->gallery->setAlignment(Qt::Horizontal);
    ui->gallery->setShowZoomAndResolution(false);

    QSettings settings;
    resize(settings.value("ImageDialog/Size").toSize());

    connect(ui->table, SIGNAL(cellClicked(int,int)), this, SLOT(imageClicked(int, int)));
    connect(ui->table, SIGNAL(sigDroppedImage(QUrl)), this, SLOT(onImageDropped(QUrl)));
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->buttonChoose, SIGNAL(clicked()), this, SLOT(chooseLocalImage()));
    connect(ui->previewSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(onPreviewSizeChange(int)));
    connect(ui->buttonZoomIn, SIGNAL(clicked()), this, SLOT(onZoomIn()));
    connect(ui->buttonZoomOut, SIGNAL(clicked()), this, SLOT(onZoomOut()));
    connect(ui->searchTerm, SIGNAL(returnPressed()), this, SLOT(onSearch()));
    connect(ui->imageProvider, SIGNAL(currentIndexChanged(int)), this, SLOT(onProviderChanged(int)));
    connect(ui->results, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onResultClicked(QTableWidgetItem*)));
    connect(ui->gallery, SIGNAL(sigRemoveImage(QString)), this, SLOT(onImageClosed(QString)));
    connect(ui->btnAcceptImages, SIGNAL(clicked()), this, SLOT(accept()));

    ui->btnAcceptImages->hide();

    QMovie *movie = new QMovie(":/img/spinner.gif");
    movie->start();
    ui->labelSpinner->setMovie(movie);
    clearSearch();
    setImageType(TypePoster);
    m_currentDownloadReply = 0;
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

    foreach (ImageProviderInterface *provider, Manager::instance()->imageProviders()) {
        connect(provider, SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchFinished(QList<ScraperSearchResult>)));
        connect(provider, SIGNAL(sigImagesLoaded(QList<Poster>)), this, SLOT(onProviderImagesLoaded(QList<Poster>)));
    }
}

/**
 * @brief ImageDialog::~ImageDialog
 */
ImageDialog::~ImageDialog()
{
    delete ui;
}

/**
 * @brief Executes the dialog and returns the result of QDialog::exec
 * @param type Type of the images (ImageDialogType)
 * @return Result of QDialog::exec
 */
int ImageDialog::exec(int type)
{
    qDebug() << "Entered, type=" << type;
    m_type = type;

    // set slider value
    QSettings settings;
    ui->previewSizeSlider->setValue(settings.value(QString("ImageDialog/PreviewSize_%1").arg(m_type), 8).toInt());

    QSize savedSize = settings.value("ImageDialog/Size").toSize();
    QPoint savedPos = settings.value("ImageDialog/Pos").toPoint();

    bool isMac = false;
#ifdef Q_OS_MAC
    isMac = true;
#endif

    if (savedSize.isValid() && !savedSize.isNull() && !isMac) {
        resize(savedSize);
    } else {
        // resize
        QSize newSize;
        newSize.setHeight(parentWidget()->size().height()-50);
        newSize.setWidth(qMin(1200, parentWidget()->size().width()-100));
        resize(newSize);
    }

    if (!savedPos.isNull() && !isMac) {
        move(savedPos);
    } else {
        // move to center
        int xMove = (parentWidget()->size().width()-size().width())/2;
        QPoint globalPos = parentWidget()->mapToGlobal(parentWidget()->pos());
        move(globalPos.x()+xMove, qMax(0, globalPos.y()-100));
    }

    // get image providers and setup combo box
    m_providers = Manager::instance()->imageProviders(type);
    bool haveDefault = m_defaultElements.count() > 0 || m_providers.isEmpty();
    ui->imageProvider->blockSignals(true);
    ui->imageProvider->clear();
    if (haveDefault) {
        ui->imageProvider->addItem(tr("Default"));
        ui->imageProvider->setItemData(0, true, Qt::UserRole+1);
    }
    foreach (ImageProviderInterface *provider, m_providers) {
        int row = ui->imageProvider->count();
        ui->imageProvider->addItem(provider->name());
        ui->imageProvider->setItemData(row, QVariant::fromValue(provider), Qt::UserRole);
        ui->imageProvider->setItemData(row, false, Qt::UserRole+1);
    }
    ui->imageProvider->blockSignals(false);

    ui->searchTerm->setLoading(false);
    ui->searchTerm->setEnabled(!haveDefault);

    // show image widget
    ui->stackedWidget->setCurrentIndex(1);

    if (m_itemType == ItemMovie)
        ui->searchTerm->setText(formatSearchText(m_movie->name()));
    else if (m_itemType == ItemConcert)
        ui->searchTerm->setText(formatSearchText(m_concert->name()));
    else if (m_itemType == ItemTvShow)
        ui->searchTerm->setText(formatSearchText(m_tvShow->name()));
    else if (m_itemType == ItemTvShowEpisode)
        ui->searchTerm->setText(formatSearchText(m_tvShowEpisode->tvShow()->name()));
    else
        ui->searchTerm->clear();

    if (!haveDefault)
        onSearch(true);

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
    QSettings settings;
    settings.setValue("ImageDialog/Size", size());
    settings.setValue("ImageDialog/Pos", pos());
    settings.sync();
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
    QSettings settings;
    settings.setValue("ImageDialog/Size", size());
    settings.setValue("ImageDialog/Pos", pos());
    settings.sync();
#endif
    QDialog::reject();
}

/**
 * @brief Returns an instance of ImageDialog
 * @param parent Parent widget (used the first time for constructing)
 * @return Instance of ImageDialog
 */
ImageDialog *ImageDialog::instance(QWidget *parent)
{
    static ImageDialog *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new ImageDialog(parent);
    }
    return m_instance;
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
 * @param event
 */
void ImageDialog::resizeEvent(QResizeEvent *event)
{
    if (calcColumnCount() != ui->table->columnCount())
        renderTable();
    QWidget::resizeEvent(event);
}

/**
 * @brief Sets a list of images to be downloaded and shown
 * @param downloads List of images (downloads)
 * @param initial If true saves downloads as defaults
 */
void ImageDialog::setDownloads(QList<Poster> downloads, bool initial)
{
    qDebug() << "Entered";
    ui->stackedWidget->setCurrentIndex(1);
    if (initial)
        m_defaultElements = downloads;
    foreach (const Poster &poster, downloads) {
        DownloadElement d;
        d.originalUrl = poster.originalUrl;
        d.thumbUrl = poster.thumbUrl;
        d.downloaded = false;
        d.resolution = poster.originalSize;
        d.hint = poster.hint;
        m_elements.append(d);
    }
    ui->labelLoading->setVisible(true);
    ui->labelSpinner->setVisible(true);
    startNextDownload();
    renderTable();
    if (downloads.count() == 0)
        ui->stackedWidget->setCurrentIndex(2);
}

/**
 * @brief Returns an instance of a network access manager
 * @return Instance of a network access manager
 */
QNetworkAccessManager *ImageDialog::qnam()
{
    return &m_qnam;
}

/**
 * @brief Starts the next download if there is one
 */
void ImageDialog::startNextDownload()
{
    qDebug() << "Entered";
    int nextIndex = -1;
    for (int i=0, n=m_elements.size() ; i<n ; i++) {
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
    connect(m_currentDownloadReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
}

/**
 * @brief Called when a download has finished
 * Renders the table and displays the downloaded image and starts the next download
 */
void ImageDialog::downloadFinished()
{
    qDebug() << "Entered";
    if (m_currentDownloadReply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << m_currentDownloadReply->errorString();
        startNextDownload();
        return;
    }

    m_elements[m_currentDownloadIndex].pixmap.loadFromData(m_currentDownloadReply->readAll());
    if (!m_elements[m_currentDownloadIndex].pixmap.isNull()) {
        m_elements[m_currentDownloadIndex].scaledPixmap = m_elements[m_currentDownloadIndex].pixmap.scaledToWidth(getColumnWidth()-10, Qt::SmoothTransformation);
        m_elements[m_currentDownloadIndex].cellWidget->setImage(m_elements[m_currentDownloadIndex].scaledPixmap);
        m_elements[m_currentDownloadIndex].cellWidget->setHint(m_elements[m_currentDownloadIndex].resolution, m_elements[m_currentDownloadIndex].hint);
    }
    ui->table->resizeRowsToContents();
    m_elements[m_currentDownloadIndex].downloaded = true;
    m_currentDownloadReply->deleteLater();
    startNextDownload();
}

/**
 * @brief Renders the table
 */
void ImageDialog::renderTable()
{
    int cols = calcColumnCount();
    ui->table->setColumnCount(cols);
    ui->table->setRowCount(0);
    ui->table->clearContents();

    for (int i=0, n=ui->table->columnCount() ; i<n ; i++)
        ui->table->setColumnWidth(i, getColumnWidth());

    for (int i=0, n=m_elements.size() ; i<n ; i++) {
        int row = (i-(i%cols))/cols;
        if (i%cols == 0)
            ui->table->insertRow(row);
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(Qt::UserRole, m_elements[i].originalUrl);
        ImageLabel *label = new ImageLabel(ui->table);
        if (!m_elements[i].pixmap.isNull()) {
            label->setImage(m_elements[i].pixmap.scaledToWidth(getColumnWidth()-10, Qt::SmoothTransformation));
            label->setHint(m_elements[i].resolution, m_elements[i].hint);
        }
        m_elements[i].cellWidget = label;
        ui->table->setItem(row, i%cols, item);
        ui->table->setCellWidget(row, i%cols, label);
        ui->table->resizeRowToContents(row);
    }
}

/**
 * @brief Calculates the number of columns that can be displayed
 * @return Number of columns that fit in the layout
 */
int ImageDialog::calcColumnCount()
{
    int width = ui->table->size().width();
    int colWidth = getColumnWidth()+4;
    int cols = qFloor((qreal)width/colWidth);
    return cols;
}

/**
 * @brief Returns the list of one column (based on the value of the slider)
 * @return Width of one column
 */
int ImageDialog::getColumnWidth()
{
    return ui->previewSizeSlider->value()*16;
}

/**
 * @brief Called when an image was clicked
 * Saves the URL of the image and accepts the dialog
 * @param row Row of the image
 * @param col Column of the image
 */
void ImageDialog::imageClicked(int row, int col)
{
    if (ui->table->item(row, col) == 0) {
        qDebug() << "Invalid item";
        return;
    }
    QUrl url = ui->table->item(row, col)->data(Qt::UserRole).toUrl();
    m_imageUrl = url;
    if (m_multiSelection) {
        if (ui->table->cellWidget(row, col) && !m_imageUrls.contains(url)) {
            m_imageUrls.append(url);
            QByteArray ba;
            QBuffer buffer(&ba);
            QImage img = static_cast<ImageLabel*>(ui->table->cellWidget(row, col))->image();
            img.save(&buffer, "jpg", 100);
            ui->gallery->addImage(ba, url.toString());
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
 * @param movie
 */
void ImageDialog::setMovie(Movie *movie)
{
    m_movie = movie;
    m_itemType = ItemMovie;
}

/**
 * @brief Sets the current concert
 * @param concert
 */
void ImageDialog::setConcert(Concert *concert)
{
    m_concert = concert;
    m_itemType = ItemConcert;
}

/**
 * @brief Sets the current tv show
 * @param show
 */
void ImageDialog::setTvShow(TvShow *show)
{
    m_tvShow = show;
    m_itemType = ItemTvShow;
}

/**
 * @brief Set season number
 * @param season
 */
void ImageDialog::setSeason(int season)
{
    m_season = season;
}

/**
 * @brief Sets the current tv show episode
 * @param episode
 */
void ImageDialog::setTvShowEpisode(TvShowEpisode *episode)
{
    m_tvShowEpisode = episode;
    m_itemType = ItemTvShowEpisode;
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
    foreach (const DownloadElement &d, m_elements) {
        if (!d.downloaded) {
            running = true;
            break;
        }
    }
    m_elements.clear();
    if (running)
        m_currentDownloadReply->abort();
}

/**
 * @brief Called when a local image should be chosen
 */
void ImageDialog::chooseLocalImage()
{
    QString fileName = QFileDialog::getOpenFileName(parentWidget(), tr("Choose Image"), QDir::homePath(), tr("Images (*.jpg *.jpeg *.png)"));
    qDebug() << "Filename=" << fileName;
    if (!fileName.isNull()) {
        int index = m_elements.size();
        DownloadElement d;
        d.originalUrl = fileName;
        d.thumbUrl = fileName;
        d.downloaded = false;
        m_elements.append(d);
        renderTable();
        m_elements[index].pixmap = QPixmap(fileName);
        m_elements[index].pixmap = m_elements[index].pixmap.scaledToWidth(getColumnWidth()-10, Qt::SmoothTransformation);
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
            m_imageUrls.append(QUrl(fileName));
        } else {
            m_imageUrl = QUrl(fileName);
            accept();
        }
    }
}

/**
 * @brief Called when an image was dropped
 * @param url URL of the dropped image
 */
void ImageDialog::onImageDropped(QUrl url)
{
    qDebug() << "Entered, url=" << url;
    int index = m_elements.size();
    DownloadElement d;
    d.originalUrl = url;
    d.thumbUrl = url;
    d.downloaded = false;
    m_elements.append(d);
    renderTable();
    if (url.toString().startsWith("file://")) {
        m_elements[index].pixmap = QPixmap(url.toLocalFile());
        m_elements[index].pixmap = m_elements[index].pixmap.scaledToWidth(getColumnWidth()-10, Qt::SmoothTransformation);
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
    QSettings settings;
    settings.setValue(QString("ImageDialog/PreviewSize_%1").arg(m_type), value);
    renderTable();
}

/**
 * @brief Increases the value of the preview size slider
 */
void ImageDialog::onZoomIn()
{
    ui->previewSizeSlider->setValue(ui->previewSizeSlider->value()+1);
}

/**
 * @brief Decreases the value of the preview size slider
 */
void ImageDialog::onZoomOut()
{
    ui->previewSizeSlider->setValue(ui->previewSizeSlider->value()-1);
}

/**
 * @brief If default provider is chosen, sets default downloads and disabled the search input
 * @param index Current provider index
 */
void ImageDialog::onProviderChanged(int index)
{
    if (index < 0 || index >= ui->imageProvider->count())
        return;

    if (ui->imageProvider->itemData(index, Qt::UserRole+1).toBool()) {
        // this is the default provider
        ui->stackedWidget->setCurrentIndex(1);
        ui->searchTerm->setLoading(false);
        ui->searchTerm->setEnabled(false);
        clearSearch();
        setDownloads(m_defaultElements);
    } else {
        ui->searchTerm->setEnabled(true);
        ui->searchTerm->setFocus();
        onSearch();
    }
}

/**
 * @brief Tells the current provider to search
 * @param onlyFirstResult If true, the results are limited to one
 */
void ImageDialog::onSearch(bool onlyFirstResult)
{
    ui->stackedWidget->setCurrentIndex(1);
    QString searchTerm = ui->searchTerm->text();
    QString initialSearchTerm;
    QString id;
    if (m_itemType == ItemMovie) {
        initialSearchTerm = m_movie->name();
        id = m_movie->tmdbId();
    } else if (m_itemType == ItemConcert) {
        initialSearchTerm = m_concert->name();
        id = m_concert->tmdbId();
    } else if (m_itemType == ItemTvShow) {
        initialSearchTerm = m_tvShow->name();
        id = m_tvShow->tvdbId();
    } else if (m_itemType == ItemTvShowEpisode) {
        initialSearchTerm = m_tvShowEpisode->tvShow()->name();
        id = m_tvShowEpisode->tvShow()->tvdbId();
    }

    clearSearch();
    ui->searchTerm->setLoading(true);
    m_currentProvider = ui->imageProvider->itemData(ui->imageProvider->currentIndex(), Qt::UserRole).value<ImageProviderInterface*>();
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
        if (m_itemType == ItemMovie)
            m_currentProvider->searchMovie(searchTerm, limit);
        else if (m_itemType == ItemConcert)
            m_currentProvider->searchConcert(searchTerm, limit);
        else if (m_itemType == ItemTvShow || m_itemType == ItemTvShowEpisode)
            m_currentProvider->searchTvShow(searchTerm, limit);
    }
}

/**
 * @brief Fills the results table
 * @param results List of results
 */
void ImageDialog::onSearchFinished(QList<ScraperSearchResult> results)
{
    ui->searchTerm->setLoading(false);
    foreach (const ScraperSearchResult &result, results) {
        QString name = result.name;
        if (!result.released.isNull())
            name.append(QString(" (%1)").arg(result.released.toString("yyyy")));

        QTableWidgetItem *item = new QTableWidgetItem(name);
        item->setData(Qt::UserRole, result.id);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }

    // if there is only one result, take it
    if (ui->results->rowCount() == 1) {
        onResultClicked(ui->results->item(0, 0));
    } else {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

/**
 * @brief Triggers loading of images from the current provider
 * @param id
 */
void ImageDialog::loadImagesFromProvider(QString id)
{
    ui->labelLoading->setVisible(true);
    ui->labelSpinner->setVisible(true);
    if (m_itemType == ItemMovie) {
        if (m_type == ImageDialogType::MovieBackdrop)
            m_currentProvider->movieBackdrops(id);
        else if (m_type == ImageDialogType::MoviePoster)
            m_currentProvider->moviePosters(id);
        else if (m_type == ImageDialogType::MovieLogo)
            m_currentProvider->movieLogos(id);
        else if (m_type == ImageDialogType::MovieBanner)
            m_currentProvider->movieBanners(id);
        else if (m_type == ImageDialogType::MovieThumb)
            m_currentProvider->movieThumbs(id);
        else if (m_type == ImageDialogType::MovieClearArt)
            m_currentProvider->movieClearArts(id);
        else if (m_type == ImageDialogType::MovieCdArt)
            m_currentProvider->movieCdArts(id);
    } else if (m_itemType == ItemConcert) {
        if (m_type == ImageDialogType::ConcertBackdrop)
            m_currentProvider->concertBackdrops(id);
        else if (m_type == ImageDialogType::ConcertPoster)
            m_currentProvider->concertPosters(id);
        else if (m_type == ImageDialogType::ConcertLogo)
            m_currentProvider->concertLogos(id);
        else if (m_type == ImageDialogType::ConcertClearArt)
            m_currentProvider->concertClearArts(id);
        else if (m_type == ImageDialogType::ConcertCdArt)
            m_currentProvider->concertCdArts(id);
    } else if (m_itemType == ItemTvShow) {
        if (m_type == ImageDialogType::TvShowBackdrop)
            m_currentProvider->tvShowBackdrops(id);
        else if (m_type == ImageDialogType::TvShowBanner)
            m_currentProvider->tvShowBanners(id);
        else if (m_type == ImageDialogType::TvShowCharacterArt)
            m_currentProvider->tvShowCharacterArts(id);
        else if (m_type == ImageDialogType::TvShowClearArt)
            m_currentProvider->tvShowClearArts(id);
        else if (m_type == ImageDialogType::TvShowLogos)
            m_currentProvider->tvShowLogos(id);
        else if (m_type == ImageDialogType::TvShowPoster)
            m_currentProvider->tvShowPosters(id);
        else if (m_type == ImageDialogType::TvShowSeason)
            m_currentProvider->tvShowSeason(id, m_season);
        else if (m_type == ImageDialogType::TvShowSeasonBanner)
            m_currentProvider->tvShowSeasonBanners(id, m_season);
        else if (m_type == ImageDialogType::TvShowSeasonBackdrop)
            m_currentProvider->tvShowSeasonBackdrops(id, m_season);
    } else if (m_itemType == ItemTvShowEpisode) {
        if (m_type == ImageDialogType::TvShowThumb)
            m_currentProvider->tvShowThumb(id, m_tvShowEpisode->season(), m_tvShowEpisode->episode());
    }

}

/**
 * @brief Triggers loading of images
 * @param item
 */
void ImageDialog::onResultClicked(QTableWidgetItem *item)
{
    ui->stackedWidget->setCurrentIndex(1);
    loadImagesFromProvider(item->data(Qt::UserRole).toString());
}

/**
 * @brief Called when the image provider has finished loading
 * @param images List of images
 */
void ImageDialog::onProviderImagesLoaded(QList<Poster> images)
{
    setDownloads(images, false);
}

void ImageDialog::setMultiSelection(const bool &enable)
{
    m_multiSelection = enable;
    ui->gallery->setVisible(enable);
    ui->btnAcceptImages->setVisible(enable);
}

QList<QUrl> ImageDialog::imageUrls()
{
    return m_imageUrls;
}

void ImageDialog::onImageClosed(const QString &url)
{
    m_imageUrls.removeOne(url);
}

QString ImageDialog::formatSearchText(const QString &text)
{
    QString fText = text;
    fText.replace(" - ", " ");
    fText.replace("-", " ");
    fText = NameFormatter::instance()->formatName(fText);
    return fText;
}
