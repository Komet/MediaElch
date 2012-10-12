#include "ConcertWidget.h"
#include "ui_ConcertWidget.h"

#include <QDoubleValidator>
#include <QFileDialog>
#include <QIntValidator>
#include <QMovie>
#include <QPainter>
#include <QScrollBar>
#include "globals/Globals.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "main/MessageBox.h"
#include "globals/ImageDialog.h"
#include "concerts/ConcertSearch.h"

/**
 * @brief ConcertWidget::ConcertWidget
 * @param parent
 */
ConcertWidget::ConcertWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConcertWidget)
{
    ui->setupUi(this);
    ui->concertName->clear();
    ui->genres->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->buttonPreviewPoster->setEnabled(false);
    ui->buttonPreviewBackdrop->setEnabled(false);

    QFont font = ui->concertName->font();
    font.setPointSize(font.pointSize()+4);
    ui->concertName->setFont(font);

    font = ui->posterResolution->font();
    #ifdef Q_WS_WIN
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->posterResolution->setFont(font);
    ui->backdropResolution->setFont(font);

    m_concert = 0;
    m_posterDownloadManager = new DownloadManager(this);

    connect(ui->poster, SIGNAL(clicked()), this, SLOT(chooseConcertPoster()));
    connect(ui->backdrop, SIGNAL(clicked()), this, SLOT(chooseConcertBackdrop()));
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(posterDownloadFinished(DownloadManagerElement)));
    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(concertNameChanged(QString)));
    connect(ui->buttonAddGenre, SIGNAL(clicked()), this, SLOT(addGenre()));
    connect(ui->buttonRemoveGenre, SIGNAL(clicked()), this, SLOT(removeGenre()));
    connect(ui->genres, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onGenreEdited(QTableWidgetItem*)));
    connect(ui->buttonPreviewPoster, SIGNAL(clicked()), this, SLOT(onPreviewPoster()));
    connect(ui->buttonPreviewBackdrop, SIGNAL(clicked()), this, SLOT(onPreviewBackdrop()));
    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();

    setDisabledTrue();
    clear();

    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    // Connect GUI change events to concert object
    connect(ui->name, SIGNAL(textEdited(QString)), this, SLOT(onNameChange(QString)));
    connect(ui->tagline, SIGNAL(textEdited(QString)), this, SLOT(onTaglineChange(QString)));
    connect(ui->rating, SIGNAL(valueChanged(double)), this, SLOT(onRatingChange(double)));
    connect(ui->trailer, SIGNAL(textEdited(QString)), this, SLOT(onTrailerChange(QString)));
    connect(ui->runtime, SIGNAL(valueChanged(int)), this, SLOT(onRuntimeChange(int)));
    connect(ui->playcount, SIGNAL(valueChanged(int)), this, SLOT(onPlayCountChange(int)));
    connect(ui->certification, SIGNAL(editTextChanged(QString)), this, SLOT(onCertificationChange(QString)));
    connect(ui->watched, SIGNAL(stateChanged(int)), this, SLOT(onWatchedChange(int)));
    connect(ui->released, SIGNAL(dateChanged(QDate)), this, SLOT(onReleasedChange(QDate)));
    connect(ui->lastPlayed, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(onLastWatchedChange(QDateTime)));
    connect(ui->overview, SIGNAL(textChanged()), this, SLOT(onOverviewChange()));

    QPixmap zoomIn(":/img/zoom_in.png");
    QPainter p;
    p.begin(&zoomIn);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(zoomIn.rect(), QColor(0, 0, 0, 150));
    p.end();
    ui->buttonPreviewBackdrop->setIcon(QIcon(zoomIn));
    ui->buttonPreviewPoster->setIcon(QIcon(zoomIn));

    QPixmap revert(":/img/arrow_circle_left.png");
    p.begin(&revert);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(revert.rect(), QColor(0, 0, 0, 200));
    p.end();
    ui->buttonRevert->setIcon(QIcon(revert));
    ui->buttonRevert->setVisible(false);
}

/**
 * @brief ConcertWidget::~ConcertWidget
 */
ConcertWidget::~ConcertWidget()
{
    delete ui;
}

/**
 * @brief Repositions the saving widget
 * @param event
 */
void ConcertWidget::resizeEvent(QResizeEvent *event)
{
    m_savingWidget->move(size().width()/2-m_savingWidget->width(), height()/2-m_savingWidget->height());
    QWidget::resizeEvent(event);
}

