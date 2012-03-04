#include "MovieImageDialog.h"
#include "ui_MovieImageDialog.h"

#include <QDebug>
#include <QSettings>
#include <QtCore/qmath.h>
#include <QLabel>
#include <QMovie>
#include <QSize>
#include <QTimer>

MovieImageDialog::MovieImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MovieImageDialog)
{
    ui->setupUi(this);

    Qt::WindowFlags flags = windowFlags();
#ifdef Q_WS_WIN
    flags |= Qt::Dialog;
#else
    flags |= Qt::SplashScreen;
    setStyleSheet(styleSheet() + " #MovieImageDialog { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#endif
    setWindowFlags(flags);

    QSettings settings;
    resize(settings.value("MovieImageDialog/Size").toSize());

    connect(ui->table, SIGNAL(cellClicked(int,int)), this, SLOT(imageClicked(int, int)));
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));
    QMovie *movie = new QMovie(":/img/spinner.gif");
    movie->start();
    ui->labelSpinner->setMovie(movie);
    this->clear();
    this->setImageType(TypePoster);
    m_currentDownloadReply = 0;
}

MovieImageDialog::~MovieImageDialog()
{
    delete ui;
}

int MovieImageDialog::exec()
{
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-200);
    newSize.setWidth(qMin(1000, parentWidget()->size().width()-200));
    resize(newSize);

    int xMove = (parentWidget()->size().width()-size().width())/2;
    QPoint globalPos = parentWidget()->mapToGlobal(parentWidget()->pos());
    move(globalPos.x()+xMove, globalPos.y());

    return QDialog::exec();
}

void MovieImageDialog::accept()
{
    this->cancelDownloads();
    QSettings settings;
    settings.setValue("MovieImageDialog/Size", size());
    QDialog::accept();
}

void MovieImageDialog::reject()
{
    this->cancelDownloads();
    QSettings settings;
    settings.setValue("MovieImageDialog/Size", size());
    QDialog::reject();
}

MovieImageDialog *MovieImageDialog::instance(QWidget *parent)
{
    static MovieImageDialog *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new MovieImageDialog(parent);
    }
    return m_instance;
}

void MovieImageDialog::clear()
{
    this->cancelDownloads();
    m_elements.clear();
    ui->table->clearContents();
    ui->table->setRowCount(0);
}

QUrl MovieImageDialog::imageUrl()
{
    return m_imageUrl;
}

void MovieImageDialog::resizeEvent(QResizeEvent *event)
{
    if (this->calcColumnCount() != ui->table->columnCount())
        this->renderTable();
    QWidget::resizeEvent(event);
}

void MovieImageDialog::setDownloads(QList<Poster> downloads)
{
    foreach (const Poster &poster, downloads) {
        DownloadElement d;
        d.originalUrl = poster.originalUrl;
        d.thumbUrl = poster.thumbUrl;
        d.downloaded = false;
        this->m_elements.append(d);
    }
    ui->labelLoading->setVisible(true);
    ui->labelSpinner->setVisible(true);
    this->startNextDownload();
    this->renderTable();
}

QNetworkAccessManager *MovieImageDialog::qnam()
{
    return &m_qnam;
}

void MovieImageDialog::startNextDownload()
{
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
    m_currentDownloadReply = this->qnam()->get(QNetworkRequest(m_elements[nextIndex].thumbUrl));
    connect(m_currentDownloadReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
}

void MovieImageDialog::downloadFinished()
{
    if (m_currentDownloadReply->error() != QNetworkReply::NoError) {
        this->startNextDownload();
        return;
    }

    m_elements[m_currentDownloadIndex].pixmap.loadFromData(m_currentDownloadReply->readAll());
    m_elements[m_currentDownloadIndex].pixmap = m_elements[m_currentDownloadIndex].pixmap.scaledToWidth(this->getColumnWidth()-10, Qt::SmoothTransformation);
    m_elements[m_currentDownloadIndex].cellWidget->setPixmap(m_elements[m_currentDownloadIndex].pixmap);
    ui->table->resizeRowsToContents();
    m_elements[m_currentDownloadIndex].downloaded = true;
    m_currentDownloadReply->deleteLater();
    this->startNextDownload();
}

void MovieImageDialog::renderTable()
{
    int cols = this->calcColumnCount();
    ui->table->setColumnCount(cols);
    ui->table->setRowCount(0);
    ui->table->clearContents();

    for (int i=0, n=ui->table->columnCount() ; i<n ; i++)
        ui->table->setColumnWidth(i, this->getColumnWidth());

    for (int i=0, n=m_elements.size() ; i<n ; i++) {
        int row = (i-(i%cols))/cols;
        if (i%cols == 0)
            ui->table->insertRow(row);
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(Qt::UserRole, m_elements[i].originalUrl);
        QLabel *label = new QLabel(ui->table);
        label->setStyleSheet("margin: 5px;");
        label->setAlignment(Qt::AlignTop);
        label->setPixmap(m_elements[i].pixmap);
        m_elements[i].cellWidget = label;
        ui->table->setItem(row, i%cols, item);
        ui->table->setCellWidget(row, i%cols, label);
        ui->table->resizeRowToContents(row);
    }
}

int MovieImageDialog::calcColumnCount()
{
    int width = ui->table->size().width();
    int colWidth = this->getColumnWidth()+4;
    int cols = qFloor((qreal)width/colWidth);
    return cols;
}

int MovieImageDialog::getColumnWidth()
{
    if (m_imageType == TypePoster)
        return 102;
    if (m_imageType == TypeBackdrop)
        return 210;
    return 102;
}

void MovieImageDialog::imageClicked(int row, int col)
{
    QUrl url = ui->table->item(row, col)->data(Qt::UserRole).toUrl();
    m_imageUrl = url;
    accept();
}

void MovieImageDialog::setImageType(ImageType type)
{
    m_imageType = type;
}

void MovieImageDialog::cancelDownloads()
{
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
