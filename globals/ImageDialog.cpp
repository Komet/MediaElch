#include "ImageDialog.h"
#include "ui_ImageDialog.h"

#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QtCore/qmath.h>
#include <QLabel>
#include <QMovie>
#include <QPainter>
#include <QSize>
#include <QTimer>

/**
 * @brief ImageDialog::ImageDialog
 * @param parent
 */
ImageDialog::ImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageDialog)
{
    ui->setupUi(this);

#ifdef Q_WS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #ImageDialog { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QSettings settings;
    resize(settings.value("ImageDialog/Size").toSize());

    connect(ui->table, SIGNAL(cellClicked(int,int)), this, SLOT(imageClicked(int, int)));
    connect(ui->table, SIGNAL(sigDroppedImage(QUrl)), this, SLOT(onImageDropped(QUrl)));
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->buttonChoose, SIGNAL(clicked()), this, SLOT(chooseLocalImage()));
    connect(ui->previewSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(onPreviewSizeChange(int)));
    connect(ui->buttonZoomIn, SIGNAL(clicked()), this, SLOT(onZoomIn()));
    connect(ui->buttonZoomOut, SIGNAL(clicked()), this, SLOT(onZoomOut()));
    QMovie *movie = new QMovie(":/img/spinner.gif");
    movie->start();
    ui->labelSpinner->setMovie(movie);
    clear();
    setImageType(TypePoster);
    m_currentDownloadReply = 0;

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

    m_noElementsLabel = new QLabel(tr("No images found"), ui->table);
    m_noElementsLabel->setMargin(10);
    m_noElementsLabel->hide();
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
    QSettings settings;
    ui->previewSizeSlider->setValue(settings.value(QString("ImageDialog/PreviewSize_%1").arg(m_type), 8).toInt());
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-50);
    newSize.setWidth(qMin(1200, parentWidget()->size().width()-100));
    resize(newSize);

    int xMove = (parentWidget()->size().width()-size().width())/2;
    QPoint globalPos = parentWidget()->mapToGlobal(parentWidget()->pos());
    move(globalPos.x()+xMove, globalPos.y());

    return QDialog::exec();
}

/**
 * @brief Accepts the dialog and saves the size of the preview images
 */
void ImageDialog::accept()
{
    qDebug() << "Entered";
    cancelDownloads();
    QSettings settings;
    settings.setValue("ImageDialog/Size", size());
    QDialog::accept();
}

/**
 * @brief Rejects the dialog and saves the size of the preview images
 */
void ImageDialog::reject()
{
    qDebug() << "Entered";
    cancelDownloads();
    QSettings settings;
    settings.setValue("ImageDialog/Size", size());
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
    qDebug() << "Entered";
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
 */
void ImageDialog::setDownloads(QList<Poster> downloads)
{
    qDebug() << "Entered";
    foreach (const Poster &poster, downloads) {
        DownloadElement d;
        d.originalUrl = poster.originalUrl;
        d.thumbUrl = poster.thumbUrl;
        d.downloaded = false;
        d.resolution = poster.originalSize;
        m_elements.append(d);
    }
    ui->labelLoading->setVisible(true);
    ui->labelSpinner->setVisible(true);
    startNextDownload();
    renderTable();
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
        m_elements[m_currentDownloadIndex].cellWidget->setResolution(m_elements[m_currentDownloadIndex].resolution);
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
    qDebug() << "Entered";
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
            label->setResolution(m_elements[i].resolution);
        }
        m_elements[i].cellWidget = label;
        ui->table->setItem(row, i%cols, item);
        ui->table->setCellWidget(row, i%cols, label);
        ui->table->resizeRowToContents(row);
    }

    m_noElementsLabel->setVisible(m_elements.size() == 0);
}

/**
 * @brief Calculates the number of columns that can be displayed
 * @return Number of columns that fit in the layout
 */
int ImageDialog::calcColumnCount()
{
    qDebug() << "Entered";
    int width = ui->table->size().width();
    int colWidth = getColumnWidth()+4;
    int cols = qFloor((qreal)width/colWidth);
    qDebug() << "Returning cols=" << cols;
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
    qDebug() << "Entered";
    if (ui->table->item(row, col) == 0) {
        qDebug() << "Invalid item";
        return;
    }
    QUrl url = ui->table->item(row, col)->data(Qt::UserRole).toUrl();
    m_imageUrl = url;
    accept();
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
    qDebug() << "Entered";
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
        m_elements[index].cellWidget->setResolution(m_elements[index].pixmap.size());
        ui->table->resizeRowsToContents();
        m_elements[index].downloaded = true;
        m_imageUrl = QUrl(fileName);
        accept();
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
        m_elements[index].cellWidget->setResolution(m_elements[index].pixmap.size());
    }
    ui->table->resizeRowsToContents();
    m_elements[index].downloaded = true;
    m_imageUrl = url;
    accept();
}

/**
 * @brief Called when the preview size slider was moved
 * @param value Current value of the slider
 */
void ImageDialog::onPreviewSizeChange(int value)
{
    qDebug() << "Entered, value=" << value;
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