/**
 * @brief Clears all contents of the widget
 */
void ConcertWidget::clear()
{
    qDebug() << "Entered";
    ui->certification->clear();
    ui->concertName->clear();
    ui->files->clear();
    ui->name->clear();
    ui->tagline->clear();
    ui->rating->clear();
    ui->released->setDate(QDate::currentDate());
    ui->runtime->clear();
    ui->trailer->clear();
    ui->playcount->clear();
    ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
    ui->overview->clear();
    ui->genres->setRowCount(0);
    ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
    ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->tabWidget->setCurrentIndex(0);
    ui->posterResolution->setText("");
    ui->backdropResolution->setText("");
    ui->buttonRevert->setVisible(false);
}

/**
 * @brief Updates the title text
 * @param text New text
 */
void ConcertWidget::concertNameChanged(QString text)
{
    ui->concertName->setText(text);
}

/**
 * @brief Sets the state of the main groupbox to enabled
 * @param concert Current concert
 */
void ConcertWidget::setEnabledTrue(Concert *concert)
{
    qDebug() << "Entered";
    if (concert)
        qDebug() << concert->name();
    if (concert && concert->downloadsInProgress()) {
        qDebug() << "Downloads are in progress";
        return;
    }
    ui->groupBox_3->setEnabled(true);
    emit setActionSaveEnabled(true, WidgetConcerts);
    emit setActionSearchEnabled(true, WidgetConcerts);
}

/**
 * @brief Sets the state of the main groupbox to disabled
 */
void ConcertWidget::setDisabledTrue()
{
    qDebug() << "Entered";
    ui->groupBox_3->setDisabled(true);
    emit setActionSaveEnabled(false, WidgetConcerts);
    emit setActionSearchEnabled(false, WidgetConcerts);
}

/**
 * @brief Sets the current concert, tells the concert to load data and images and updates widgets contents
 * @param concert Current concert
 */
void ConcertWidget::setConcert(Concert *concert)
{
    qDebug() << "Entered, concert=" << concert->name();
    concert->loadData(Manager::instance()->mediaCenterInterfaceConcert());
    m_concert = concert;
    updateConcertInfo();
    if (concert->downloadsInProgress())
        setDisabledTrue();
    else
        setEnabledTrue();
}

/**
 * @brief Shows the search widget
 */
void ConcertWidget::onStartScraperSearch()
{
    qDebug() << "Entered";
    if (m_concert == 0) {
        qDebug() << "My concert is invalid";
        return;
    }
    emit setActionSearchEnabled(false, WidgetConcerts);
    emit setActionSaveEnabled(false, WidgetConcerts);
    ConcertSearch::instance()->exec(m_concert->name());
    if (ConcertSearch::instance()->result() == QDialog::Accepted) {
        setDisabledTrue();
        m_concert->loadData(ConcertSearch::instance()->scraperId(), Manager::instance()->concertScrapers().at(ConcertSearch::instance()->scraperNo()),
                            ConcertSearch::instance()->infosToLoad());
        connect(m_concert, SIGNAL(loaded(Concert*)), this, SLOT(loadDone(Concert*)), Qt::UniqueConnection);
    } else {
        emit setActionSearchEnabled(true, WidgetConcerts);
        emit setActionSaveEnabled(true, WidgetConcerts);
    }
}

/**
 * @brief Called when the search widget finishes
 * Updates infos and starts downloads
 * @param concert Concert
 */
void ConcertWidget::loadDone(Concert *concert)
{
    qDebug() << "Entered";
    if (m_concert == 0) {
        qDebug() << "My concert is invalid";
        return;
    }

    if (m_concert == concert)
        updateConcertInfo();
    else
        qDebug() << "Concert has changed";
    int downloadsSize = 0;

    if (ConcertSearch::instance()->infosToLoad().contains(ConcertScraperInfos::Poster) && concert->posters().size() > 0) {
        emit setActionSaveEnabled(false, WidgetConcerts);
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = concert->posters().at(0).originalUrl;
        d.concert = concert;
        m_posterDownloadManager->addDownload(d);
        if (m_concert == concert) {
            ui->poster->setPixmap(QPixmap());
            ui->poster->setMovie(m_loadingMovie);
        }
        downloadsSize++;
    }

    if (ConcertSearch::instance()->infosToLoad().contains(ConcertScraperInfos::Backdrop) && concert->backdrops().size() > 0) {
        emit setActionSaveEnabled(false, WidgetConcerts);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = concert->backdrops().at(0).originalUrl;
        d.concert = concert;
        m_posterDownloadManager->addDownload(d);
        if (m_concert == concert) {
            ui->backdrop->setPixmap(QPixmap());
            ui->backdrop->setMovie(m_loadingMovie);
        }
        downloadsSize++;
    }

    if (m_concert == concert)
        setEnabledTrue();

    concert->setDownloadsInProgress(downloadsSize > 0);
    concert->setDownloadsSize(downloadsSize);
    ui->buttonRevert->setVisible(true);

    connect(m_posterDownloadManager, SIGNAL(allDownloadsFinished(Concert*)), this, SLOT(downloadActorsFinished(Concert*)), Qt::UniqueConnection);
}

