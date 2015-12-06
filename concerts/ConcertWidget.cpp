#include "ConcertWidget.h"
#include "ui_ConcertWidget.h"

#include <QtCore/qmath.h>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QIntValidator>
#include <QMovie>
#include <QPainter>
#include <QScrollBar>
#include "concerts/ConcertSearch.h"
#include "concerts/ConcertFilesWidget.h"
#include "data/ImageCache.h"
#include "globals/ComboDelegate.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "notifications/NotificationBox.h"

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
    ui->artStackedWidget->setAnimation(QEasingCurve::OutCubic);
    ui->artStackedWidget->setSpeed(300);

#ifndef Q_OS_MAC
    QFont nameFont = ui->concertName->font();
    nameFont.setPointSize(nameFont.pointSize()-4);
    ui->concertName->setFont(nameFont);
#endif

    QFont font = ui->labelClearArt->font();
    #ifdef Q_OS_WIN32
        font.setPointSize(font.pointSize()-1);
    #else
        font.setPointSize(font.pointSize()-2);
    #endif

    ui->labelClearArt->setFont(font);
    ui->labelDiscArt->setFont(font);
    ui->labelFanart->setFont(font);
    ui->labelLogo->setFont(font);
    ui->labelPoster->setFont(font);

    ui->badgeWatched->setBadgeType(Badge::BadgeInfo);

    m_concert = 0;

    ui->poster->setImageType(ImageType::ConcertPoster);
    ui->backdrop->setImageType(ImageType::ConcertBackdrop);
    ui->logo->setImageType(ImageType::ConcertLogo);
    ui->cdArt->setImageType(ImageType::ConcertCdArt);
    ui->clearArt->setImageType(ImageType::ConcertClearArt);
    foreach (ClosableImage *image, ui->artStackedWidget->findChildren<ClosableImage*>()) {
        connect(image, &ClosableImage::clicked, this, &ConcertWidget::onChooseImage);
        connect(image, &ClosableImage::sigClose, this, &ConcertWidget::onDeleteImage);
        connect(image, &ClosableImage::sigImageDropped, this, &ConcertWidget::onImageDropped);
    }

    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(concertNameChanged(QString)));
    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));
    connect(ui->buttonReloadStreamDetails, SIGNAL(clicked()), this, SLOT(onReloadStreamDetails()));

    ui->genreCloud->setText(tr("Genres"));
    ui->genreCloud->setPlaceholder(tr("Add Genre"));
    connect(ui->genreCloud, SIGNAL(activated(QString)), this, SLOT(addGenre(QString)));
    connect(ui->genreCloud, SIGNAL(deactivated(QString)), this, SLOT(removeGenre(QString)));

    ui->tagCloud->setText(tr("Tags"));
    ui->tagCloud->setPlaceholder(tr("Add Tag"));
    connect(ui->tagCloud, SIGNAL(activated(QString)), this, SLOT(addTag(QString)));
    connect(ui->tagCloud, SIGNAL(deactivated(QString)), this, SLOT(removeTag(QString)));

    ui->poster->setDefaultPixmap(QPixmap(":/img/placeholders/poster.png"));
    ui->backdrop->setDefaultPixmap(QPixmap(":/img/placeholders/fanart.png"));
    ui->logo->setDefaultPixmap(QPixmap(":/img/placeholders/logo.png"));
    ui->clearArt->setDefaultPixmap(QPixmap(":/img/placeholders/clear_art.png"));
    ui->cdArt->setDefaultPixmap(QPixmap(":/img/placeholders/cd_art.png"));

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();

    setDisabledTrue();
    clear();

    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    connect(ui->fanarts, SIGNAL(sigRemoveImage(QByteArray)), this, SLOT(onRemoveExtraFanart(QByteArray)));
    connect(ui->fanarts, SIGNAL(sigRemoveImage(QString)), this, SLOT(onRemoveExtraFanart(QString)));
    connect(ui->btnAddExtraFanart, SIGNAL(clicked()), this, SLOT(onAddExtraFanart()));
    connect(ui->fanarts, &ImageGallery::sigImageDropped, this, &ConcertWidget::onExtraFanartDropped);

    // Connect GUI change events to concert object
    connect(ui->name, SIGNAL(textEdited(QString)), this, SLOT(onNameChange(QString)));
    connect(ui->artist, SIGNAL(textEdited(QString)), this, SLOT(onArtistChange(QString)));
    connect(ui->album, SIGNAL(textEdited(QString)), this, SLOT(onAlbumChange(QString)));
    connect(ui->tagline, SIGNAL(textEdited(QString)), this, SLOT(onTaglineChange(QString)));
    connect(ui->rating, SIGNAL(valueChanged(double)), this, SLOT(onRatingChange(double)));
    connect(ui->trailer, SIGNAL(textEdited(QString)), this, SLOT(onTrailerChange(QString)));
    connect(ui->runtime, SIGNAL(valueChanged(int)), this, SLOT(onRuntimeChange(int)));
    connect(ui->playcount, SIGNAL(valueChanged(int)), this, SLOT(onPlayCountChange(int)));
    connect(ui->certification, SIGNAL(editTextChanged(QString)), this, SLOT(onCertificationChange(QString)));
    connect(ui->badgeWatched, SIGNAL(clicked()), this, SLOT(onWatchedClicked()));
    connect(ui->released, SIGNAL(dateChanged(QDate)), this, SLOT(onReleasedChange(QDate)));
    connect(ui->lastPlayed, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(onLastWatchedChange(QDateTime)));
    connect(ui->overview, SIGNAL(textChanged()), this, SLOT(onOverviewChange()));
    connect(ui->videoAspectRatio, SIGNAL(valueChanged(double)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoCodec, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoDuration, SIGNAL(timeChanged(QTime)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoHeight, SIGNAL(valueChanged(int)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoWidth, SIGNAL(valueChanged(int)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoScantype, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->stereoMode, SIGNAL(currentIndexChanged(int)), this, SLOT(onStreamDetailsEdited()));

    QPainter p;
    QPixmap revert(":/img/arrow_circle_left.png");
    p.begin(&revert);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(revert.rect(), QColor(0, 0, 0, 200));
    p.end();
    ui->buttonRevert->setIcon(QIcon(revert));
    ui->buttonRevert->setVisible(false);

    Helper::instance()->applyStyle(ui->artStackedWidget);
    Helper::instance()->applyStyle(ui->tabWidget);
    Helper::instance()->applyEffect(ui->groupBox_3);
    Helper::instance()->fillStereoModeCombo(ui->stereoMode);
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

void ConcertWidget::setBigWindow(bool bigWindow)
{
    if (bigWindow && !ui->artStackedWidget->isExpanded()) {
        ui->artStackedWidget->expandToOne();
        ui->artStackedWidgetButtons->setVisible(false);
    } else if (!bigWindow && ui->artStackedWidget->isExpanded()) {
        ui->artStackedWidget->collapse();
        ui->artStackedWidgetButtons->setVisible(true);
    }
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
    ui->artist->clear();
    ui->album->clear();
    ui->tagline->clear();
    ui->rating->clear();
    ui->released->setDate(QDate::currentDate());
    ui->runtime->clear();
    ui->trailer->clear();
    ui->playcount->clear();
    ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
    ui->overview->clear();

    ui->poster->clear();
    ui->backdrop->clear();
    ui->logo->clear();
    ui->clearArt->clear();
    ui->cdArt->clear();
    ui->genreCloud->clear();
    ui->tagCloud->clear();

    ui->fanarts->clear();
    ui->poster->clear();
    ui->backdrop->clear();
    ui->logo->clear();
    ui->clearArt->clear();
    ui->cdArt->clear();

    ui->videoCodec->clear();
    ui->videoScantype->clear();

    bool blocked;
    blocked = ui->videoAspectRatio->blockSignals(true);
    ui->videoAspectRatio->clear();
    ui->videoAspectRatio->blockSignals(blocked);

    blocked = ui->videoDuration->blockSignals(true);
    ui->videoDuration->clear();
    ui->videoDuration->blockSignals(blocked);

    blocked = ui->videoHeight->blockSignals(true);
    ui->videoHeight->clear();
    ui->videoHeight->blockSignals(blocked);

    blocked = ui->videoWidth->blockSignals(true);
    ui->videoWidth->clear();
    ui->videoWidth->blockSignals(blocked);

    blocked = ui->stereoMode->blockSignals(true);
    ui->stereoMode->setCurrentIndex(0);
    ui->stereoMode->blockSignals(blocked);

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
    if (concert && concert->controller()->downloadsInProgress()) {
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
    concert->controller()->loadData(Manager::instance()->mediaCenterInterfaceConcert());
    m_concert = concert;
    if (!concert->streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails()) {
        concert->controller()->loadStreamDetailsFromFile();
        if (concert->streamDetailsLoaded() && concert->streamDetails()->videoDetails().value("durationinseconds").toInt() != 0)
            concert->setRuntime(qFloor(concert->streamDetails()->videoDetails().value("durationinseconds").toInt()/60));
    }
    updateConcertInfo();

    connect(m_concert->controller(), SIGNAL(sigInfoLoadDone(Concert*)), this, SLOT(onInfoLoadDone(Concert*)), Qt::UniqueConnection);
    connect(m_concert->controller(), SIGNAL(sigLoadDone(Concert*)), this, SLOT(onLoadDone(Concert*)), Qt::UniqueConnection);
    connect(m_concert->controller(), SIGNAL(sigDownloadProgress(Concert*,int, int)), this, SLOT(onDownloadProgress(Concert*,int,int)), Qt::UniqueConnection);
    connect(m_concert->controller(), SIGNAL(sigLoadingImages(Concert*,QList<int>)), this, SLOT(onLoadingImages(Concert*,QList<int>)), Qt::UniqueConnection);
    connect(m_concert->controller(), SIGNAL(sigLoadImagesStarted(Concert*)), this, SLOT(onLoadImagesStarted(Concert*)), Qt::UniqueConnection);
    connect(m_concert->controller(), SIGNAL(sigImage(Concert*,int,QByteArray)), this, SLOT(onSetImage(Concert*,int,QByteArray)), Qt::UniqueConnection);

    if (concert->controller()->downloadsInProgress())
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
        m_concert->controller()->loadData(ConcertSearch::instance()->scraperId(), Manager::instance()->concertScrapers().at(ConcertSearch::instance()->scraperNo()),
                            ConcertSearch::instance()->infosToLoad());
    } else {
        emit setActionSearchEnabled(true, WidgetConcerts);
        emit setActionSaveEnabled(true, WidgetConcerts);
    }
}

/**
 * @brief ConcertWidget::infoLoadDone
 * @param concert
 */
void ConcertWidget::onInfoLoadDone(Concert *concert)
{
    if (m_concert == 0)
        return;

    if (m_concert == concert) {
        updateConcertInfo();
        ui->buttonRevert->setVisible(true);
        emit setActionSaveEnabled(false, WidgetConcerts);
    }
}

void ConcertWidget::onLoadDone(Concert *concert)
{
    if (m_concert == 0 || m_concert != concert)
        return;
    setEnabledTrue();
    ui->fanarts->setLoading(false);
}

void ConcertWidget::onLoadImagesStarted(Concert *concert)
{
    Q_UNUSED(concert);
    // emit actorDownloadStarted(tr("Downloading images..."), Constants::MovieProgressMessageId+movie->movieId());
}

void ConcertWidget::onLoadingImages(Concert *concert, QList<int> imageTypes)
{
    if (concert != m_concert)
        return;

    foreach (const int &imageType, imageTypes) {
        foreach (ClosableImage *cImage, ui->artStackedWidget->findChildren<ClosableImage*>()) {
            if (cImage->imageType() == imageType)
                cImage->setLoading(true);
        }
    }

    if (imageTypes.contains(ImageType::ConcertExtraFanart))
        ui->fanarts->setLoading(true);
    ui->groupBox_3->update();
}

void ConcertWidget::onSetImage(Concert *concert, int type, QByteArray data)
{
    if (concert != m_concert)
        return;

    if (type == ImageType::ConcertExtraFanart) {
        ui->fanarts->addImage(data);
        return;
    }

    foreach (ClosableImage *image, ui->artStackedWidget->findChildren<ClosableImage*>()) {
        if (image->imageType() == type) {
            image->setLoading(false);
            image->setImage(data);
        }
    }
}

void ConcertWidget::onDownloadProgress(Concert *concert, int current, int maximum)
{
    Q_UNUSED(concert);
    Q_UNUSED(current);
    Q_UNUSED(maximum);
    //emit actorDownloadProgress(maximum-current, maximum, Constants::MovieProgressMessageId+movie->movieId());
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
    ui->released->blockSignals(true);
    ui->lastPlayed->blockSignals(true);
    ui->overview->blockSignals(true);

    clear();

    ui->files->setText(m_concert->files().join(", "));
    ui->files->setToolTip(m_concert->files().join("\n"));
    ui->name->setText(m_concert->name());
    ui->artist->setText(m_concert->artist());
    ui->album->setText(m_concert->album());
    ui->concertName->setText(m_concert->name());
    ui->tagline->setText(m_concert->tagline());
    ui->rating->setValue(m_concert->rating());
    ui->released->setDate(m_concert->released());
    ui->runtime->setValue(m_concert->runtime());
    ui->trailer->setText(m_concert->trailer().toString());
    ui->playcount->setValue(m_concert->playcount());
    ui->lastPlayed->setDateTime(m_concert->lastPlayed());
    ui->overview->setPlainText(m_concert->overview());
    ui->badgeWatched->setActive(m_concert->watched());

    QStringList certifications;
    QStringList genres;
    QStringList tags;
    certifications.append("");
    foreach (Concert *concert, Manager::instance()->concertModel()->concerts()) {
        if (!certifications.contains(concert->certification()) && !concert->certification().isEmpty())
            certifications.append(concert->certification());
        genres.append(concert->genres());
        tags.append(concert->tags());
    }
    qSort(certifications.begin(), certifications.end(), LocaleStringCompare());
    ui->certification->addItems(certifications);
    ui->certification->setCurrentIndex(certifications.indexOf(m_concert->certification()));
    ui->certification->blockSignals(false);
    ui->genreCloud->setTags(genres, m_concert->genres());
    ui->tagCloud->setTags(tags, m_concert->tags());

    // Streamdetails
    updateStreamDetails();
    ui->videoAspectRatio->setEnabled(m_concert->streamDetailsLoaded());
    ui->videoCodec->setEnabled(m_concert->streamDetailsLoaded());
    ui->videoDuration->setEnabled(m_concert->streamDetailsLoaded());
    ui->videoHeight->setEnabled(m_concert->streamDetailsLoaded());
    ui->videoWidth->setEnabled(m_concert->streamDetailsLoaded());
    ui->videoScantype->setEnabled(m_concert->streamDetailsLoaded());
    ui->stereoMode->setEnabled(m_concert->streamDetailsLoaded());

    updateImages(QList<int>() << ImageType::ConcertPoster << ImageType::ConcertBackdrop << ImageType::ConcertLogo << ImageType::ConcertCdArt << ImageType::ConcertClearArt);

    ui->fanarts->setImages(m_concert->extraFanarts(Manager::instance()->mediaCenterInterfaceConcert()));

    ui->rating->blockSignals(false);
    ui->runtime->blockSignals(false);
    ui->playcount->blockSignals(false);
    ui->released->blockSignals(false);
    ui->lastPlayed->blockSignals(false);
    ui->overview->blockSignals(false);

    emit setActionSaveEnabled(true, WidgetConcerts);

    ui->rating->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertRating));
    ui->tagline->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertTagline));
    ui->certification->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertCertification));
    ui->trailer->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertTrailer));
    ui->badgeWatched->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertWatched));

    ui->buttonRevert->setVisible(m_concert->hasChanged());
}

