#include "ConcertWidget.h"
#include "ui_ConcertWidget.h"

#include <QDoubleValidator>
#include <QFileDialog>
#include <QIntValidator>
#include <QMovie>
#include <QPainter>
#include <QScrollBar>
#include <QtCore/qmath.h>

#include "data/ImageCache.h"
#include "globals/ComboDelegate.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "ui/concerts/ConcertFilesWidget.h"
#include "ui/concerts/ConcertSearch.h"
#include "ui/notifications/NotificationBox.h"
#include "ui/small_widgets/ClosableImage.h"

ConcertWidget::ConcertWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ConcertWidget)
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
#ifdef Q_OS_WIN
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif

    ui->labelClearArt->setFont(font);
    ui->labelDiscArt->setFont(font);
    ui->labelFanart->setFont(font);
    ui->labelLogo->setFont(font);
    ui->labelPoster->setFont(font);

    ui->poster->setImageType(ImageType::ConcertPoster);
    ui->backdrop->setImageType(ImageType::ConcertBackdrop);
    ui->logo->setImageType(ImageType::ConcertLogo);
    ui->cdArt->setImageType(ImageType::ConcertCdArt);
    ui->clearArt->setImageType(ImageType::ConcertClearArt);
    for (ClosableImage* image : ui->artStackedWidget->findChildren<ClosableImage*>()) {
        connect(image, &ClosableImage::clicked, this, &ConcertWidget::onChooseImage);
        connect(image, &ClosableImage::sigClose, this, &ConcertWidget::onDeleteImage);
        connect(image, &ClosableImage::sigImageDropped, this, &ConcertWidget::onImageDropped);
    }

    // clang-format off
    connect(ui->concertInfo,          &ConcertInfoWidget::concertNameChanged,            this, &ConcertWidget::concertNameChanged);
    connect(ui->concertInfo,          &ConcertInfoWidget::infoChanged,                   this, &ConcertWidget::onInfoChanged);
    connect(ui->concertStreamdetails, &ConcertStreamDetailsWidget::streamDetailsChanged, this, [this]() {
        ui->mediaFlags->setStreamDetails(m_concert->streamDetails());
        onInfoChanged();
    });
    connect(ui->buttonRevert,         &QAbstractButton::clicked,                         this, &ConcertWidget::onRevertChanges);
    connect(ui->concertStreamdetails, &ConcertStreamDetailsWidget::runtimeChanged,       this, [ui = ui](std::chrono::minutes runtime) {
        ui->concertInfo->setRuntime(runtime);
    });
    // clang-format on

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

    connect(ui->fanarts,
        elchOverload<QByteArray>(&ImageGallery::sigRemoveImage),
        this,
        elchOverload<QByteArray>(&ConcertWidget::onRemoveExtraFanart));
    connect(ui->fanarts,
        elchOverload<QString>(&ImageGallery::sigRemoveImage),
        this,
        elchOverload<QString>(&ConcertWidget::onRemoveExtraFanart));
    connect(ui->btnAddExtraFanart, &QAbstractButton::clicked, this, &ConcertWidget::onAddExtraFanart);
    connect(ui->fanarts, &ImageGallery::sigImageDropped, this, &ConcertWidget::onExtraFanartDropped);

    QPainter p;
    QPixmap revert(":/img/arrow_circle_left.png");
    p.begin(&revert);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(revert.rect(), QColor(0, 0, 0, 200));
    p.end();
    ui->buttonRevert->setIcon(QIcon(revert));
    ui->buttonRevert->setVisible(false);

    helper::applyStyle(ui->artStackedWidget);
    helper::applyStyle(ui->tabWidget);
    helper::applyEffect(ui->groupBox_3);
}

/**
 * \brief ConcertWidget::~ConcertWidget
 */
ConcertWidget::~ConcertWidget()
{
    delete ui;
}

/**
 * \brief Repositions the saving widget
 */
void ConcertWidget::resizeEvent(QResizeEvent* event)
{
    m_savingWidget->move(size().width() / 2 - m_savingWidget->width(), height() / 2 - m_savingWidget->height());
    QWidget::resizeEvent(event);
}