/**
 * @brief Updates the contents of the widget with the current concert infos
 */
void ConcertWidget::updateConcertInfo()
{
    qDebug() << "Entered";
    if (m_concert == 0) {
        qDebug() << "My concert is invalid";
        return;
    }

    ui->rating->blockSignals(true);
    ui->runtime->blockSignals(true);
    ui->playcount->blockSignals(true);
    ui->certification->blockSignals(true);
    ui->watched->blockSignals(true);
    ui->released->blockSignals(true);
    ui->lastPlayed->blockSignals(true);
    ui->overview->blockSignals(true);
    ui->genres->blockSignals(true);

    clear();

    ui->files->setText(m_concert->files().join(", "));
    ui->files->setToolTip(m_concert->files().join("\n"));
    ui->name->setText(m_concert->name());
    ui->concertName->setText(m_concert->name());
    ui->tagline->setText(m_concert->tagline());
    ui->rating->setValue(m_concert->rating());
    ui->released->setDate(m_concert->released());
    ui->runtime->setValue(m_concert->runtime());
    ui->trailer->setText(m_concert->trailer().toString());
    ui->playcount->setValue(m_concert->playcount());
    ui->lastPlayed->setDateTime(m_concert->lastPlayed());
    ui->overview->setPlainText(m_concert->overview());
    ui->watched->setChecked(m_concert->watched());

    QStringList certifications;
    certifications.append("");
    foreach (Concert *concert, Manager::instance()->concertModel()->concerts()) {
        if (!certifications.contains(concert->certification()) && !concert->certification().isEmpty())
            certifications.append(concert->certification());
    }
    certifications.sort();
    ui->certification->addItems(certifications);
    ui->certification->setCurrentIndex(certifications.indexOf(m_concert->certification()));
    ui->certification->blockSignals(false);

    ui->genres->blockSignals(true);
    foreach (QString *genre, m_concert->genresPointer()) {
        int row = ui->genres->rowCount();
        ui->genres->insertRow(row);
        ui->genres->setItem(row, 0, new QTableWidgetItem(*genre));
        ui->genres->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(genre));
    }
    ui->genres->blockSignals(false);

    if (!m_concert->posterImage()->isNull()) {
        ui->poster->setPixmap(QPixmap::fromImage(*m_concert->posterImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(m_concert->posterImage()->width()).arg(m_concert->posterImage()->height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = *m_concert->posterImage();
    } else if (!Manager::instance()->mediaCenterInterface()->posterImageName(m_concert).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->posterImageName(m_concert));
        ui->poster->setPixmap(p.scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = p.toImage();
    } else {
        ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
        ui->posterResolution->setText("");
        ui->buttonPreviewPoster->setEnabled(false);
    }

    if (!m_concert->backdropImage()->isNull()) {
        ui->backdrop->setPixmap(QPixmap::fromImage(*m_concert->backdropImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(m_concert->backdropImage()->width()).arg(m_concert->backdropImage()->height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = *m_concert->backdropImage();
    } else if (!Manager::instance()->mediaCenterInterface()->backdropImageName(m_concert).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->backdropImageName(m_concert));
        ui->backdrop->setPixmap(p.scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = p.toImage();
    } else {
        ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText("");
        ui->buttonPreviewBackdrop->setEnabled(false);
    }

    ui->rating->blockSignals(false);
    ui->runtime->blockSignals(false);
    ui->playcount->blockSignals(false);
    ui->watched->blockSignals(false);
    ui->released->blockSignals(false);
    ui->lastPlayed->blockSignals(false);
    ui->overview->blockSignals(false);
    ui->genres->blockSignals(false);

    emit setActionSaveEnabled(true, WidgetConcerts);

    ui->rating->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertRating));
    ui->tagline->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertTagline));
    ui->certification->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertCertification));
    ui->trailer->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertTrailer));
    ui->watched->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertWatched));

    ui->buttonRevert->setVisible(m_concert->hasChanged());
}

