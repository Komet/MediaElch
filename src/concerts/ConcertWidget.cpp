#include "ConcertWidget.h"
#include "ui_ConcertWidget.h"

#include <QDoubleValidator>
#include <QFileDialog>
#include <QIntValidator>
#include <QMovie>
#include <QPainter>
#include <QScrollBar>
#include <QtCore/qmath.h>

#include "concerts/ConcertFilesWidget.h"
#include "concerts/ConcertSearch.h"
#include "data/ImageCache.h"
#include "globals/ComboDelegate.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "notifications/NotificationBox.h"
#include "smallWidgets/ClosableImage.h"

/**
 * @brief ConcertWidget::ConcertWidget
 * @param parent
 */
ConcertWidget::ConcertWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ConcertWidget)
{
    ui->setupUi(this);
    ui->concertName->clear();
    ui->artStackedWidget->setAnimation(QEasingCurve::OutCubic);
    ui->artStackedWidget->setSpeed(300);

#ifndef Q_OS_MAC
    QFont nameFont = ui->concertName->font();
    nameFont.setPointSize(nameFont.pointSize() - 4);
    ui->concertName->setFont(nameFont);
#endif

    QFont font = ui->labelClearArt->font();
#ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif

    ui->labelClearArt->setFont(font);
    ui->labelDiscArt->setFont(font);
    ui->labelFanart->setFont(font);
    ui->labelLogo->setFont(font);
    ui->labelPoster->setFont(font);

    ui->badgeWatched->setBadgeType(Badge::Type::BadgeInfo);

    m_concert = nullptr;

    ui->poster->setImageType(ImageType::ConcertPoster);
    ui->backdrop->setImageType(ImageType::ConcertBackdrop);
    ui->logo->setImageType(ImageType::ConcertLogo);
    ui->cdArt->setImageType(ImageType::ConcertCdArt);
    ui->clearArt->setImageType(ImageType::ConcertClearArt);
    foreach (ClosableImage *image, ui->artStackedWidget->findChildren<ClosableImage *>()) {
        connect(image, &ClosableImage::clicked, this, &ConcertWidget::onChooseImage);
        connect(image, &ClosableImage::sigClose, this, &ConcertWidget::onDeleteImage);
        connect(image, &ClosableImage::sigImageDropped, this, &ConcertWidget::onImageDropped);
    }

    connect(ui->name, &QLineEdit::textChanged, this, &ConcertWidget::concertNameChanged);
    connect(ui->buttonRevert, &QAbstractButton::clicked, this, &ConcertWidget::onRevertChanges);
    connect(ui->buttonReloadStreamDetails, &QAbstractButton::clicked, this, &ConcertWidget::onReloadStreamDetails);

    ui->genreCloud->setText(tr("Genres"));
    ui->genreCloud->setPlaceholder(tr("Add Genre"));
    connect(ui->genreCloud, &TagCloud::activated, this, &ConcertWidget::addGenre);
    connect(ui->genreCloud, &TagCloud::deactivated, this, &ConcertWidget::removeGenre);

    ui->tagCloud->setText(tr("Tags"));
    ui->tagCloud->setPlaceholder(tr("Add Tag"));
    connect(ui->tagCloud, &TagCloud::activated, this, &ConcertWidget::addTag);
    connect(ui->tagCloud, &TagCloud::deactivated, this, &ConcertWidget::removeTag);

    ui->poster->setDefaultPixmap(QPixmap(":/img/placeholders/poster.png"));
    ui->backdrop->setDefaultPixmap(QPixmap(":/img/placeholders/fanart.png"));
    ui->logo->setDefaultPixmap(QPixmap(":/img/placeholders/logo.png"));
    ui->clearArt->setDefaultPixmap(QPixmap(":/img/placeholders/clear_art.png"));
    ui->cdArt->setDefaultPixmap(QPixmap(":/img/placeholders/cd_art.png"));

    m_loadingMovie = new QMovie(":/img/spinner.gif", QByteArray(), this);
    m_loadingMovie->start();

    setDisabledTrue();
    clear();

    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    connect(ui->fanarts, SIGNAL(sigRemoveImage(QByteArray)), this, SLOT(onRemoveExtraFanart(QByteArray)));
    connect(ui->fanarts, SIGNAL(sigRemoveImage(QString)), this, SLOT(onRemoveExtraFanart(QString)));
    connect(ui->btnAddExtraFanart, &QAbstractButton::clicked, this, &ConcertWidget::onAddExtraFanart);
    connect(ui->fanarts, &ImageGallery::sigImageDropped, this, &ConcertWidget::onExtraFanartDropped);

    // Connect GUI change events to concert object
    // clang-format off
    connect(ui->name,             &QLineEdit::textEdited, this, &ConcertWidget::onNameChange);
    connect(ui->artist,           &QLineEdit::textEdited, this, &ConcertWidget::onArtistChange);
    connect(ui->album,            &QLineEdit::textEdited, this, &ConcertWidget::onAlbumChange);
    connect(ui->tagline,          &QLineEdit::textEdited, this, &ConcertWidget::onTaglineChange);
    connect(ui->rating,           SIGNAL(valueChanged(double)), this, SLOT(onRatingChange(double)));
    connect(ui->trailer,          &QLineEdit::textEdited, this, &ConcertWidget::onTrailerChange);
    connect(ui->runtime,          SIGNAL(valueChanged(int)), this, SLOT(onRuntimeChange(int)));
    connect(ui->playcount,        SIGNAL(valueChanged(int)), this, SLOT(onPlayCountChange(int)));
    connect(ui->certification,    &QComboBox::editTextChanged, this, &ConcertWidget::onCertificationChange);
    connect(ui->badgeWatched,     &Badge::clicked, this, &ConcertWidget::onWatchedClicked);
    connect(ui->released,         &QDateTimeEdit::dateChanged, this, &ConcertWidget::onReleasedChange);
    connect(ui->lastPlayed,       &QDateTimeEdit::dateTimeChanged, this, &ConcertWidget::onLastWatchedChange);
    connect(ui->overview,         &QTextEdit::textChanged, this, &ConcertWidget::onOverviewChange);
    connect(ui->videoAspectRatio, SIGNAL(valueChanged(double)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoCodec,       &QLineEdit::textEdited, this, &ConcertWidget::onStreamDetailsEdited);
    connect(ui->videoDuration,    &QDateTimeEdit::timeChanged, this, &ConcertWidget::onStreamDetailsEdited);
    connect(ui->videoHeight,      SIGNAL(valueChanged(int)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoWidth,       SIGNAL(valueChanged(int)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoScantype,    &QLineEdit::textEdited, this, &ConcertWidget::onStreamDetailsEdited);
    connect(ui->stereoMode,       SIGNAL(currentIndexChanged(int)), this, SLOT(onStreamDetailsEdited()));
    // clang-format on

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
    m_savingWidget->move(size().width() / 2 - m_savingWidget->width(), height() / 2 - m_savingWidget->height());
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
    if (concert) {
        qDebug() << concert->name();
    }
    if (concert && concert->controller()->downloadsInProgress()) {
        qDebug() << "Downloads are in progress";
        return;
    }
    ui->groupBox_3->setEnabled(true);
    emit setActionSaveEnabled(true, MainWidgets::Concerts);
    emit setActionSearchEnabled(true, MainWidgets::Concerts);
}

/**
 * @brief Sets the state of the main groupbox to disabled
 */
void ConcertWidget::setDisabledTrue()
{
    qDebug() << "Entered";
    ui->groupBox_3->setDisabled(true);
    emit setActionSaveEnabled(false, MainWidgets::Concerts);
    emit setActionSearchEnabled(false, MainWidgets::Concerts);
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
        const auto videoDetails = concert->streamDetails()->videoDetails();
        if (concert->streamDetailsLoaded()
            && videoDetails.value(StreamDetails::VideoDetails::DurationInSeconds).toInt() != 0) {
            using namespace std::chrono;
            seconds runtime{videoDetails.value(StreamDetails::VideoDetails::DurationInSeconds).toInt()};
            concert->setRuntime(duration_cast<minutes>(runtime));
        }
    }
    updateConcertInfo();

    // clang-format off
    connect(m_concert->controller(), &ConcertController::sigInfoLoadDone,      this, &ConcertWidget::onInfoLoadDone,      Qt::UniqueConnection);
    connect(m_concert->controller(), &ConcertController::sigLoadDone,          this, &ConcertWidget::onLoadDone,          Qt::UniqueConnection);
    connect(m_concert->controller(), &ConcertController::sigDownloadProgress,  this, &ConcertWidget::onDownloadProgress,  Qt::UniqueConnection);
    connect(m_concert->controller(), &ConcertController::sigLoadingImages,     this, &ConcertWidget::onLoadingImages,     Qt::UniqueConnection);
    connect(m_concert->controller(), &ConcertController::sigLoadImagesStarted, this, &ConcertWidget::onLoadImagesStarted, Qt::UniqueConnection);
    connect(m_concert->controller(), &ConcertController::sigImage,             this, &ConcertWidget::onSetImage,          Qt::UniqueConnection);
    // clang-format on

    if (concert->controller()->downloadsInProgress()) {
        setDisabledTrue();
    } else {
        setEnabledTrue();
    }
}

/**
 * @brief Shows the search widget
 */
void ConcertWidget::onStartScraperSearch()
{
    qDebug() << "Entered";
    if (m_concert == nullptr) {
        qDebug() << "My concert is invalid";
        return;
    }
    emit setActionSearchEnabled(false, MainWidgets::Concerts);
    emit setActionSaveEnabled(false, MainWidgets::Concerts);
    ConcertSearch::instance()->exec(m_concert->name());
    if (ConcertSearch::instance()->result() == QDialog::Accepted) {
        setDisabledTrue();
        ConcertSearch::instance()->scraperId();
        m_concert->controller()->loadData(ConcertSearch::instance()->scraperId(),
            Manager::instance()->concertScrapers().at(ConcertSearch::instance()->scraperNo()),
            ConcertSearch::instance()->infosToLoad());
    } else {
        emit setActionSearchEnabled(true, MainWidgets::Concerts);
        emit setActionSaveEnabled(true, MainWidgets::Concerts);
    }
}

/**
 * @brief ConcertWidget::infoLoadDone
 * @param concert
 */
void ConcertWidget::onInfoLoadDone(Concert *concert)
{
    if (m_concert == nullptr) {
        return;
    }

    if (m_concert == concert) {
        updateConcertInfo();
        ui->buttonRevert->setVisible(true);
        emit setActionSaveEnabled(false, MainWidgets::Concerts);
    }
}

void ConcertWidget::onLoadDone(Concert *concert)
{
    if (m_concert == nullptr || m_concert != concert) {
        return;
    }
    setEnabledTrue();
    ui->fanarts->setLoading(false);
}

void ConcertWidget::onLoadImagesStarted(Concert *concert)
{
    Q_UNUSED(concert);
    // emit actorDownloadStarted(tr("Downloading images..."), Constants::MovieProgressMessageId+movie->movieId());
}

void ConcertWidget::onLoadingImages(Concert *concert, QList<ImageType> imageTypes)
{
    if (concert != m_concert) {
        return;
    }

    for (const auto imageType : imageTypes) {
        for (auto cImage : ui->artStackedWidget->findChildren<ClosableImage *>()) {
            if (cImage->imageType() == imageType) {
                cImage->setLoading(true);
            }
        }
    }

    if (imageTypes.contains(ImageType::ConcertExtraFanart)) {
        ui->fanarts->setLoading(true);
    }
    ui->groupBox_3->update();
}

void ConcertWidget::onSetImage(Concert *concert, ImageType type, QByteArray data)
{
    if (concert != m_concert) {
        return;
    }

    if (type == ImageType::ConcertExtraFanart) {
        ui->fanarts->addImage(data);
        return;
    }

    for (auto image : ui->artStackedWidget->findChildren<ClosableImage *>()) {
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
    // emit actorDownloadProgress(maximum-current, maximum, Constants::MovieProgressMessageId+movie->movieId());
}


/**
 * @brief Updates the contents of the widget with the current concert infos
 */
void ConcertWidget::updateConcertInfo()
{
    qDebug() << "Entered";
    if (m_concert == nullptr) {
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
    ui->runtime->setValue(static_cast<int>(m_concert->runtime().count()));
    ui->trailer->setText(m_concert->trailer().toString());
    ui->playcount->setValue(m_concert->playcount());
    ui->lastPlayed->setDateTime(m_concert->lastPlayed());
    ui->overview->setPlainText(m_concert->overview());
    ui->badgeWatched->setActive(m_concert->watched());

    QStringList certifications;
    QStringList genres;
    QStringList tags;
    certifications.append("");
    for (const Concert *concert : Manager::instance()->concertModel()->concerts()) {
        if (!certifications.contains(concert->certification().toString()) && concert->certification().isValid()) {
            certifications.append(concert->certification().toString());
        }
        genres.append(concert->genres());
        tags.append(concert->tags());
    }
    qSort(certifications.begin(), certifications.end(), LocaleStringCompare());
    ui->certification->addItems(certifications);
    ui->certification->setCurrentIndex(certifications.indexOf(m_concert->certification().toString()));
    ui->certification->blockSignals(false);

    // `setTags` requires distinct lists
    genres.removeDuplicates();
    tags.removeDuplicates();

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

    updateImages(QList<ImageType>{ImageType::ConcertPoster,
        ImageType::ConcertBackdrop,
        ImageType::ConcertLogo,
        ImageType::ConcertCdArt,
        ImageType::ConcertClearArt});

    ui->fanarts->setImages(m_concert->extraFanarts(Manager::instance()->mediaCenterInterfaceConcert()));

    ui->rating->blockSignals(false);
    ui->runtime->blockSignals(false);
    ui->playcount->blockSignals(false);
    ui->released->blockSignals(false);
    ui->lastPlayed->blockSignals(false);
    ui->overview->blockSignals(false);

    emit setActionSaveEnabled(true, MainWidgets::Concerts);

    ui->rating->setEnabled(
        Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertRating));
    ui->tagline->setEnabled(
        Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertTagline));
    ui->certification->setEnabled(
        Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertCertification));
    ui->trailer->setEnabled(
        Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertTrailer));
    ui->badgeWatched->setEnabled(
        Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertWatched));

    ui->buttonRevert->setVisible(m_concert->hasChanged());
}