void ConcertWidget::updateImages(QList<int> images)
{
    foreach (const int &imageType, images) {
        foreach (ClosableImage *cImage, ui->artStackedWidget->findChildren<ClosableImage*>()) {
            if (cImage->imageType() == imageType) {
                updateImage(imageType, cImage);
                break;
            }
        }
    }
}

void ConcertWidget::updateImage(const int &imageType, ClosableImage *image)
{
    if (!m_concert->image(imageType).isNull()) {
        image->setImage(m_concert->image(imageType));
    } else if (!m_concert->imagesToRemove().contains(imageType) && m_concert->hasImage(imageType)) {
        QString imgFileName = Manager::instance()->mediaCenterInterface()->imageFileName(m_concert, imageType);
        if (!imgFileName.isEmpty())
            image->setImage(imgFileName);
    }
}

/**
 * @brief Fills the widget with streamdetails
 * @param reloadFromFile If true forces a reload of streamdetails from the file
 */
void ConcertWidget::updateStreamDetails(bool reloadFromFile)
{
    ui->videoAspectRatio->blockSignals(true);
    ui->videoDuration->blockSignals(true);
    ui->videoWidth->blockSignals(true);
    ui->videoHeight->blockSignals(true);
    ui->stereoMode->blockSignals(true);

    if (reloadFromFile)
        m_concert->controller()->loadStreamDetailsFromFile();

    StreamDetails *streamDetails = m_concert->streamDetails();
    ui->videoWidth->setValue(streamDetails->videoDetails().value("width").toInt());
    ui->videoHeight->setValue(streamDetails->videoDetails().value("height").toInt());
    ui->videoAspectRatio->setValue(QString(streamDetails->videoDetails().value("aspect")).replace(",", ".").toDouble());
    ui->videoCodec->setText(streamDetails->videoDetails().value("codec"));
    ui->videoScantype->setText(streamDetails->videoDetails().value("scantype"));
    ui->stereoMode->setCurrentIndex(0);
    for (int i=0, n=ui->stereoMode->count() ; i<n ; ++i) {
        if (ui->stereoMode->itemData(i).toString() == streamDetails->videoDetails().value("stereomode"))
            ui->stereoMode->setCurrentIndex(i);
    }
    QTime time(0, 0, 0, 0);
    time = time.addSecs(streamDetails->videoDetails().value("durationinseconds").toInt());
    ui->videoDuration->setTime(time);
    if (reloadFromFile)
        ui->runtime->setValue(qFloor(streamDetails->videoDetails().value("durationinseconds").toInt()/60));

    foreach (QWidget *widget, m_streamDetailsWidgets)
        widget->deleteLater();
    m_streamDetailsWidgets.clear();
    m_streamDetailsAudio.clear();
    m_streamDetailsSubtitles.clear();

    int audioTracks = streamDetails->audioDetails().count();
    for (int i=0 ; i<audioTracks ; ++i) {
        QLabel *label = new QLabel(tr("Track %1").arg(i+1));
        ui->streamDetails->addWidget(label, 8+i, 0);
        QLineEdit *edit1 = new QLineEdit(streamDetails->audioDetails().at(i).value("language"));
        QLineEdit *edit2 = new QLineEdit(streamDetails->audioDetails().at(i).value("codec"));
        QLineEdit *edit3 = new QLineEdit(streamDetails->audioDetails().at(i).value("channels"));
        edit3->setMaximumWidth(50);
        edit1->setToolTip(tr("Language"));
        edit2->setToolTip(tr("Codec"));
        edit3->setToolTip(tr("Channels"));
        edit1->setPlaceholderText(tr("Language"));
        edit2->setPlaceholderText(tr("Codec"));
        edit2->setPlaceholderText(tr("Channels"));
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget(edit1);
        layout->addWidget(edit2);
        layout->addWidget(edit3);
        layout->addStretch(10);
        ui->streamDetails->addLayout(layout, 8+i, 1);
        m_streamDetailsWidgets << label << edit1 << edit2 << edit3;
        m_streamDetailsAudio << (QList<QLineEdit*>() << edit1 << edit2 << edit3);
        connect(edit1, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
        connect(edit2, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
        connect(edit3, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
    }

    if (!streamDetails->subtitleDetails().isEmpty()) {
        QLabel *label = new QLabel(tr("Subtitles"));
        QFont font = ui->labelStreamDetailsAudio->font();
        font.setBold(true);
        label->setFont(font);
        ui->streamDetails->addWidget(label, 8+audioTracks, 0);
        m_streamDetailsWidgets << label;

        for (int i=0, n=streamDetails->subtitleDetails().count() ; i<n ; ++i) {
            QLabel *label = new QLabel(tr("Track %1").arg(i+1));
            ui->streamDetails->addWidget(label, 9+audioTracks+i, 0);
            QLineEdit *edit1 = new QLineEdit(streamDetails->subtitleDetails().at(i).value("language"));
            edit1->setToolTip(tr("Language"));
            edit1->setPlaceholderText(tr("Language"));
            QHBoxLayout *layout = new QHBoxLayout();
            layout->addWidget(edit1);
            layout->addStretch(10);
            ui->streamDetails->addLayout(layout, 9+audioTracks+i, 1);
            m_streamDetailsWidgets << label << edit1;
            m_streamDetailsSubtitles << (QList<QLineEdit*>() << edit1);
            connect(edit1, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
        }
    }

    // Media Flags
    ui->mediaFlags->setStreamDetails(streamDetails);

    ui->videoAspectRatio->blockSignals(false);
    ui->videoDuration->blockSignals(false);
    ui->videoWidth->blockSignals(false);
    ui->videoHeight->blockSignals(false);
    ui->stereoMode->blockSignals(false);
}

/**
 * @brief Forces a reload of stream details
 */
void ConcertWidget::onReloadStreamDetails()
{
    updateStreamDetails(true);
    ui->videoAspectRatio->setEnabled(true);
    ui->videoCodec->setEnabled(true);
    ui->videoDuration->setEnabled(true);
    ui->videoHeight->setEnabled(true);
    ui->videoWidth->setEnabled(true);
    ui->videoScantype->setEnabled(true);
    ui->stereoMode->setEnabled(true);
}

/**
 * @brief Saves concert information
 */
void ConcertWidget::onSaveInformation()
{
    QList<Concert*> concerts = ConcertFilesWidget::instance()->selectedConcerts();
    if (concerts.count() == 0)
        concerts.append(m_concert);

    setDisabledTrue();
    m_savingWidget->show();

    if (concerts.count() == 1) {
        m_concert->controller()->saveData(Manager::instance()->mediaCenterInterfaceConcert());
        m_concert->controller()->loadData(Manager::instance()->mediaCenterInterfaceConcert(), true);
        updateConcertInfo();
        NotificationBox::instance()->showMessage(tr("<b>\"%1\"</b> Saved").arg(m_concert->name()));
    } else {
        foreach (Concert *concert, concerts) {
            if (concert->hasChanged()) {
                concert->controller()->saveData(Manager::instance()->mediaCenterInterfaceConcert());
                concert->controller()->loadData(Manager::instance()->mediaCenterInterfaceConcert(), true);
                if (m_concert == concert)
                    updateConcertInfo();
            }
        }
        NotificationBox::instance()->showMessage(tr("Concerts Saved"));
    }

    setEnabledTrue();
    m_savingWidget->hide();
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
            concert->controller()->saveData(Manager::instance()->mediaCenterInterfaceConcert());
            concert->controller()->loadData(Manager::instance()->mediaCenterInterfaceConcert(), true);
            if (m_concert == concert)
                updateConcertInfo();
        }
    }
    setEnabledTrue();
    m_savingWidget->hide();
    NotificationBox::instance()->showMessage(tr("All Concerts Saved"));
    ui->buttonRevert->setVisible(false);
}

/**
 * @brief Revert changes for current concert
 */
void ConcertWidget::onRevertChanges()
{
    qDebug() << "Entered";
    m_concert->controller()->loadData(Manager::instance()->mediaCenterInterfaceConcert(), true);
    updateConcertInfo();
}

/*** add/remove/edit Genres ***/

/**
 * @brief Adds a genre
 */
void ConcertWidget::addGenre(QString genre)
{
    if (!m_concert)
        return;
    m_concert->addGenre(genre);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a genre
 */
void ConcertWidget::removeGenre(QString genre)
{
    if (!m_concert)
        return;
    m_concert->removeGenre(genre);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::addTag(QString tag)
{
    if (!m_concert)
        return;
    m_concert->addTag(tag);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::removeTag(QString tag)
{
    if (!m_concert)
        return;
    m_concert->removeTag(tag);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Shows the first page with movie art
 */
void ConcertWidget::onArtPageOne()
{
    ui->artStackedWidget->slideInIdx(0);
    ui->buttonArtPageTwo->setChecked(false);
    ui->buttonArtPageOne->setChecked(true);
}

/**
 * @brief Shows the second page with movie art
 */
void ConcertWidget::onArtPageTwo()
{
    ui->artStackedWidget->slideInIdx(1);
    ui->buttonArtPageOne->setChecked(false);
    ui->buttonArtPageTwo->setChecked(true);
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
 * @brief Marks the concert as changed when the artist has changed
 */
void ConcertWidget::onArtistChange(QString text)
{
    if (!m_concert)
        return;
    m_concert->setArtist(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the album has changed
 */
void ConcertWidget::onAlbumChange(QString text)
{
    if (!m_concert)
        return;
    m_concert->setAlbum(text);
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

void ConcertWidget::onWatchedClicked()
{
    if (!m_concert)
        return;

    bool active = !ui->badgeWatched->isActive();
    ui->badgeWatched->setActive(active);
    m_concert->setWatched(active);

    if (active) {
        if (m_concert->playcount() < 1)
            ui->playcount->setValue(1);
        if (!m_concert->lastPlayed().isValid())
            ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
    }
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
    ui->badgeWatched->setActive(value > 0);
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

/**
 * @brief Updates all stream details for this concert with values from the widget
 */
void ConcertWidget::onStreamDetailsEdited()
{
    StreamDetails *details = m_concert->streamDetails();
    details->setVideoDetail("codec", ui->videoCodec->text());
    details->setVideoDetail("aspect", ui->videoAspectRatio->text());
    details->setVideoDetail("width", ui->videoWidth->text());
    details->setVideoDetail("height", ui->videoHeight->text());
    details->setVideoDetail("scantype", ui->videoScantype->text());
    details->setVideoDetail("durationinseconds", QString("%1").arg(-ui->videoDuration->time().secsTo(QTime(0, 0))));
    details->setVideoDetail("stereomode", ui->stereoMode->currentData().toString());

    for (int i=0, n=m_streamDetailsAudio.count() ; i<n ; ++i) {
        details->setAudioDetail(i, "language", m_streamDetailsAudio[i][0]->text());
        details->setAudioDetail(i, "codec", m_streamDetailsAudio[i][1]->text());
        details->setAudioDetail(i, "channels", m_streamDetailsAudio[i][2]->text());
    }
    for (int i=0, n=m_streamDetailsSubtitles.count() ; i<n ; ++i)
        details->setSubtitleDetail(i, "language", m_streamDetailsSubtitles[i][0]->text());

    m_concert->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::onRemoveExtraFanart(const QByteArray &image)
{
    if (!m_concert)
        return;
    m_concert->removeExtraFanart(image);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::onRemoveExtraFanart(const QString &file)
{
    if (!m_concert)
        return;
    m_concert->removeExtraFanart(file);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::onAddExtraFanart()
{
    if (!m_concert)
        return;

    ImageDialog::instance()->setImageType(ImageType::ConcertExtraFanart);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMultiSelection(true);
    ImageDialog::instance()->setConcert(m_concert);
    ImageDialog::instance()->setDownloads(m_concert->backdrops());
    ImageDialog::instance()->exec(ImageType::ConcertBackdrop);

    if (ImageDialog::instance()->result() == QDialog::Accepted && !ImageDialog::instance()->imageUrls().isEmpty()) {
        ui->fanarts->setLoading(true);
        emit setActionSaveEnabled(false, WidgetConcerts);
        m_concert->controller()->loadImages(ImageType::ConcertExtraFanart, ImageDialog::instance()->imageUrls());
        ui->buttonRevert->setVisible(true);
    }
}

void ConcertWidget::onExtraFanartDropped(QUrl imageUrl)
{
    if (!m_concert)
        return;
    ui->fanarts->setLoading(true);
    emit setActionSaveEnabled(false, WidgetConcerts);
    m_concert->controller()->loadImages(ImageType::ConcertExtraFanart, QList<QUrl>() << imageUrl);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::onChooseImage()
{
    if (m_concert == 0)
        return;

    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    ImageDialog::instance()->setImageType(image->imageType());
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setConcert(m_concert);
    if (image->imageType() == ImageType::ConcertPoster)
        ImageDialog::instance()->setDownloads(m_concert->posters());
    else if (image->imageType() == ImageType::ConcertBackdrop)
        ImageDialog::instance()->setDownloads(m_concert->posters());
    else
        ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(image->imageType());

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetConcerts);
        m_concert->controller()->loadImage(image->imageType(), ImageDialog::instance()->imageUrl());
        ui->buttonRevert->setVisible(true);
    }
}

void ConcertWidget::onDeleteImage()
{
    if (m_concert == 0)
        return;

    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    m_concert->removeImage(image->imageType());
    updateImages(QList<int>() << image->imageType());
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::onImageDropped(int imageType, QUrl imageUrl)
{
    if (!m_concert)
        return;
    emit setActionSaveEnabled(false, WidgetConcerts);
    m_concert->controller()->loadImage(imageType, imageUrl);
    ui->buttonRevert->setVisible(true);
}