/**
 * @brief Shows the ImageDialog and after successful execution starts poster download
 */
void ConcertWidget::chooseConcertPoster()
{
    qDebug() << "Entered";
    if (m_concert == 0) {
        qDebug() << "My concert is invalid";
        return;
    }

    ImageDialog::instance()->setImageType(TypePoster);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setDownloads(m_concert->posters());
    ImageDialog::instance()->exec(ImageDialogType::ConcertPoster);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetConcerts);
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = ImageDialog::instance()->imageUrl();
        d.concert = m_concert;
        m_posterDownloadManager->addDownload(d);
        ui->poster->setPixmap(QPixmap());
        ui->poster->setMovie(m_loadingMovie);
        ui->buttonPreviewPoster->setEnabled(false);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the ImageDialog and after successful execution starts backdrop download
 */
void ConcertWidget::chooseConcertBackdrop()
{
    qDebug() << "Entered";
    if (m_concert == 0) {
        qDebug() << "My concert is invalid";
        return;
    }

    ImageDialog::instance()->setImageType(TypeBackdrop);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setDownloads(m_concert->backdrops());
    ImageDialog::instance()->exec(ImageDialogType::ConcertBackdrop);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetConcerts);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = ImageDialog::instance()->imageUrl();
        d.concert = m_concert;
        m_posterDownloadManager->addDownload(d);
        ui->backdrop->setPixmap(QPixmap());
        ui->backdrop->setMovie(m_loadingMovie);
        ui->buttonPreviewBackdrop->setEnabled(false);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Adjusts the size of the backdrop to common values (1080p or 720p) and shows the image
 * @param elem Downloaded element
 */
