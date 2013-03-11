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
#include "data/ImageCache.h"
#include "globals/ComboDelegate.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "main/MessageBox.h"

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
    ui->buttonPreviewPoster->setEnabled(false);
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewLogo->setEnabled(false);
    ui->buttonPreviewClearArt->setEnabled(false);
    ui->buttonPreviewCdArt->setEnabled(false);
    ui->artStackedWidget->setAnimation(QEasingCurve::OutCubic);
    ui->artStackedWidget->setSpeed(300);

    QFont font = ui->concertName->font();
    font.setPointSize(font.pointSize()+4);
    ui->concertName->setFont(font);

    font = ui->posterResolution->font();
    #ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->posterResolution->setFont(font);
    ui->backdropResolution->setFont(font);
    ui->logoResolution->setFont(font);
    ui->clearArtResolution->setFont(font);
    ui->cdArtResolution->setFont(font);

    m_concert = 0;
    m_posterDownloadManager = new DownloadManager(this);

    connect(ui->poster, SIGNAL(clicked()), this, SLOT(chooseConcertPoster()));
    connect(ui->backdrop, SIGNAL(clicked()), this, SLOT(chooseConcertBackdrop()));
    connect(ui->logo, SIGNAL(clicked()), this, SLOT(chooseConcertLogo()));
    connect(ui->clearArt, SIGNAL(clicked()), this, SLOT(chooseConcertClearArt()));
    connect(ui->cdArt, SIGNAL(clicked()), this, SLOT(chooseConcertCdArt()));
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(posterDownloadFinished(DownloadManagerElement)));
    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(concertNameChanged(QString)));
    connect(ui->buttonPreviewPoster, SIGNAL(clicked()), this, SLOT(onPreviewPoster()));
    connect(ui->buttonPreviewBackdrop, SIGNAL(clicked()), this, SLOT(onPreviewBackdrop()));
    connect(ui->buttonPreviewLogo, SIGNAL(clicked()), this, SLOT(onPreviewLogo()));
    connect(ui->buttonPreviewClearArt, SIGNAL(clicked()), this, SLOT(onPreviewClearArt()));
    connect(ui->buttonPreviewCdArt, SIGNAL(clicked()), this, SLOT(onPreviewCdArt()));
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
    connect(ui->watched, SIGNAL(stateChanged(int)), this, SLOT(onWatchedChange(int)));
    connect(ui->released, SIGNAL(dateChanged(QDate)), this, SLOT(onReleasedChange(QDate)));
    connect(ui->lastPlayed, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(onLastWatchedChange(QDateTime)));
    connect(ui->overview, SIGNAL(textChanged()), this, SLOT(onOverviewChange()));
    connect(ui->videoAspectRatio, SIGNAL(valueChanged(double)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoCodec, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoDuration, SIGNAL(timeChanged(QTime)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoHeight, SIGNAL(valueChanged(int)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoWidth, SIGNAL(valueChanged(int)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoScantype, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));

    QPixmap zoomIn(":/img/zoom_in.png");
    QPainter p;
    p.begin(&zoomIn);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(zoomIn.rect(), QColor(0, 0, 0, 150));
    p.end();
    ui->buttonPreviewBackdrop->setIcon(QIcon(zoomIn));
    ui->buttonPreviewPoster->setIcon(QIcon(zoomIn));
    ui->buttonPreviewLogo->setIcon(QIcon(zoomIn));
    ui->buttonPreviewClearArt->setIcon(QIcon(zoomIn));
    ui->buttonPreviewCdArt->setIcon(QIcon(zoomIn));

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
    ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
    ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->logo->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->clearArt->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->cdArt->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->posterResolution->setText("");
    ui->backdropResolution->setText("");
    ui->logoResolution->setText("");
    ui->clearArtResolution->setText("");
    ui->cdArtResolution->setText("");
    ui->genreCloud->clear();
    ui->fanarts->clear();

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
    if (!concert->streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails()) {
        concert->loadStreamDetailsFromFile();
        if (concert->streamDetailsLoaded() && concert->streamDetails()->videoDetails().value("durationinseconds").toInt() != 0)
            concert->setRuntime(qFloor(concert->streamDetails()->videoDetails().value("durationinseconds").toInt()/60));
    }
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
        connect(m_concert, SIGNAL(loaded(Concert*)), this, SLOT(infoLoadDone(Concert*)), Qt::UniqueConnection);
    } else {
        emit setActionSearchEnabled(true, WidgetConcerts);
        emit setActionSaveEnabled(true, WidgetConcerts);
    }
}

/**
 * @brief ConcertWidget::infoLoadDone
 * @param concert
 */
void ConcertWidget::infoLoadDone(Concert *concert)
{
    QList<int> types;
    if (concert->infosToLoad().contains(ConcertScraperInfos::ExtraArts))
        types << TypeClearArt << TypeCdArt << TypeLogo;
    if (!concert->tmdbId().isEmpty() && !types.isEmpty()) {
        Manager::instance()->fanartTv()->concertImages(concert, concert->tmdbId(), types);
        connect(Manager::instance()->fanartTv(), SIGNAL(sigImagesLoaded(Concert*,QMap<int,QList<Poster> >)), this, SLOT(loadDone(Concert*,QMap<int,QList<Poster> >)), Qt::UniqueConnection);
    } else {
        QMap<int, QList<Poster> > map;
        loadDone(concert, map);
    }
}

/**
 * @brief Called when the search widget finishes
 * Updates infos and starts downloads
 * @param concert Concert
 * @param posters
 */
void ConcertWidget::loadDone(Concert *concert, QMap<int, QList<Poster> > posters)
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

    if (concert->infosToLoad().contains(ConcertScraperInfos::Poster) && concert->posters().size() > 0) {
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

    if (concert->infosToLoad().contains(ConcertScraperInfos::Backdrop) && concert->backdrops().size() > 0) {
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

    QMapIterator<int, QList<Poster> > it(posters);
    while (it.hasNext()) {
        it.next();
        if (it.key() == TypeClearArt && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = TypeClearArt;
            d.url = it.value().at(0).originalUrl;
            d.concert = concert;
            m_posterDownloadManager->addDownload(d);
            if (m_concert == concert) {
                ui->clearArt->setPixmap(QPixmap());
                ui->clearArt->setMovie(m_loadingMovie);
            }
            downloadsSize++;
        } else if (it.key() == TypeCdArt && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = TypeCdArt;
            d.url = it.value().at(0).originalUrl;
            d.concert = concert;
            m_posterDownloadManager->addDownload(d);
            if (m_concert == concert) {
                ui->cdArt->setPixmap(QPixmap());
                ui->cdArt->setMovie(m_loadingMovie);
            }
            downloadsSize++;
        } else if (it.key() == TypeLogo && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = TypeLogo;
            d.url = it.value().at(0).originalUrl;
            d.concert = concert;
            m_posterDownloadManager->addDownload(d);
            if (m_concert == concert) {
                ui->logo->setPixmap(QPixmap());
                ui->logo->setMovie(m_loadingMovie);
            }
            downloadsSize++;
        }
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
    ui->watched->setChecked(m_concert->watched());

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
    certifications.sort();
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

    if (!m_concert->posterImage().isNull()) {
        QImage img = QImage::fromData(m_concert->posterImage());
        ui->poster->setPixmap(QPixmap::fromImage(img).scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = img;
    } else if (!Manager::instance()->mediaCenterInterface()->posterImageName(m_concert).isEmpty()) {
        int w = 0;
        int h = 0;
        QImage img = ImageCache::instance()->image(Manager::instance()->mediaCenterInterface()->posterImageName(m_concert), 200, 0, w, h);
        ui->poster->setPixmap(QPixmap::fromImage(img));
        ui->posterResolution->setText(QString("%1x%2").arg(w).arg(h));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = QImage();
    } else {
        ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
        ui->posterResolution->setText("");
        ui->buttonPreviewPoster->setEnabled(false);
    }

    if (!m_concert->backdropImage().isNull()) {
        QImage img = QImage::fromData(m_concert->backdropImage());
        ui->backdrop->setPixmap(QPixmap::fromImage(img).scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = img;
    } else if (!Manager::instance()->mediaCenterInterface()->backdropImageName(m_concert).isEmpty()) {
        int w = 0;
        int h = 0;
        QImage img = ImageCache::instance()->image(Manager::instance()->mediaCenterInterface()->backdropImageName(m_concert), 200, 0, w, h);
        ui->backdrop->setPixmap(QPixmap::fromImage(img));
        ui->backdropResolution->setText(QString("%1x%2").arg(w).arg(h));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = QImage();
    } else {
        ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText("");
        ui->buttonPreviewBackdrop->setEnabled(false);
    }

    // Logo
    if (!m_concert->logoImage().isNull()) {
        QImage img = QImage::fromData(m_concert->logoImage());
        ui->logo->setPixmap(QPixmap::fromImage(img).scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->logoResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewLogo->setEnabled(true);
        m_currentLogo = img;
    } else if (!Manager::instance()->mediaCenterInterface()->logoImageName(m_concert).isEmpty()) {
        int w = 0;
        int h = 0;
        QImage img = ImageCache::instance()->image(Manager::instance()->mediaCenterInterface()->logoImageName(m_concert), 200, 100, w, h);
        ui->logo->setPixmap(QPixmap::fromImage(img));
        ui->logoResolution->setText(QString("%1x%2").arg(w).arg(h));
        ui->buttonPreviewLogo->setEnabled(true);
        m_currentLogo = QImage();
    } else {
        ui->logo->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->logoResolution->setText("");
        ui->buttonPreviewLogo->setEnabled(false);
    }

    // Clear art
    if (!m_concert->clearArtImage().isNull()) {
        QImage img = QImage::fromData(m_concert->clearArtImage());
        ui->clearArt->setPixmap(QPixmap::fromImage(img).scaled(200, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->clearArtResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewClearArt->setEnabled(true);
        m_currentClearArt = img;
    } else if (!Manager::instance()->mediaCenterInterface()->clearArtImageName(m_concert).isEmpty()) {
        int w = 0;
        int h = 0;
        QImage img = ImageCache::instance()->image(Manager::instance()->mediaCenterInterface()->clearArtImageName(m_concert), 200, 150, w, h);
        ui->clearArt->setPixmap(QPixmap::fromImage(img));
        ui->clearArtResolution->setText(QString("%1x%2").arg(w).arg(h));
        ui->buttonPreviewClearArt->setEnabled(true);
        m_currentClearArt = QImage();
    } else {
        ui->clearArt->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->clearArtResolution->setText("");
        ui->buttonPreviewClearArt->setEnabled(false);
    }

    // CD Art
    if (!m_concert->cdArtImage().isNull()) {
        QImage img = QImage::fromData(m_concert->cdArtImage());
        ui->cdArt->setPixmap(QPixmap::fromImage(img).scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->cdArtResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewCdArt->setEnabled(true);
        m_currentCdArt = img;
    } else if (!Manager::instance()->mediaCenterInterface()->cdArtImageName(m_concert).isEmpty()) {
        int w = 0;
        int h = 0;
        QImage img = ImageCache::instance()->image(Manager::instance()->mediaCenterInterface()->cdArtImageName(m_concert), 200, 200, w, h);
        ui->cdArt->setPixmap(QPixmap::fromImage(img));
        ui->cdArtResolution->setText(QString("%1x%2").arg(w).arg(h));
        ui->buttonPreviewCdArt->setEnabled(true);
        m_currentCdArt = QImage();
    } else {
        ui->cdArt->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->cdArtResolution->setText("");
        ui->buttonPreviewCdArt->setEnabled(false);
    }

    ui->fanarts->setImages(m_concert->extraFanarts(Manager::instance()->mediaCenterInterfaceConcert()));

    ui->rating->blockSignals(false);
    ui->runtime->blockSignals(false);
    ui->playcount->blockSignals(false);
    ui->watched->blockSignals(false);
    ui->released->blockSignals(false);
    ui->lastPlayed->blockSignals(false);
    ui->overview->blockSignals(false);

    emit setActionSaveEnabled(true, WidgetConcerts);

    ui->rating->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertRating));
    ui->tagline->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertTagline));
    ui->certification->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertCertification));
    ui->trailer->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertTrailer));
    ui->watched->setEnabled(Manager::instance()->mediaCenterInterfaceConcert()->hasFeature(MediaCenterFeatures::EditConcertWatched));

    ui->buttonRevert->setVisible(m_concert->hasChanged());
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

    if (reloadFromFile)
        m_concert->loadStreamDetailsFromFile();

    StreamDetails *streamDetails = m_concert->streamDetails();
    ui->videoWidth->setValue(streamDetails->videoDetails().value("width").toInt());
    ui->videoHeight->setValue(streamDetails->videoDetails().value("height").toInt());
    ui->videoAspectRatio->setValue(streamDetails->videoDetails().value("aspect").toDouble());
    ui->videoCodec->setText(streamDetails->videoDetails().value("codec"));
    ui->videoScantype->setText(streamDetails->videoDetails().value("scantype"));
    QTime time;
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
        ui->streamDetails->addWidget(label, 7+i, 0);
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
        ui->streamDetails->addLayout(layout, 7+i, 1);
        m_streamDetailsWidgets << label << edit1 << edit2 << edit3;
        m_streamDetailsAudio << (QList<QLineEdit*>() << edit1 << edit2 << edit3);
        connect(edit1, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
        connect(edit2, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
        connect(edit3, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
    }

    if (!streamDetails->subtitleDetails().isEmpty()) {
        QLabel *label = new QLabel(tr("Subtitles"));
        QFont font = label->font();
        font.setBold(true);
        label->setFont(font);
        ui->streamDetails->addWidget(label, 7+audioTracks, 0);
        m_streamDetailsWidgets << label;

        for (int i=0, n=streamDetails->subtitleDetails().count() ; i<n ; ++i) {
            QLabel *label = new QLabel(tr("Track %1").arg(i+1));
            ui->streamDetails->addWidget(label, 8+audioTracks+i, 0);
            QLineEdit *edit1 = new QLineEdit(streamDetails->subtitleDetails().at(i).value("language"));
            edit1->setToolTip(tr("Language"));
            edit1->setPlaceholderText(tr("Language"));
            QHBoxLayout *layout = new QHBoxLayout();
            layout->addWidget(edit1);
            layout->addStretch(10);
            ui->streamDetails->addLayout(layout, 8+audioTracks+i, 1);
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
    ImageDialog::instance()->setConcert(m_concert);
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
    ImageDialog::instance()->setConcert(m_concert);
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
 * @brief Shows the ImageDialog and after successful execution starts logo download
 */
void ConcertWidget::chooseConcertLogo()
{
    if (m_concert == 0)
        return;

    ImageDialog::instance()->setImageType(TypeLogo);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setConcert(m_concert);
    ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(ImageDialogType::ConcertLogo);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetConcerts);
        DownloadManagerElement d;
        d.imageType = TypeLogo;
        d.url = ImageDialog::instance()->imageUrl();
        d.concert = m_concert;
        m_posterDownloadManager->addDownload(d);
        ui->logo->setPixmap(QPixmap());
        ui->logo->setMovie(m_loadingMovie);
        ui->buttonPreviewLogo->setEnabled(false);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the ImageDialog and after successful execution starts clear art download
 */
void ConcertWidget::chooseConcertClearArt()
{
    if (m_concert == 0)
        return;

    ImageDialog::instance()->setImageType(TypeClearArt);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setConcert(m_concert);
    ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(ImageDialogType::ConcertClearArt);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetConcerts);
        DownloadManagerElement d;
        d.imageType = TypeClearArt;
        d.url = ImageDialog::instance()->imageUrl();
        d.concert = m_concert;
        m_posterDownloadManager->addDownload(d);
        ui->clearArt->setPixmap(QPixmap());
        ui->clearArt->setMovie(m_loadingMovie);
        ui->buttonPreviewClearArt->setEnabled(false);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the ImageDialog and after successful execution starts cd art download
 */
void ConcertWidget::chooseConcertCdArt()
{
    if (m_concert == 0)
        return;

    ImageDialog::instance()->setImageType(TypeCdArt);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setConcert(m_concert);
    ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(ImageDialogType::ConcertCdArt);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetConcerts);
        DownloadManagerElement d;
        d.imageType = TypeCdArt;
        d.url = ImageDialog::instance()->imageUrl();
        d.concert = m_concert;
        m_posterDownloadManager->addDownload(d);
        ui->cdArt->setPixmap(QPixmap());
        ui->cdArt->setMovie(m_loadingMovie);
        ui->buttonPreviewCdArt->setEnabled(false);
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
            QImage img = QImage::fromData(elem.data);
            ui->poster->setPixmap(QPixmap::fromImage(img).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->posterResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
            ui->buttonPreviewPoster->setEnabled(true);
            m_currentPoster = img;
        }
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->posterImageName(elem.concert));
        elem.concert->setPosterImage(elem.data);
    } else if (elem.imageType == TypeBackdrop) {
        qDebug() << "Got a backdrop";
        Helper::resizeBackdrop(elem.data);
        QImage img = QImage::fromData(elem.data);
        if (m_concert == elem.concert) {
            ui->backdrop->setPixmap(QPixmap::fromImage(img).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->backdropResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
            ui->buttonPreviewBackdrop->setEnabled(true);
            m_currentBackdrop = img;
        }
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->backdropImageName(elem.concert));
        elem.concert->setBackdropImage(elem.data);
    } else if (elem.imageType == TypeLogo) {
        qDebug() << "Got a logo";
        if (m_concert == elem.concert) {
            QImage img = QImage::fromData(elem.data);
            ui->logo->setPixmap(QPixmap::fromImage(img).scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->logoResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
            ui->buttonPreviewLogo->setEnabled(true);
            m_currentLogo = img;
        }
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->logoImageName(elem.concert));
        elem.concert->setLogoImage(elem.data);
    } else if (elem.imageType == TypeClearArt) {
        qDebug() << "Got a clear art";
        if (m_concert == elem.concert) {
            QImage img = QImage::fromData(elem.data);
            ui->clearArt->setPixmap(QPixmap::fromImage(img).scaled(200, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->clearArtResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
            ui->buttonPreviewClearArt->setEnabled(true);
            m_currentClearArt = img;
        }
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->clearArtImageName(elem.concert));
        elem.concert->setClearArtImage(elem.data);
    } else if (elem.imageType == TypeCdArt) {
        qDebug() << "Got a cd art";
        if (m_concert == elem.concert) {
            QImage img = QImage::fromData(elem.data);
            ui->cdArt->setPixmap(QPixmap::fromImage(img).scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->cdArtResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
            ui->buttonPreviewCdArt->setEnabled(true);
            m_currentCdArt = img;
        }
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->cdArtImageName(elem.concert));
        elem.concert->setCdArtImage(elem.data);
    } else if (elem.imageType == TypeExtraFanart) {
        Helper::resizeBackdrop(elem.data);
        elem.concert->addExtraFanart(elem.data);
        if (elem.concert == m_concert)
            ui->fanarts->addImage(elem.data);
    }
    if (m_posterDownloadManager->downloadQueueSize() == 0) {
        emit setActionSaveEnabled(true, WidgetConcerts);
        elem.concert->setDownloadsInProgress(false);
        ui->fanarts->setLoading(false);
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
 * @brief Shows a full size image of the backdrop
 */
void ConcertWidget::onPreviewBackdrop()
{
    if (m_currentBackdrop.isNull())
        m_currentBackdrop = QImage(Manager::instance()->mediaCenterInterface()->backdropImageName(m_concert));
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentBackdrop));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows a full size image of the poster
 */
void ConcertWidget::onPreviewPoster()
{
    if (m_currentPoster.isNull())
        m_currentPoster = QImage(Manager::instance()->mediaCenterInterface()->posterImageName(m_concert));
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentPoster));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows a full size image of the logo
 */
void ConcertWidget::onPreviewLogo()
{
    if (m_currentLogo.isNull())
        m_currentLogo = QImage(Manager::instance()->mediaCenterInterface()->logoImageName(m_concert));
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentLogo));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows a full size image of the clear art
 */
void ConcertWidget::onPreviewClearArt()
{
    if (m_currentClearArt.isNull())
        m_currentClearArt = QImage(Manager::instance()->mediaCenterInterface()->clearArtImageName(m_concert));
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentClearArt));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows a full size image of the cd art
 */
void ConcertWidget::onPreviewCdArt()
{
    if (m_currentCdArt.isNull())
        m_currentCdArt= QImage(Manager::instance()->mediaCenterInterface()->cdArtImageName(m_concert));
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentCdArt));
    ImagePreviewDialog::instance()->exec();
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

    ImageDialog::instance()->setImageType(TypeExtraFanart);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMultiSelection(true);
    ImageDialog::instance()->setConcert(m_concert);
    ImageDialog::instance()->setDownloads(m_concert->backdrops());
    ImageDialog::instance()->exec(ImageDialogType::ConcertBackdrop);

    if (ImageDialog::instance()->result() == QDialog::Accepted && !ImageDialog::instance()->imageUrls().isEmpty()) {
        ui->fanarts->setLoading(true);
        emit setActionSaveEnabled(false, WidgetConcerts);
        foreach (const QUrl &url, ImageDialog::instance()->imageUrls()) {
            DownloadManagerElement d;
            d.imageType = TypeExtraFanart;
            d.url = url;
            d.concert = m_concert;
            m_posterDownloadManager->addDownload(d);
        }
        ui->buttonRevert->setVisible(true);
    }
}