void ConcertWidget::onInfoChanged()
{
    ui->buttonRevert->setVisible(true);
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
 * \brief Clears all contents of the widget
 */
void ConcertWidget::clear()
{
    ui->concertName->clear();

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

    ui->buttonRevert->setVisible(false);
}

/**
 * \brief Updates the title text
 * \param text New text
 */
void ConcertWidget::concertNameChanged(QString text)
{
    ui->concertName->setText(text);
}

/**
 * \brief Sets the state of the main groupbox to enabled
 * \param concert Current concert
 */
void ConcertWidget::setEnabledTrue(Concert* concert)
{
    if (concert != nullptr) {
        qDebug() << concert->title();
    }
    if ((concert != nullptr) && concert->controller()->downloadsInProgress()) {
        qDebug() << "Downloads are in progress";
        return;
    }
    ui->groupBox_3->setEnabled(true);
    emit setActionSaveEnabled(true, MainWidgets::Concerts);
    emit setActionSearchEnabled(true, MainWidgets::Concerts);
}

/**
 * \brief Sets the state of the main groupbox to disabled
 */
void ConcertWidget::setDisabledTrue()
{
    ui->groupBox_3->setDisabled(true);
    emit setActionSaveEnabled(false, MainWidgets::Concerts);
    emit setActionSearchEnabled(false, MainWidgets::Concerts);
}

/**
 * \brief Sets the current concert, tells the concert to load data and images and updates widgets contents
 * \param concert Current concert
 */
void ConcertWidget::setConcert(Concert* concert)
{
    qDebug() << "Entered, concert=" << concert->title();
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
    ui->concertInfo->setConcertController(concert->controller());
    ui->concertStreamdetails->setConcertController(concert->controller());
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
 * \brief Shows the search widget
 */
void ConcertWidget::onStartScraperSearch()
{
    if (m_concert == nullptr) {
        qDebug() << "My concert is invalid";
        return;
    }
    emit setActionSearchEnabled(false, MainWidgets::Concerts);
    emit setActionSaveEnabled(false, MainWidgets::Concerts);

    auto* searchWidget = new ConcertSearch(this);
    searchWidget->execWithSearch(m_concert->title());

    if (searchWidget->result() == QDialog::Accepted) {
        setDisabledTrue();
        // TODO: Not only TmdbId
        m_concert->controller()->loadData(
            TmdbId(searchWidget->concertIdentifier()), searchWidget->scraper(), searchWidget->infosToLoad());
        searchWidget->deleteLater();

    } else {
        searchWidget->deleteLater();
        emit setActionSearchEnabled(true, MainWidgets::Concerts);
        emit setActionSaveEnabled(true, MainWidgets::Concerts);
    }
}

void ConcertWidget::onInfoLoadDone(Concert* concert)
{
    if (m_concert == nullptr) {
        return;
    }

    if (m_concert == concert) {
        updateConcertInfo();
        onInfoChanged();
        emit setActionSaveEnabled(false, MainWidgets::Concerts);
    }
}

void ConcertWidget::onLoadDone(Concert* concert)
{
    if (m_concert == nullptr || m_concert != concert) {
        return;
    }
    setEnabledTrue();
    ui->fanarts->setLoading(false);
}

void ConcertWidget::onLoadImagesStarted(Concert* concert)
{
    Q_UNUSED(concert);
    // emit actorDownloadStarted(tr("Downloading images..."), Constants::MovieProgressMessageId+movie->movieId());
}

void ConcertWidget::onLoadingImages(Concert* concert, QVector<ImageType> imageTypes)
{
    if (concert != m_concert) {
        return;
    }

    for (const auto imageType : imageTypes) {
        for (auto* cImage : ui->artStackedWidget->findChildren<ClosableImage*>()) {
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

void ConcertWidget::onSetImage(Concert* concert, ImageType type, QByteArray imageData)
{
    if (concert != m_concert) {
        return;
    }

    if (type == ImageType::ConcertExtraFanart) {
        ui->fanarts->addImage(imageData);
        return;
    }

    for (auto* image : ui->artStackedWidget->findChildren<ClosableImage*>()) {
        if (image->imageType() == type) {
            image->setLoading(false);
            image->setImage(imageData);
        }
    }
}

void ConcertWidget::onDownloadProgress(Concert* concert, int current, int maximum)
{
    Q_UNUSED(concert);
    Q_UNUSED(current);
    Q_UNUSED(maximum);
    // emit actorDownloadProgress(maximum-current, maximum, Constants::MovieProgressMessageId+movie->movieId());
}


/**
 * \brief Updates the contents of the widget with the current concert infos
 */
void ConcertWidget::updateConcertInfo()
{
    if (m_concert == nullptr) {
        qWarning() << "[ConcertWidget] Concert is invalid; can't update";
        return;
    }

    clear();

    ui->concertInfo->updateConcertInfo();
    ui->concertStreamdetails->updateConcertInfo();

    ui->concertName->setText(m_concert->title());

    QStringList genres;
    QStringList tags;
    for (const Concert* concert : Manager::instance()->concertModel()->concerts()) {
        genres.append(concert->genres());
        tags.append(concert->tags());
    }

    // `setTags` requires distinct lists
    genres.removeDuplicates();
    tags.removeDuplicates();

    ui->genreCloud->setTags(genres, m_concert->genres());
    ui->tagCloud->setTags(tags, m_concert->tags());

    updateImages(QVector<ImageType>{ImageType::ConcertPoster,
        ImageType::ConcertBackdrop,
        ImageType::ConcertLogo,
        ImageType::ConcertCdArt,
        ImageType::ConcertClearArt});

    ui->fanarts->setImages(m_concert->extraFanarts(Manager::instance()->mediaCenterInterfaceConcert()));

    emit setActionSaveEnabled(true, MainWidgets::Concerts);

    ui->buttonRevert->setVisible(m_concert->hasChanged());
}

void ConcertWidget::updateImages(QVector<ImageType> images)
{
    for (const auto imageType : images) {
        for (auto* cImage : ui->artStackedWidget->findChildren<ClosableImage*>()) {
            if (cImage->imageType() == imageType) {
                updateImage(imageType, cImage);
                break;
            }
        }
    }
}

void ConcertWidget::updateImage(ImageType imageType, ClosableImage* image)
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
 * \brief Saves concert information
 */
void ConcertWidget::onSaveInformation()
{
    QVector<Concert*> concerts = ConcertFilesWidget::instance()->selectedConcerts();
    if (concerts.count() == 0) {
        concerts.append(m_concert);
    }

    setDisabledTrue();
    m_savingWidget->show();

    if (concerts.count() == 1) {
        m_concert->controller()->saveData(Manager::instance()->mediaCenterInterfaceConcert());
        m_concert->controller()->loadData(Manager::instance()->mediaCenterInterfaceConcert(), true);
        updateConcertInfo();
        NotificationBox::instance()->showSuccess(tr("<b>\"%1\"</b> Saved").arg(m_concert->title()));
    } else {
        for (Concert* concert : concerts) {
            if (concert->hasChanged()) {
                concert->controller()->saveData(Manager::instance()->mediaCenterInterfaceConcert());
                concert->controller()->loadData(Manager::instance()->mediaCenterInterfaceConcert(), true);
                if (m_concert == concert) {
                    updateConcertInfo();
                }
            }
        }
        NotificationBox::instance()->showSuccess(tr("Concerts Saved"));
    }

    setEnabledTrue();
    m_savingWidget->hide();
    ui->buttonRevert->setVisible(false);
}

/**
 * \brief Saves all changed concerts
 */
void ConcertWidget::onSaveAll()
{
    setDisabledTrue();
    m_savingWidget->show();

    for (Concert* concert : Manager::instance()->concertModel()->concerts()) {
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
    NotificationBox::instance()->showSuccess(tr("All Concerts Saved"));
    ui->buttonRevert->setVisible(false);
}

/**
 * \brief Revert changes for current concert
 */
void ConcertWidget::onRevertChanges()
{
    m_concert->controller()->loadData(Manager::instance()->mediaCenterInterfaceConcert(), true);
    updateConcertInfo();
}

/*** add/remove/edit Genres ***/

/**
 * \brief Adds a genre
 */
void ConcertWidget::addGenre(QString genre)
{
    if (m_concert == nullptr) {
        return;
    }
    m_concert->addGenre(genre);
    onInfoChanged();
}

/**
 * \brief Removes a genre
 */
void ConcertWidget::removeGenre(QString genre)
{
    if (m_concert == nullptr) {
        return;
    }
    m_concert->removeGenre(genre);
    onInfoChanged();
}

void ConcertWidget::addTag(QString tag)
{
    if (m_concert == nullptr) {
        return;
    }
    m_concert->addTag(tag);
    onInfoChanged();
}

void ConcertWidget::removeTag(QString tag)
{
    if (m_concert == nullptr) {
        return;
    }
    m_concert->removeTag(tag);
    onInfoChanged();
}

/**
 * \brief Shows the first page with movie art
 */
void ConcertWidget::onArtPageOne()
{
    ui->artStackedWidget->slideInIdx(0);
    ui->buttonArtPageTwo->setChecked(false);
    ui->buttonArtPageOne->setChecked(true);
}

/**
 * \brief Shows the second page with movie art
 */
void ConcertWidget::onArtPageTwo()
{
    ui->artStackedWidget->slideInIdx(1);
    ui->buttonArtPageOne->setChecked(false);
    ui->buttonArtPageTwo->setChecked(true);
}

/*** Pass GUI events to concert object ***/


void ConcertWidget::onRemoveExtraFanart(QByteArray image)
{
    if (m_concert == nullptr) {
        return;
    }
    m_concert->removeExtraFanart(image);
    onInfoChanged();
}

void ConcertWidget::onRemoveExtraFanart(QString file)
{
    if (m_concert == nullptr) {
        return;
    }
    m_concert->removeExtraFanart(file);
    onInfoChanged();
}

void ConcertWidget::onAddExtraFanart()
{
    if (m_concert == nullptr) {
        return;
    }

    auto* imageDialog = new ImageDialog(this);
    imageDialog->setImageType(ImageType::ConcertExtraFanart);
    imageDialog->setMultiSelection(true);
    imageDialog->setConcert(m_concert);
    imageDialog->setDefaultDownloads(m_concert->backdrops());

    imageDialog->execWithType(ImageType::ConcertBackdrop);
    const int exitCode = imageDialog->result();
    const QVector<QUrl> imageUrls = imageDialog->imageUrls();
    imageDialog->deleteLater();

    if (exitCode == QDialog::Accepted && !imageUrls.isEmpty()) {
        ui->fanarts->setLoading(true);
        emit setActionSaveEnabled(false, MainWidgets::Concerts);
        m_concert->controller()->loadImages(ImageType::ConcertExtraFanart, imageUrls);
        onInfoChanged();
    }
}

void ConcertWidget::onExtraFanartDropped(QUrl imageUrl)
{
    if (m_concert == nullptr) {
        return;
    }
    ui->fanarts->setLoading(true);
    emit setActionSaveEnabled(false, MainWidgets::Concerts);
    m_concert->controller()->loadImages(ImageType::ConcertExtraFanart, QVector<QUrl>() << imageUrl);
    onInfoChanged();
}

void ConcertWidget::onChooseImage()
{
    if (m_concert == nullptr) {
        return;
    }

    auto* image = dynamic_cast<ClosableImage*>(QObject::sender());
    if (image == nullptr) {
        return;
    }

    auto* imageDialog = new ImageDialog(this);
    imageDialog->setImageType(image->imageType());
    imageDialog->setConcert(m_concert);
    if (image->imageType() == ImageType::ConcertPoster) {
        imageDialog->setDefaultDownloads(m_concert->posters());
    } else if (image->imageType() == ImageType::ConcertBackdrop) {
        imageDialog->setDefaultDownloads(m_concert->backdrops());
    }

    imageDialog->execWithType(image->imageType());
    const int exitCode = imageDialog->result();
    const QUrl imageUrl = imageDialog->imageUrl();
    imageDialog->deleteLater();

    if (exitCode == QDialog::Accepted) {
        emit setActionSaveEnabled(false, MainWidgets::Concerts);
        m_concert->controller()->loadImage(image->imageType(), imageUrl);
        onInfoChanged();
    }
}

void ConcertWidget::onDeleteImage()
{
    if (m_concert == nullptr) {
        return;
    }

    auto* image = dynamic_cast<ClosableImage*>(QObject::sender());
    if (image == nullptr) {
        return;
    }

    m_concert->removeImage(image->imageType());
    updateImages({image->imageType()});
    onInfoChanged();
}

void ConcertWidget::onImageDropped(ImageType imageType, QUrl imageUrl)
{
    if (m_concert == nullptr) {
        return;
    }
    emit setActionSaveEnabled(false, MainWidgets::Concerts);
    m_concert->controller()->loadImage(imageType, imageUrl);
    onInfoChanged();
}