void ConcertWidget::posterDownloadFinished(DownloadManagerElement elem)
{
    qDebug() << "Entered";
    if (elem.imageType == TypePoster) {
        qDebug() << "Got a poster";
        if (m_concert == elem.concert) {
            ui->poster->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->posterResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewPoster->setEnabled(true);
            m_currentPoster = elem.image;
        }
        elem.concert->setPosterImage(elem.image);
    } else if (elem.imageType == TypeBackdrop) {
        qDebug() << "Got a backdrop";
        if ((elem.image.width() != 1920 || elem.image.height() != 1080) &&
            elem.image.width() > 1915 && elem.image.width() < 1925 && elem.image.height() > 1075 && elem.image.height() < 1085)
            elem.image = elem.image.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        if ((elem.image.width() != 1280 || elem.image.height() != 720) &&
            elem.image.width() > 1275 && elem.image.width() < 1285 && elem.image.height() > 715 && elem.image.height() < 725)
            elem.image = elem.image.scaled(1280, 720, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        if (m_concert == elem.concert) {
            ui->backdrop->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->backdropResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewBackdrop->setEnabled(true);
            m_currentBackdrop = elem.image;
        }
        elem.concert->setBackdropImage(elem.image);
    }
    if (m_posterDownloadManager->downloadQueueSize() == 0) {
        emit setActionSaveEnabled(true, WidgetConcerts);
        elem.concert->setDownloadsInProgress(false);
    }
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Saves concert information
 */
void ConcertWidget::onSaveInformation()
{
    qDebug() << "Entered";
    setDisabledTrue();
    m_savingWidget->show();
    m_concert->saveData(Manager::instance()->mediaCenterInterfaceConcert());
    m_concert->loadData(Manager::instance()->mediaCenterInterfaceConcert(), true);
    updateConcertInfo();
    setEnabledTrue();
    m_savingWidget->hide();
    MessageBox::instance()->showMessage(tr("<b>\"%1\"</b> Saved").arg(m_concert->name()));
    ui->buttonRevert->setVisible(false);
}

/**
 * @brief Saves all changed concerts
 */
void ConcertWidget::onSaveAll()
{
    qDebug() << "Entered";
    setDisabledTrue();
    m_savingWidget->show();

    foreach (Concert *concert, Manager::instance()->concertModel()->concerts()) {
        if (concert->hasChanged()) {
            concert->saveData(Manager::instance()->mediaCenterInterfaceConcert());
            concert->loadData(Manager::instance()->mediaCenterInterfaceConcert(), true);
            if (m_concert == concert)
                updateConcertInfo();
        }
    }
    setEnabledTrue();
    m_savingWidget->hide();
    MessageBox::instance()->showMessage(tr("All Concerts Saved"));
    ui->buttonRevert->setVisible(false);
}

/**
 * @brief Revert changes for current concert
 */
void ConcertWidget::onRevertChanges()
{
    qDebug() << "Entered";
    m_concert->loadData(Manager::instance()->mediaCenterInterfaceConcert(), true);
    updateConcertInfo();
}

/**
 * @brief Toggles enabled state of the widget
 * @param concert
 */
void ConcertWidget::downloadActorsFinished(Concert *concert)
{
    qDebug() << "Entered, concert=" << concert->name();
    if (concert == m_concert)
        setEnabledTrue();
    else
        qDebug() << "Concert has changed";
    concert->setDownloadsInProgress(false);
}

/*** add/remove/edit Genres ***/

/**
 * @brief Adds a genre
 */
void ConcertWidget::addGenre()
{
    QString g = tr("Unknown Genre");
    m_concert->addGenre(g);
    QString *genre = m_concert->genresPointer().last();

    ui->genres->blockSignals(true);
    int row = ui->genres->rowCount();
    ui->genres->insertRow(row);
    ui->genres->setItem(row, 0, new QTableWidgetItem(g));
    ui->genres->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(genre));
    ui->genres->scrollToBottom();
    ui->genres->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a genre
 */
void ConcertWidget::removeGenre()
{
    int row = ui->genres->currentRow();
    if (row < 0 || row >= ui->genres->rowCount() || !ui->genres->currentItem()->isSelected())
        return;

    QString *genre = ui->genres->item(row, 0)->data(Qt::UserRole).value<QString*>();
    m_concert->removeGenre(genre);
    ui->genres->blockSignals(true);
    ui->genres->removeRow(row);
    ui->genres->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Stores changed values for a genre
 * @param item Edited item
 */
void ConcertWidget::onGenreEdited(QTableWidgetItem *item)
{
    QString *genre = ui->genres->item(item->row(), 0)->data(Qt::UserRole).value<QString*>();
    genre->clear();
    genre->append(item->text());
    m_concert->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Shows a full size image of the backdrop
 */
void ConcertWidget::onPreviewBackdrop()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentBackdrop));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows a full size image of the poster
 */
void ConcertWidget::onPreviewPoster()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentPoster));
    ImagePreviewDialog::instance()->exec();
}

/*** Pass GUI events to concert object ***/

/**
 * @brief Marks the concert as changed when the name has changed
 */
void ConcertWidget::onNameChange(QString text)
{
    if (!m_concert)
        return;
    m_concert->setName(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the tagline has changed
 */
void ConcertWidget::onTaglineChange(QString text)
{
    if (!m_concert)
        return;
    m_concert->setTagline(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the rating has changed
 */
void ConcertWidget::onRatingChange(double value)
{
    if (!m_concert)
        return;
    m_concert->setRating(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the release date has changed
 */
void ConcertWidget::onReleasedChange(QDate date)
{
    if (!m_concert)
        return;
    m_concert->setReleased(date);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the runtime has changed
 */
void ConcertWidget::onRuntimeChange(int value)
{
    if (!m_concert)
        return;
    m_concert->setRuntime(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the certification has changed
 */
void ConcertWidget::onCertificationChange(QString text)
{
    if (!m_concert)
        return;
    m_concert->setCertification(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the trailer has changed
 */
void ConcertWidget::onTrailerChange(QString text)
{
    if (!m_concert)
        return;
    m_concert->setTrailer(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the watched state has changed
 */
void ConcertWidget::onWatchedChange(int state)
{
    if (!m_concert)
        return;
    m_concert->setWatched(state == Qt::Checked);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the play count has changed
 */
void ConcertWidget::onPlayCountChange(int value)
{
    if (!m_concert)
        return;
    m_concert->setPlayCount(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the last watched date has changed
 */
void ConcertWidget::onLastWatchedChange(QDateTime dateTime)
{
    if (!m_concert)
        return;
    m_concert->setLastPlayed(dateTime);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the overview has changed
 */
void ConcertWidget::onOverviewChange()
{
    if (!m_concert)
        return;
    m_concert->setOverview(ui->overview->toPlainText());
    ui->buttonRevert->setVisible(true);
}