void ConcertWidget::updateImages(QList<ImageType> images)
{
    for (const auto imageType : images) {
        for (auto cImage : ui->artStackedWidget->findChildren<ClosableImage *>()) {
            if (cImage->imageType() == imageType) {
                updateImage(imageType, cImage);
                break;
            }
        }
    }
}

void ConcertWidget::updateImage(ImageType imageType, ClosableImage *image)
{
    if (!m_concert->image(imageType).isNull()) {
        image->setImage(m_concert->image(imageType));

    } else if (!m_concert->imagesToRemove().contains(imageType) && m_concert->hasImage(imageType)) {
        QString imgFileName = Manager::instance()->mediaCenterInterface()->imageFileName(m_concert, imageType);
        if (!imgFileName.isEmpty()) {
            image->setImage(imgFileName);
        }
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

    if (reloadFromFile) {
        m_concert->controller()->loadStreamDetailsFromFile();
    }

    StreamDetails *streamDetails = m_concert->streamDetails();
    const auto videoDetails = streamDetails->videoDetails();
    ui->videoWidth->setValue(videoDetails.value(StreamDetails::VideoDetails::Width).toInt());
    ui->videoHeight->setValue(videoDetails.value(StreamDetails::VideoDetails::Height).toInt());
    ui->videoAspectRatio->setValue(
        QString{videoDetails.value(StreamDetails::VideoDetails::Aspect)}.replace(",", ".").toDouble());
    ui->videoCodec->setText(videoDetails.value(StreamDetails::VideoDetails::Codec));
    ui->videoScantype->setText(videoDetails.value(StreamDetails::VideoDetails::ScanType));
    ui->stereoMode->setCurrentIndex(0);
    for (int i = 0, n = ui->stereoMode->count(); i < n; ++i) {
        if (ui->stereoMode->itemData(i).toString() == videoDetails.value(StreamDetails::VideoDetails::StereoMode)) {
            ui->stereoMode->setCurrentIndex(i);
        }
    }
    QTime time(0, 0, 0, 0);
    time = time.addSecs(videoDetails.value(StreamDetails::VideoDetails::DurationInSeconds).toInt());
    ui->videoDuration->setTime(time);
    if (reloadFromFile) {
        ui->runtime->setValue(qFloor(videoDetails.value(StreamDetails::VideoDetails::DurationInSeconds).toInt() / 60));
    }

    foreach (QWidget *widget, m_streamDetailsWidgets)
        widget->deleteLater();
    m_streamDetailsWidgets.clear();
    m_streamDetailsAudio.clear();
    m_streamDetailsSubtitles.clear();

    const auto audioDetails = streamDetails->audioDetails();
    int audioTracks = audioDetails.count();
    for (int i = 0; i < audioTracks; ++i) {
        QLabel *label = new QLabel(tr("Track %1").arg(i + 1));
        ui->streamDetails->addWidget(label, 8 + i, 0);
        QLineEdit *edit1 = new QLineEdit(audioDetails.at(i).value(StreamDetails::AudioDetails::Language));
        QLineEdit *edit2 = new QLineEdit(audioDetails.at(i).value(StreamDetails::AudioDetails::Codec));
        QLineEdit *edit3 = new QLineEdit(audioDetails.at(i).value(StreamDetails::AudioDetails::Channels));
        edit3->setMaximumWidth(50);
        edit1->setToolTip(tr("Language"));
        edit2->setToolTip(tr("Codec"));
        edit3->setToolTip(tr("Channels"));
        edit1->setPlaceholderText(tr("Language"));
        edit2->setPlaceholderText(tr("Codec"));
        edit2->setPlaceholderText(tr("Channels"));
        auto layout = new QHBoxLayout();
        layout->addWidget(edit1);
        layout->addWidget(edit2);
        layout->addWidget(edit3);
        layout->addStretch(10);
        ui->streamDetails->addLayout(layout, 8 + i, 1);
        m_streamDetailsWidgets << label << edit1 << edit2 << edit3;
        m_streamDetailsAudio << (QList<QLineEdit *>() << edit1 << edit2 << edit3);
        connect(edit1, &QLineEdit::textEdited, this, &ConcertWidget::onStreamDetailsEdited);
        connect(edit2, &QLineEdit::textEdited, this, &ConcertWidget::onStreamDetailsEdited);
        connect(edit3, &QLineEdit::textEdited, this, &ConcertWidget::onStreamDetailsEdited);
    }

    if (!streamDetails->subtitleDetails().isEmpty()) {
        QLabel *label = new QLabel(tr("Subtitles"));
        QFont font = ui->labelStreamDetailsAudio->font();
        font.setBold(true);
        label->setFont(font);
        ui->streamDetails->addWidget(label, 8 + audioTracks, 0);
        m_streamDetailsWidgets << label;

        for (int i = 0, n = streamDetails->subtitleDetails().count(); i < n; ++i) {
            QLabel *label = new QLabel(tr("Track %1").arg(i + 1));
            ui->streamDetails->addWidget(label, 9 + audioTracks + i, 0);
            QLineEdit *edit1 =
                new QLineEdit(streamDetails->subtitleDetails().at(i).value(StreamDetails::SubtitleDetails::Language));
            edit1->setToolTip(tr("Language"));
            edit1->setPlaceholderText(tr("Language"));
            auto layout = new QHBoxLayout();
            layout->addWidget(edit1);
            layout->addStretch(10);
            ui->streamDetails->addLayout(layout, 9 + audioTracks + i, 1);
            m_streamDetailsWidgets << label << edit1;
            m_streamDetailsSubtitles << (QList<QLineEdit *>() << edit1);
            connect(edit1, &QLineEdit::textEdited, this, &ConcertWidget::onStreamDetailsEdited);
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
    QList<Concert *> concerts = ConcertFilesWidget::instance()->selectedConcerts();
    if (concerts.count() == 0) {
        concerts.append(m_concert);
    }

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
                if (m_concert == concert) {
                    updateConcertInfo();
                }
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
            if (m_concert == concert) {
                updateConcertInfo();
            }
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
    if (!m_concert) {
        return;
    }
    m_concert->addGenre(genre);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a genre
 */
void ConcertWidget::removeGenre(QString genre)
{
    if (!m_concert) {
        return;
    }
    m_concert->removeGenre(genre);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::addTag(QString tag)
{
    if (!m_concert) {
        return;
    }
    m_concert->addTag(tag);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::removeTag(QString tag)
{
    if (!m_concert) {
        return;
    }
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
    if (!m_concert) {
        return;
    }
    m_concert->setName(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the artist has changed
 */
void ConcertWidget::onArtistChange(QString text)
{
    if (!m_concert) {
        return;
    }
    m_concert->setArtist(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the album has changed
 */
void ConcertWidget::onAlbumChange(QString text)
{
    if (!m_concert) {
        return;
    }
    m_concert->setAlbum(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the tagline has changed
 */
void ConcertWidget::onTaglineChange(QString text)
{
    if (!m_concert) {
        return;
    }
    m_concert->setTagline(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the rating has changed
 */
void ConcertWidget::onRatingChange(double value)
{
    if (!m_concert) {
        return;
    }
    m_concert->setRating(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the release date has changed
 */
void ConcertWidget::onReleasedChange(QDate date)
{
    if (!m_concert) {
        return;
    }
    m_concert->setReleased(date);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the runtime has changed
 */
void ConcertWidget::onRuntimeChange(int value)
{
    if (!m_concert) {
        return;
    }
    m_concert->setRuntime(std::chrono::minutes(value));
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the certification has changed
 */
void ConcertWidget::onCertificationChange(QString text)
{
    if (!m_concert) {
        return;
    }
    m_concert->setCertification(Certification(text));
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the trailer has changed
 */
void ConcertWidget::onTrailerChange(QString text)
{
    if (!m_concert) {
        return;
    }
    m_concert->setTrailer(text);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::onWatchedClicked()
{
    if (!m_concert) {
        return;
    }

    bool active = !ui->badgeWatched->isActive();
    ui->badgeWatched->setActive(active);
    m_concert->setWatched(active);

    if (active) {
        if (m_concert->playcount() < 1) {
            ui->playcount->setValue(1);
        }
        if (!m_concert->lastPlayed().isValid()) {
            ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
        }
    }
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the play count has changed
 */
void ConcertWidget::onPlayCountChange(int value)
{
    if (!m_concert) {
        return;
    }
    m_concert->setPlayCount(value);
    ui->badgeWatched->setActive(value > 0);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the last watched date has changed
 */
void ConcertWidget::onLastWatchedChange(QDateTime dateTime)
{
    if (!m_concert) {
        return;
    }
    m_concert->setLastPlayed(dateTime);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the concert as changed when the overview has changed
 */
void ConcertWidget::onOverviewChange()
{
    if (!m_concert) {
        return;
    }
    m_concert->setOverview(ui->overview->toPlainText());
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Updates all stream details for this concert with values from the widget
 */
void ConcertWidget::onStreamDetailsEdited()
{
    StreamDetails *details = m_concert->streamDetails();
    details->setVideoDetail(StreamDetails::VideoDetails::Codec, ui->videoCodec->text());
    details->setVideoDetail(StreamDetails::VideoDetails::Aspect, ui->videoAspectRatio->text());
    details->setVideoDetail(StreamDetails::VideoDetails::Width, ui->videoWidth->text());
    details->setVideoDetail(StreamDetails::VideoDetails::Height, ui->videoHeight->text());
    details->setVideoDetail(StreamDetails::VideoDetails::ScanType, ui->videoScantype->text());
    details->setVideoDetail(StreamDetails::VideoDetails::DurationInSeconds,
        QString("%1").arg(-ui->videoDuration->time().secsTo(QTime(0, 0))));
    details->setVideoDetail(StreamDetails::VideoDetails::StereoMode, ui->stereoMode->currentData().toString());

    for (int i = 0, n = m_streamDetailsAudio.count(); i < n; ++i) {
        details->setAudioDetail(i, StreamDetails::AudioDetails::Language, m_streamDetailsAudio[i][0]->text());
        details->setAudioDetail(i, StreamDetails::AudioDetails::Codec, m_streamDetailsAudio[i][1]->text());
        details->setAudioDetail(i, StreamDetails::AudioDetails::Channels, m_streamDetailsAudio[i][2]->text());
    }
    for (int i = 0, n = m_streamDetailsSubtitles.count(); i < n; ++i) {
        details->setSubtitleDetail(i, StreamDetails::SubtitleDetails::Language, m_streamDetailsSubtitles[i][0]->text());
    }

    m_concert->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::onRemoveExtraFanart(const QByteArray &image)
{
    if (!m_concert) {
        return;
    }
    m_concert->removeExtraFanart(image);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::onRemoveExtraFanart(const QString &file)
{
    if (!m_concert) {
        return;
    }
    m_concert->removeExtraFanart(file);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::onAddExtraFanart()
{
    if (!m_concert) {
        return;
    }

    ImageDialog::instance()->setImageType(ImageType::ConcertExtraFanart);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMultiSelection(true);
    ImageDialog::instance()->setConcert(m_concert);
    ImageDialog::instance()->setDownloads(m_concert->backdrops());
    ImageDialog::instance()->exec(ImageType::ConcertBackdrop);

    if (ImageDialog::instance()->result() == QDialog::Accepted && !ImageDialog::instance()->imageUrls().isEmpty()) {
        ui->fanarts->setLoading(true);
        emit setActionSaveEnabled(false, MainWidgets::Concerts);
        m_concert->controller()->loadImages(ImageType::ConcertExtraFanart, ImageDialog::instance()->imageUrls());
        ui->buttonRevert->setVisible(true);
    }
}

void ConcertWidget::onExtraFanartDropped(QUrl imageUrl)
{
    if (!m_concert) {
        return;
    }
    ui->fanarts->setLoading(true);
    emit setActionSaveEnabled(false, MainWidgets::Concerts);
    m_concert->controller()->loadImages(ImageType::ConcertExtraFanart, QList<QUrl>() << imageUrl);
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::onChooseImage()
{
    if (m_concert == nullptr) {
        return;
    }

    auto image = static_cast<ClosableImage *>(QObject::sender());
    if (!image) {
        return;
    }

    ImageDialog::instance()->setImageType(image->imageType());
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setConcert(m_concert);
    if (image->imageType() == ImageType::ConcertPoster) {
        ImageDialog::instance()->setDownloads(m_concert->posters());
    } else if (image->imageType() == ImageType::ConcertBackdrop) {
        ImageDialog::instance()->setDownloads(m_concert->backdrops());
    } else {
        ImageDialog::instance()->setDownloads(QList<Poster>());
    }
    ImageDialog::instance()->exec(image->imageType());

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, MainWidgets::Concerts);
        m_concert->controller()->loadImage(image->imageType(), ImageDialog::instance()->imageUrl());
        ui->buttonRevert->setVisible(true);
    }
}

void ConcertWidget::onDeleteImage()
{
    if (m_concert == nullptr) {
        return;
    }

    auto image = static_cast<ClosableImage *>(QObject::sender());
    if (!image) {
        return;
    }

    m_concert->removeImage(image->imageType());
    updateImages({image->imageType()});
    ui->buttonRevert->setVisible(true);
}

void ConcertWidget::onImageDropped(ImageType imageType, QUrl imageUrl)
{
    if (!m_concert) {
        return;
    }
    emit setActionSaveEnabled(false, MainWidgets::Concerts);
    m_concert->controller()->loadImage(imageType, imageUrl);
    ui->buttonRevert->setVisible(true);
}
