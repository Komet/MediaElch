#include "TvShowWidgetTvShow.h"
#include "ui_TvShowWidgetTvShow.h"

#include <QFileDialog>
#include <QMovie>
#include <QPainter>
#include <utility>

#include "data/ImageCache.h"
#include "globals/ComboDelegate.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"
#include "globals/ScraperInfos.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "ui/notifications/NotificationBox.h"
#include "ui/tv_show/TvShowSearch.h"
#include "ui/tv_show/TvTunesDialog.h"

using namespace mediaelch;

TvShowWidgetTvShow::TvShowWidgetTvShow(QWidget* parent) :
    QWidget(parent), ui(new Ui::TvShowWidgetTvShow), m_show{nullptr}
{
    ui->setupUi(this);

    ui->showTitle->clear();
    ui->actors->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->artStackedWidget->setAnimation(QEasingCurve::OutCubic);
    ui->artStackedWidget->setSpeed(300);

    QFont font = ui->labelClearArt->font();
#ifdef Q_OS_WIN
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif

#ifndef Q_OS_MAC
    QFont nameFont = ui->showTitle->font();
    nameFont.setPointSize(nameFont.pointSize() - 4);
    ui->showTitle->setFont(nameFont);
#endif

    ui->labelClearArt->setFont(font);
    ui->labelCharacterArt->setFont(font);
    ui->labelFanart->setFont(font);
    ui->labelLogo->setFont(font);
    ui->labelPoster->setFont(font);
    ui->labelBanner->setFont(font);
    ui->labelThumb->setFont(font);

    ui->poster->setDefaultPixmap(QPixmap(":/img/placeholders/poster.png"));
    ui->backdrop->setDefaultPixmap(QPixmap(":/img/placeholders/fanart.png"));
    ui->logo->setDefaultPixmap(QPixmap(":/img/placeholders/logo.png"));
    ui->clearArt->setDefaultPixmap(QPixmap(":/img/placeholders/clear_art.png"));
    ui->banner->setDefaultPixmap(QPixmap(":/img/placeholders/banner.png"));
    ui->characterArt->setDefaultPixmap(QPixmap(":/img/placeholders/character_art.png"));
    ui->thumb->setDefaultPixmap(QPixmap(":/img/placeholders/thumb.png"));

    font = ui->actorResolution->font();
#ifdef Q_OS_WIN
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->actorResolution->setFont(font);

    ui->badgeTuneExisting->setBadgeType(Badge::Type::LabelSuccess);
    ui->badgeTuneMissing->setBadgeType(Badge::Type::LabelWarning);

    ui->btnDownloadTune->setIcon(Manager::instance()->iconFont()->icon("download", QColor(150, 150, 150), "", -1, 1.0));

    ui->genreCloud->setText(tr("Genres"));
    ui->genreCloud->setPlaceholder(tr("Add Genre"));
    connect(ui->genreCloud, &TagCloud::activated, this, &TvShowWidgetTvShow::onAddGenre);
    connect(ui->genreCloud, &TagCloud::deactivated, this, &TvShowWidgetTvShow::onRemoveGenre);

    ui->tagCloud->setText(tr("Tags"));
    ui->tagCloud->setPlaceholder(tr("Add Tag"));
    connect(ui->tagCloud, &TagCloud::activated, this, &TvShowWidgetTvShow::onAddTag);
    connect(ui->tagCloud, &TagCloud::deactivated, this, &TvShowWidgetTvShow::onRemoveTag);

    ui->comboStatus->setItemData(0, "");
    ui->comboStatus->setItemData(1, "Continuing");
    ui->comboStatus->setItemData(2, "Ended");

    m_loadingMovie = new QMovie(":/img/spinner.gif", QByteArray(), this);
    m_loadingMovie->start();
    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    m_posterDownloadManager = new DownloadManager(this);

    ui->poster->setImageType(ImageType::TvShowPoster);
    ui->backdrop->setImageType(ImageType::TvShowBackdrop);
    ui->logo->setImageType(ImageType::TvShowLogos);
    ui->characterArt->setImageType(ImageType::TvShowCharacterArt);
    ui->banner->setImageType(ImageType::TvShowBanner);
    ui->thumb->setImageType(ImageType::TvShowThumb);
    ui->clearArt->setImageType(ImageType::TvShowClearArt);
    for (ClosableImage* image : ui->artStackedWidget->findChildren<ClosableImage*>()) {
        connect(image, &ClosableImage::clicked, this, &TvShowWidgetTvShow::onChooseImage);
        connect(image, &ClosableImage::sigClose, this, &TvShowWidgetTvShow::onDeleteImage);
        connect(image, &ClosableImage::sigImageDropped, this, &TvShowWidgetTvShow::onImageDropped);
    }

    QPixmap pixmap(":/img/man.png");
    helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
    ui->actor->setPixmap(pixmap);

    connect(ui->title, &QLineEdit::textChanged, ui->showTitle, &QLabel::setText);
    connect(ui->buttonAddActor, &QAbstractButton::clicked, this, &TvShowWidgetTvShow::onAddActor);
    connect(ui->buttonRemoveActor, &QAbstractButton::clicked, this, &TvShowWidgetTvShow::onRemoveActor);
    connect(m_posterDownloadManager,
        &DownloadManager::sigDownloadFinished,
        this,
        &TvShowWidgetTvShow::onPosterDownloadFinished,
        static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    connect(m_posterDownloadManager, &DownloadManager::showDownloadsLeft, this, &TvShowWidgetTvShow::onDownloadsLeft);
    connect(ui->actors, &QTableWidget::itemSelectionChanged, this, &TvShowWidgetTvShow::onActorChanged);
    connect(ui->actor, &MyLabel::clicked, this, &TvShowWidgetTvShow::onChangeActorImage);
    connect(ui->buttonRevert, &QAbstractButton::clicked, this, &TvShowWidgetTvShow::onRevertChanges);
    connect(ui->btnDownloadTune, &QAbstractButton::clicked, this, &TvShowWidgetTvShow::onDownloadTune);

    connect(ui->fanarts,
        elchOverload<QByteArray>(&ImageGallery::sigRemoveImage),
        this,
        elchOverload<QByteArray>(&TvShowWidgetTvShow::onRemoveExtraFanart));
    connect(ui->fanarts,
        elchOverload<QString>(&ImageGallery::sigRemoveImage),
        this,
        elchOverload<QString>(&TvShowWidgetTvShow::onRemoveExtraFanart));
    connect(ui->btnAddExtraFanart, &QAbstractButton::clicked, this, &TvShowWidgetTvShow::onAddExtraFanart);
    connect(ui->fanarts, &ImageGallery::sigImageDropped, this, &TvShowWidgetTvShow::onExtraFanartDropped);

    onClear();

    // Connect GUI change events to movie object
    // clang-format off
    connect(ui->title,         &QLineEdit::textEdited,           this, &TvShowWidgetTvShow::onTitleChange);
    connect(ui->imdbId,        &QLineEdit::textEdited,           this, &TvShowWidgetTvShow::onImdbIdChange);
    connect(ui->tmdbId,        &QLineEdit::textEdited,           this, &TvShowWidgetTvShow::onTmdbIdChange);
    connect(ui->tvdbId,        &QLineEdit::textEdited,           this, &TvShowWidgetTvShow::onTvdbIdChange);
    connect(ui->tvmazeId,      &QLineEdit::textEdited,           this, &TvShowWidgetTvShow::onTvMazeIdChange);
    connect(ui->sortTitle,     &QLineEdit::textEdited,           this, &TvShowWidgetTvShow::onSortTitleChange);
    connect(ui->originalTitle, &QLineEdit::textEdited,           this, &TvShowWidgetTvShow::onOriginalTitleChange);
    connect(ui->certification, &QComboBox::editTextChanged,      this, &TvShowWidgetTvShow::onCertificationChange);
    connect(ui->rating,        elchOverload<double>(&QDoubleSpinBox::valueChanged), this, &TvShowWidgetTvShow::onRatingChange);
    connect(ui->userRating,    elchOverload<double>(&QDoubleSpinBox::valueChanged), this, &TvShowWidgetTvShow::onUserRatingChange);
    connect(ui->votes,         elchOverload<int>(&QSpinBox::valueChanged),          this, &TvShowWidgetTvShow::onVotesChange);
    connect(ui->top250,        elchOverload<int>(&QSpinBox::valueChanged),          this, &TvShowWidgetTvShow::onTop250Change);
    connect(ui->firstAired,    &QDateTimeEdit::dateChanged,      this, &TvShowWidgetTvShow::onFirstAiredChange);
    connect(ui->studio,        &QLineEdit::textEdited,           this, &TvShowWidgetTvShow::onStudioChange);
    connect(ui->overview,      &QTextEdit::textChanged,          this, &TvShowWidgetTvShow::onOverviewChange);
    connect(ui->actors,        &QTableWidget::itemChanged,       this, &TvShowWidgetTvShow::onActorEdited);
    connect(ui->runtime,       elchOverload<int>(&QSpinBox::valueChanged),         this, &TvShowWidgetTvShow::onRuntimeChange);
    connect(ui->comboStatus,   elchOverload<int>(&QComboBox::currentIndexChanged), this, &TvShowWidgetTvShow::onStatusChange);
    // clang-format on

    ui->rating->setSingleStep(0.1);
    ui->rating->setMinimum(0.0);
    ui->userRating->setSingleStep(0.1);
    ui->userRating->setMinimum(0.0);

    onSetEnabled(false);

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
 * \brief TvShowWidgetTvShow::~TvShowWidgetTvShow
 */
TvShowWidgetTvShow::~TvShowWidgetTvShow()
{
    delete ui;
}

/**
 * \brief Repositions the saving widget
 */
void TvShowWidgetTvShow::resizeEvent(QResizeEvent* event)
{
    m_savingWidget->move(size().width() / 2 - m_savingWidget->width(), height() / 2 - m_savingWidget->height());
    QWidget::resizeEvent(event);
}

void TvShowWidgetTvShow::setBigWindow(bool bigWindow)
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
void TvShowWidgetTvShow::onClear()
{
    bool blocked = false;

    blocked = ui->certification->blockSignals(true);
    ui->certification->clear();
    ui->certification->blockSignals(blocked);

    blocked = ui->rating->blockSignals(true);
    ui->rating->clear();
    ui->rating->blockSignals(blocked);

    blocked = ui->userRating->blockSignals(true);
    ui->userRating->clear();
    ui->userRating->blockSignals(blocked);

    blocked = ui->votes->blockSignals(true);
    ui->votes->clear();
    ui->votes->blockSignals(blocked);

    blocked = ui->top250->blockSignals(true);
    ui->top250->clear();
    ui->top250->blockSignals(blocked);

    blocked = ui->firstAired->blockSignals(true);
    ui->firstAired->setDate(QDate::currentDate());
    ui->firstAired->blockSignals(blocked);

    blocked = ui->overview->blockSignals(true);
    ui->overview->clear();
    ui->overview->blockSignals(blocked);

    blocked = ui->runtime->blockSignals(true);
    ui->runtime->clear();
    ui->runtime->blockSignals(blocked);

    blocked = ui->comboStatus->blockSignals(true);
    ui->comboStatus->setCurrentIndex(0);
    ui->comboStatus->blockSignals(blocked);

    ui->showTitle->clear();
    ui->imdbId->clear();
    ui->tmdbId->clear();
    ui->tvdbId->clear();
    ui->tvmazeId->clear();
    ui->actors->setRowCount(0);
    ui->dir->clear();
    ui->title->clear();
    ui->sortTitle->clear();
    ui->originalTitle->clear();
    ui->studio->clear();
    ui->genreCloud->clear();
    ui->fanarts->clear();
    ui->poster->clear();
    ui->backdrop->clear();
    ui->banner->clear();
    ui->logo->clear();
    ui->thumb->clear();
    ui->clearArt->clear();
    ui->characterArt->clear();
    ui->tagCloud->clear();
    ui->badgeTuneExisting->setVisible(false);
    ui->badgeTuneMissing->setVisible(true);
    ui->buttonRevert->setVisible(false);
}

/**
 * \brief Toggles the enabled state of the main groupbox
 * \param enabled Status
 */
void TvShowWidgetTvShow::onSetEnabled(bool enabled)
{
    ui->groupBox_3->setEnabled(enabled);
}

/**
 * \brief Sets the current show and updates widgets contents
 * \param show Show object
 */
void TvShowWidgetTvShow::setTvShow(TvShow* show)
{
    qDebug() << "Entered, show=" << show->title();
    show->loadData(Manager::instance()->mediaCenterInterface());
    m_show = show;
    updateTvShowInfo();
    if (show->downloadsInProgress()) {
        onSetEnabled(false);
        emit sigSetActionSearchEnabled(false, MainWidgets::TvShows);
        emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
    } else {
        onSetEnabled(true);
        emit sigSetActionSearchEnabled(true, MainWidgets::TvShows);
        emit sigSetActionSaveEnabled(true, MainWidgets::TvShows);
    }
}

/**
 * \brief Updates the widgets contents
 */
void TvShowWidgetTvShow::updateTvShowInfo()
{
    if (m_show == nullptr) {
        qDebug() << "My show is invalid";
        return;
    }

    ui->certification->blockSignals(true);
    ui->rating->blockSignals(true);
    ui->userRating->blockSignals(true);
    ui->votes->blockSignals(true);
    ui->top250->blockSignals(true);
    ui->firstAired->blockSignals(true);
    ui->overview->blockSignals(true);
    ui->runtime->blockSignals(true);
    ui->comboStatus->blockSignals(true);

    onClear();

    ui->dir->setText(m_show->dir().toNativePathString());
    ui->title->setText(m_show->title());
    ui->originalTitle->setText(m_show->originalTitle());
    ui->sortTitle->setText(m_show->sortTitle());
    ui->imdbId->setText(m_show->imdbId().toString());
    ui->tmdbId->setText(m_show->tmdbId().toString());
    ui->tvdbId->setText(m_show->tvdbId().toString());
    ui->tvmazeId->setText(m_show->tvmazeId().toString());
    // TODO: multiple ratings
    if (!m_show->ratings().isEmpty()) {
        ui->rating->setValue(m_show->ratings().back().rating);
        ui->votes->setValue(m_show->ratings().back().voteCount);
    }
    ui->userRating->setValue(m_show->userRating());
    ui->top250->setValue(m_show->top250());
    ui->firstAired->setDate(m_show->firstAired());
    ui->studio->setText(m_show->network());
    ui->overview->setPlainText(m_show->overview());
    ui->runtime->setValue(static_cast<int>(m_show->runtime().count()));
    if (m_show->status() == "Continuing") {
        ui->comboStatus->setCurrentIndex(1);
    } else if (m_show->status() == "Ended") {
        ui->comboStatus->setCurrentIndex(2);
    } else {
        ui->comboStatus->setCurrentIndex(0);
    }

    ui->actors->blockSignals(true);
    for (Actor* actor : m_show->actors()) {
        int row = ui->actors->rowCount();
        ui->actors->insertRow(row);
        ui->actors->setItem(row, 0, new QTableWidgetItem(actor->name));
        ui->actors->setItem(row, 1, new QTableWidgetItem(actor->role));
        ui->actors->item(row, 0)->setData(Qt::UserRole, actor->thumb);
        ui->actors->item(row, 1)->setData(Qt::UserRole, QVariant::fromValue(actor));
    }
    ui->actors->blockSignals(false);

    QStringList genres;
    QStringList tags;
    for (const TvShow* show : Manager::instance()->tvShowModel()->tvShows()) {
        genres.append(show->genres());
        tags.append(show->tags());
    }

    // `setTags` requires distinct lists
    genres.removeDuplicates();
    tags.removeDuplicates();

    ui->genreCloud->setTags(genres, m_show->genres());
    ui->tagCloud->setTags(tags, m_show->tags());

    auto certifications = m_show->certifications();
    certifications.prepend(Certification::NoCertification);
    for (const auto& cert : certifications) {
        ui->certification->addItem(cert.toString());
    }
    ui->certification->setCurrentIndex(certifications.indexOf(m_show->certification()));

    updateImages(QVector<ImageType>() << ImageType::TvShowPoster       //
                                      << ImageType::TvShowBackdrop     //
                                      << ImageType::TvShowBanner       //
                                      << ImageType::TvShowCharacterArt //
                                      << ImageType::TvShowClearArt     //
                                      << ImageType::TvShowLogos        //
                                      << ImageType::TvShowThumb);
    ui->fanarts->setImages(m_show->extraFanarts(Manager::instance()->mediaCenterInterfaceTvShow()));

    ui->badgeTuneExisting->setVisible(m_show->hasTune());
    ui->badgeTuneMissing->setVisible(!m_show->hasTune());

    ui->certification->blockSignals(false);
    ui->rating->blockSignals(false);
    ui->userRating->blockSignals(false);
    ui->votes->blockSignals(false);
    ui->top250->blockSignals(false);
    ui->firstAired->blockSignals(false);
    ui->overview->blockSignals(false);
    ui->runtime->blockSignals(false);
    ui->comboStatus->blockSignals(false);
    ui->buttonRevert->setVisible(m_show->hasChanged());
}

void TvShowWidgetTvShow::updateImages(QVector<ImageType> images)
{
    for (const auto imageType : images) {
        ClosableImage* image = nullptr;

        for (ClosableImage* cImage : ui->artStackedWidget->findChildren<ClosableImage*>()) {
            if (cImage->imageType() == imageType) {
                image = cImage;
            }
        }

        if (image == nullptr) {
            continue;
        }

        if (!m_show->image(imageType).isNull()) {
            image->setImage(m_show->image(imageType));
        } else if (!m_show->imagesToRemove().contains(imageType)
                   && !Manager::instance()->mediaCenterInterface()->imageFileName(m_show, imageType).isEmpty()) {
            image->setImage(Manager::instance()->mediaCenterInterface()->imageFileName(m_show, imageType));
        }
    }
}

/**
 * \brief Saves the current show
 */
void TvShowWidgetTvShow::onSaveInformation()
{
    if (m_show == nullptr) {
        qDebug() << "My show is invalid";
        return;
    }

    onSetEnabled(false);
    m_savingWidget->show();
    m_show->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
    m_show->loadData(Manager::instance()->mediaCenterInterfaceTvShow(), true);
    updateTvShowInfo();
    m_savingWidget->hide();
    onSetEnabled(true);
    ui->buttonRevert->setVisible(false);
    NotificationBox::instance()->showSuccess(tr("<b>\"%1\"</b> Saved").arg(m_show->title()));
}

/**
 * \brief Reverts changes made to the current show
 */
void TvShowWidgetTvShow::onRevertChanges()
{
    m_show->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
    updateTvShowInfo();
}

void TvShowWidgetTvShow::onStartScraperSearch()
{
    if (m_show == nullptr) {
        qCritical() << "[TvShowWidgetTvShow] Cannot start show search without valid show! This must not happen!";
        return;
    }

    emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
    emit sigSetActionSearchEnabled(false, MainWidgets::TvShows);

    auto* searchWidget = new TvShowSearch(this);
    searchWidget->setSearchType(TvShowType::TvShow);
    searchWidget->execWithSearch(m_show->title());

    const int result = searchWidget->result();
    const mediaelch::scraper::ShowIdentifier identifier(searchWidget->showIdentifier());
    const auto updateType = searchWidget->updateType();
    const auto showInfosToLoad = searchWidget->showDetailsToLoad();
    const auto episodeInfosToLoad = searchWidget->episodeDetailsToLoad();
    const Locale locale = searchWidget->locale();
    const SeasonOrder seasonOrder = searchWidget->seasonOrder();
    scraper::TvScraper* scraper = searchWidget->scraper();

    searchWidget->deleteLater();

    if (result == QDialog::Accepted) {
        const int boxId = NotificationBox::instance()->addProgressBar(tr("Please wait while your TV show is scraped"));
        m_show->setProperty("progressBarId", boxId);
        onSetEnabled(false);
        connect(m_show.data(), &TvShow::sigLoaded, this, &TvShowWidgetTvShow::onInfoLoadDone, Qt::UniqueConnection);
        m_show->scrapeData(scraper, identifier, locale, seasonOrder, updateType, showInfosToLoad, episodeInfosToLoad);

    } else {
        emit sigSetActionSearchEnabled(true, MainWidgets::TvShows);
        emit sigSetActionSaveEnabled(true, MainWidgets::TvShows);
    }
}

void TvShowWidgetTvShow::onInfoLoadDone(TvShow* show, QSet<ShowScraperInfo> details, Locale locale)
{
    if (show->showMissingEpisodes()) {
        show->clearMissingEpisodes();
        show->fillMissingEpisodes();
    }

    QVector<ImageType> types{ImageType::TvShowClearArt,
        ImageType::TvShowLogos,
        ImageType::TvShowCharacterArt,
        ImageType::TvShowThumb,
        ImageType::TvShowSeasonThumb};

    if (show->tvdbId().isValid() && !types.isEmpty() && details.contains(ShowScraperInfo::ExtraArts)) {
        Manager::instance()->fanartTv()->tvShowImages(show, show->tvdbId(), types, locale);
        connect(Manager::instance()->fanartTv(),
            &mediaelch::scraper::ImageProvider::sigTvShowImagesLoaded,
            this,
            &TvShowWidgetTvShow::onLoadDone,
            Qt::UniqueConnection);
    } else {
        QMap<ImageType, QVector<Poster>> map;
        onLoadDone(show, map);
    }
    NotificationBox::instance()->hideProgressBar(show->property("progressBarId").toInt());
}

/**
 * \brief Called when the search widget finishes
 * Updates infos and starts downloads
 */
void TvShowWidgetTvShow::onLoadDone(TvShow* show, QMap<ImageType, QVector<Poster>> posters)
{
    if (m_show == nullptr) {
        qDebug() << "My show is invalid";
        return;
    }

    if (m_show == show) {
        updateTvShowInfo();
    } else {
        qDebug() << "Show has changed";
    }
    int downloadsSize = 0;
    if (!show->posters().isEmpty() && show->infosToLoad().contains(ShowScraperInfo::Poster)) {
        emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
        DownloadManagerElement d;
        d.imageType = ImageType::TvShowPoster;
        d.url = show->posters().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show) {
            ui->poster->setLoading(true);
        }
    }

    if (!show->backdrops().isEmpty() && show->infosToLoad().contains(ShowScraperInfo::Fanart)) {
        emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
        DownloadManagerElement d;
        d.imageType = ImageType::TvShowBackdrop;
        d.url = show->backdrops().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show) {
            ui->backdrop->setLoading(true);
        }
    }

    if (!show->banners().isEmpty() && show->infosToLoad().contains(ShowScraperInfo::Banner)) {
        emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
        DownloadManagerElement d;
        d.imageType = ImageType::TvShowBanner;
        d.url = show->banners().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show) {
            ui->banner->setLoading(true);
        }
    }

    QVector<SeasonNumber> thumbsForSeasons;
    QMapIterator<ImageType, QVector<Poster>> it(posters);
    while (it.hasNext()) {
        it.next();
        if (it.key() == ImageType::TvShowClearArt && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowClearArt;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show) {
                ui->clearArt->setLoading(true);
            }
            downloadsSize++;
        } else if (it.key() == ImageType::TvShowCharacterArt && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowCharacterArt;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show) {
                ui->characterArt->setLoading(true);
            }
            downloadsSize++;
        } else if (it.key() == ImageType::TvShowLogos && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowLogos;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show) {
                ui->logo->setLoading(true);
            }
            downloadsSize++;
        } else if (it.key() == ImageType::TvShowThumb && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowThumb;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show) {
                ui->thumb->setLoading(true);
            }
            downloadsSize++;
        } else if (it.key() == ImageType::TvShowSeasonThumb && !it.value().isEmpty()) {
            for (const Poster& p : it.value()) {
                if (thumbsForSeasons.contains(p.season)) {
                    continue;
                }
                if (!show->seasons().contains(p.season)) {
                    continue;
                }

                DownloadManagerElement d;
                d.imageType = ImageType::TvShowSeasonThumb;
                d.url = p.originalUrl;
                d.show = show;
                d.season = p.season;
                m_posterDownloadManager->addDownload(d);
                downloadsSize++;
                thumbsForSeasons.append(p.season);
            }
        }
    }

    if (show->infosToLoad().contains(ShowScraperInfo::Actors) && Settings::instance()->downloadActorImages()) {
        for (Actor* actor : show->actors()) {
            if (actor->thumb.isEmpty()) {
                continue;
            }
            DownloadManagerElement d;
            d.imageType = ImageType::Actor;
            d.url = QUrl(actor->thumb);
            d.actor = actor;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
        }
    }

    for (SeasonNumber season : show->seasons()) {
        if (!show->seasonPosters(season).isEmpty() && show->infosToLoad().contains(ShowScraperInfo::SeasonPoster)) {
            emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowSeasonPoster;
            d.url = show->seasonPosters(season).at(0).originalUrl;
            d.season = season;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
        }
        if (!show->seasonBackdrops(season).isEmpty() && show->infosToLoad().contains(ShowScraperInfo::SeasonBackdrop)) {
            emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowSeasonBackdrop;
            d.url = show->seasonBackdrops(season).at(0).originalUrl;
            d.season = season;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
        }
        if (!show->seasonBanners(season).isEmpty() && show->infosToLoad().contains(ShowScraperInfo::SeasonBanner)) {
            emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowSeasonBanner;
            d.url = show->seasonBanners(season).at(0).originalUrl;
            d.season = season;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
        }
    }

    if (show->episodeInfosToLoad().contains(EpisodeScraperInfo::Thumbnail)) {
        for (TvShowEpisode* episode : show->episodes()) {
            if (episode->thumbnail().isEmpty() || !episode->hasChanged()) {
                continue;
            }
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowEpisodeThumb;
            d.url = episode->thumbnail();
            d.episode = episode;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
        }
    }

    show->setDownloadsInProgress(downloadsSize > 0);

    if (downloadsSize > 0) {
        emit sigDownloadsStarted(tr("Downloading images..."), Constants::TvShowProgressMessageId + show->showId());
        connect(m_posterDownloadManager,
            &DownloadManager::allTvShowDownloadsFinished,
            this,
            &TvShowWidgetTvShow::onDownloadsFinished,
            static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    } else if (show == m_show) {
        onSetEnabled(true);
        emit sigSetActionSearchEnabled(true, MainWidgets::TvShows);
        emit sigSetActionSaveEnabled(true, MainWidgets::TvShows);
    }
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Adjusts the size of the backdrop to common values (1080p or 720p) and shows the image
 * \param elem Downloaded element
 */
void TvShowWidgetTvShow::onPosterDownloadFinished(DownloadManagerElement elem)
{
    if (TvShow::seasonImageTypes().contains(elem.imageType)) {
        if (elem.imageType == ImageType::TvShowSeasonBackdrop) {
            helper::resizeBackdrop(elem.data);
        }
        ImageCache::instance()->invalidateImages(
            Manager::instance()->mediaCenterInterface()->imageFileName(elem.show, elem.imageType, elem.season));
        elem.show->setSeasonImage(elem.season, elem.imageType, elem.data);
    } else if (elem.imageType == ImageType::TvShowExtraFanart) {
        helper::resizeBackdrop(elem.data);
        elem.show->addExtraFanart(elem.data);
        if (elem.show == m_show) {
            ui->fanarts->addImage(elem.data);
        }
    } else {
        for (ClosableImage* image : ui->artStackedWidget->findChildren<ClosableImage*>()) {
            if (image->imageType() == elem.imageType) {
                if (elem.imageType == ImageType::TvShowBackdrop) {
                    helper::resizeBackdrop(elem.data);
                }
                if (m_show == elem.show) {
                    image->setImage(elem.data);
                }
                ImageCache::instance()->invalidateImages(
                    Manager::instance()->mediaCenterInterface()->imageFileName(elem.show, elem.imageType));
                elem.show->setImage(elem.imageType, elem.data);
                break;
            }
        }
    }

    if (m_posterDownloadManager->downloadsLeftForShow(m_show) == 0) {
        ui->fanarts->setLoading(false);
        emit sigSetActionSaveEnabled(true, MainWidgets::TvShows);
    }
}

/**
 * \brief Toggles the state of save and search buttons when downloads have finished
 * \param show Show for the download has finished
 */
void TvShowWidgetTvShow::onDownloadsFinished(TvShow* show)
{
    if (show == nullptr) {
        qCritical() << "[TvShowWidgetTvShow]";
        return;
    }
    qDebug() << "Downloads finished for show:" << show->title();
    emit sigDownloadsFinished(Constants::TvShowProgressMessageId + show->showId());
    if (show == m_show) {
        onSetEnabled(true);
        emit sigSetActionSaveEnabled(true, MainWidgets::TvShows);
        emit sigSetActionSearchEnabled(true, MainWidgets::TvShows);
    }
    show->setDownloadsInProgress(false);
}

/**
 * \brief Emits the progress signal
 * \param left Number of downloads left
 * \param elem Current downloaded element
 */
void TvShowWidgetTvShow::onDownloadsLeft(int left, DownloadManagerElement elem)
{
    emit sigDownloadsProgress(elem.show->actors().size() + elem.show->episodes().size() - left,
        elem.show->actors().size() + elem.show->episodes().size(),
        Constants::TvShowProgressMessageId + elem.show->showId());
}

/*** add/remove/edit Actors, Genres, Countries and Studios ***/

/**
 * \brief Adds a genre
 */
void TvShowWidgetTvShow::onAddGenre(QString genre)
{
    if (m_show == nullptr) {
        return;
    }
    m_show->addGenre(genre);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Removes a genre
 */
void TvShowWidgetTvShow::onRemoveGenre(QString genre)
{
    if (m_show == nullptr) {
        return;
    }
    m_show->removeGenre(genre);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onAddTag(QString tag)
{
    if (m_show == nullptr) {
        return;
    }
    m_show->addTag(tag);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onRemoveTag(QString tag)
{
    if (m_show == nullptr) {
        return;
    }
    m_show->removeTag(tag);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Stores changed values for actors
 * \param item Edited item
 */
void TvShowWidgetTvShow::onActorEdited(QTableWidgetItem* item)
{
    auto* actor = ui->actors->item(item->row(), 1)->data(Qt::UserRole).value<Actor*>();
    if (item->column() == 0) {
        actor->name = item->text();
    } else if (item->column() == 1) {
        actor->role = item->text();
    }
    m_show->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Adds an actor
 */
void TvShowWidgetTvShow::onAddActor()
{
    Actor a;
    a.name = tr("Unknown Actor");
    a.role = tr("Unknown Role");
    m_show->addActor(a);

    Actor* actor = m_show->actors().back();

    ui->actors->blockSignals(true);
    int row = ui->actors->rowCount();
    ui->actors->insertRow(row);
    ui->actors->setItem(row, 0, new QTableWidgetItem(actor->name));
    ui->actors->setItem(row, 1, new QTableWidgetItem(actor->role));
    ui->actors->item(row, 1)->setData(Qt::UserRole, QVariant::fromValue(actor));
    ui->actors->scrollToBottom();
    ui->actors->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Removes an actor
 */
void TvShowWidgetTvShow::onRemoveActor()
{
    int row = ui->actors->currentRow();
    if (row < 0 || row >= ui->actors->rowCount() || !ui->actors->currentItem()->isSelected()) {
        return;
    }

    auto* actor = ui->actors->item(row, 1)->data(Qt::UserRole).value<Actor*>();
    m_show->removeActor(actor);
    ui->actors->blockSignals(true);
    ui->actors->removeRow(row);
    ui->actors->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Shows the image of the selected actor
 */
void TvShowWidgetTvShow::onActorChanged()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount()
        || ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        QPixmap pixmap(":/img/man.png");
        helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
        ui->actor->setPixmap(pixmap);
        ui->actorResolution->setText("");
        return;
    }

    auto* actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
    if (!actor->image.isNull()) {
        QImage img = QImage::fromData(actor->image);
        ui->actorResolution->setText(QString("%1 x %2").arg(img.width()).arg(img.height()));
        QPixmap pixmap = QPixmap::fromImage(img).scaled(
            QSize(120, 180) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
        ui->actor->setPixmap(pixmap);
    } else if (!Manager::instance()->mediaCenterInterface()->actorImageName(m_show, *actor).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->actorImageName(m_show, *actor));
        ui->actorResolution->setText(QString("%1 x %2").arg(p.width()).arg(p.height()));
        p = p.scaled(QSize(120, 180) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        helper::setDevicePixelRatio(p, helper::devicePixelRatio(this));
        ui->actor->setPixmap(p);
    } else {
        QPixmap pixmap(":/img/man.png");
        helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
        ui->actor->setPixmap(pixmap);
        ui->actorResolution->setText("");
    }
}

/**
 * \brief Stores the changed actor image
 */
void TvShowWidgetTvShow::onChangeActorImage()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount()
        || ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        return;
    }

    QString fileName =
        QFileDialog::getOpenFileName(parentWidget(), tr("Choose Image"), QDir::homePath(), tr("Images (*.jpg *.jpeg)"));
    if (!fileName.isNull()) {
        QImage img(fileName);
        if (!img.isNull()) {
            QByteArray ba;
            QBuffer buffer(&ba);
            img.save(&buffer, "jpg", 100);
            auto* actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
            actor->image = ba;
            actor->imageHasChanged = true;
            onActorChanged();
            m_show->setChanged(true);
        }
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * \brief Shows the first page with movie art
 */
void TvShowWidgetTvShow::onArtPageOne()
{
    ui->artStackedWidget->slideInIdx(0);
    ui->buttonArtPageTwo->setChecked(false);
    ui->buttonArtPageOne->setChecked(true);
}

/**
 * \brief Shows the second page with movie art
 */
void TvShowWidgetTvShow::onArtPageTwo()
{
    ui->artStackedWidget->slideInIdx(1);
    ui->buttonArtPageOne->setChecked(false);
    ui->buttonArtPageTwo->setChecked(true);
}

/*** Pass GUI events to TV show object ***/

/**
 * \brief Marks the show as changed when the name has changed
 */
void TvShowWidgetTvShow::onTitleChange(QString text)
{
    m_show->setTitle(std::move(text));
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onImdbIdChange(QString text)
{
    m_show->setImdbId(ImdbId(std::move(text)));
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onTmdbIdChange(QString text)
{
    m_show->setTmdbId(TmdbId(std::move(text)));
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onTvdbIdChange(QString text)
{
    m_show->setTvdbId(TvDbId(std::move(text)));
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onTvMazeIdChange(QString text)
{
    m_show->setTvMazeId(TvMazeId(std::move(text)));
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onSortTitleChange(QString text)
{
    m_show->setSortTitle(std::move(text));
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onOriginalTitleChange(QString text)
{
    m_show->setOriginalTitle(std::move(text));
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the show as changed when the certification has changed
 */
void TvShowWidgetTvShow::onCertificationChange(QString text)
{
    m_show->setCertification(Certification(std::move(text)));
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the show as changed when the rating has changed
 */
void TvShowWidgetTvShow::onRatingChange(double value)
{
    if ((m_show == nullptr) || m_show->ratings().isEmpty()) {
        return;
    }
    m_show->ratings().back().rating = value;
    m_show->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onUserRatingChange(double value)
{
    if (m_show == nullptr) {
        return;
    }
    m_show->setUserRating(value);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onRuntimeChange(int runtime)
{
    m_show->setRuntime(std::chrono::minutes(runtime));
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the show as changed when the first aired date has changed
 */
void TvShowWidgetTvShow::onFirstAiredChange(QDate date)
{
    m_show->setFirstAired(date);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the show as changed when the studio has changed
 */
void TvShowWidgetTvShow::onStudioChange(QString studio)
{
    m_show->setNetwork(std::move(studio));
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the show as changed when the overview has changed
 */
void TvShowWidgetTvShow::onOverviewChange()
{
    m_show->setOverview(ui->overview->toPlainText());
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onRemoveExtraFanart(QByteArray image)
{
    if (m_show == nullptr) {
        return;
    }
    m_show->removeExtraFanart(image);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onRemoveExtraFanart(QString file)
{
    if (m_show == nullptr) {
        return;
    }
    m_show->removeExtraFanart(file);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onAddExtraFanart()
{
    if (m_show == nullptr) {
        return;
    }

    auto* imageDialog = new ImageDialog(this);
    imageDialog->setImageType(ImageType::TvShowExtraFanart);
    imageDialog->setMultiSelection(true);
    imageDialog->setTvShow(m_show);
    imageDialog->setDefaultDownloads(m_show->backdrops());

    imageDialog->execWithType(ImageType::TvShowExtraFanart);
    const int exitCode = imageDialog->result();
    const QVector<QUrl> imageUrls = imageDialog->imageUrls();
    imageDialog->deleteLater();

    if (exitCode == QDialog::Accepted && !imageUrls.isEmpty()) {
        ui->fanarts->setLoading(true);
        emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
        for (const QUrl& url : imageUrls) {
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowExtraFanart;
            d.url = url;
            d.show = m_show;
            m_posterDownloadManager->addDownload(d);
        }
        ui->buttonRevert->setVisible(true);
    }
}

void TvShowWidgetTvShow::onExtraFanartDropped(QUrl imageUrl)
{
    if (m_show == nullptr) {
        return;
    }
    emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
    DownloadManagerElement d;
    d.imageType = ImageType::TvShowExtraFanart;
    d.url = std::move(imageUrl);
    d.show = m_show;
    m_posterDownloadManager->addDownload(d);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onDownloadTune()
{
    if (m_show == nullptr) {
        qCritical() << "[TvShowWidgetTvShow] Show is undefined, cannot download TV tunes!";
        return;
    }
    auto* tvTunesDialog = new TvTunesDialog(*m_show, this);
    tvTunesDialog->setAttribute(Qt::WA_DeleteOnClose);
    const int result = tvTunesDialog->exec();
    if (result == QDialog::Accepted) {
        ui->badgeTuneExisting->setVisible(true);
        ui->badgeTuneMissing->setVisible(false);
    }
}

void TvShowWidgetTvShow::onChooseImage()
{
    if (m_show == nullptr) {
        return;
    }

    auto* image = dynamic_cast<ClosableImage*>(QObject::sender());
    if (image == nullptr) {
        return;
    }

    auto* imageDialog = new ImageDialog(this);
    imageDialog->setImageType(image->imageType());
    imageDialog->setTvShow(m_show);
    switch (image->imageType()) {
    case ImageType::TvShowPoster: imageDialog->setDefaultDownloads(m_show->posters()); break;
    case ImageType::TvShowBackdrop: imageDialog->setDefaultDownloads(m_show->backdrops()); break;
    case ImageType::TvShowBanner: imageDialog->setDefaultDownloads(m_show->banners()); break;
    default: break;
    }

    imageDialog->execWithType(image->imageType());
    const int exitCode = imageDialog->result();
    const QUrl imageUrl = imageDialog->imageUrl();
    imageDialog->deleteLater();

    if (exitCode == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
        DownloadManagerElement d;
        d.imageType = image->imageType();
        d.url = imageUrl;
        d.show = m_show;
        m_posterDownloadManager->addDownload(d);
        image->setLoading(true);
        ui->buttonRevert->setVisible(true);
    }
}

void TvShowWidgetTvShow::onDeleteImage()
{
    if (m_show == nullptr) {
        return;
    }

    auto* image = dynamic_cast<ClosableImage*>(QObject::sender());
    if (image == nullptr) {
        return;
    }

    m_show->removeImage(image->imageType());
    updateImages({image->imageType()});
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onImageDropped(ImageType imageType, QUrl imageUrl)
{
    if (m_show == nullptr) {
        return;
    }
    auto* image = dynamic_cast<ClosableImage*>(QObject::sender());
    if (image == nullptr) {
        return;
    }

    emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
    DownloadManagerElement d;
    d.imageType = imageType;
    d.url = std::move(imageUrl);
    d.show = m_show;
    m_posterDownloadManager->addDownload(d);
    image->setLoading(true);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onVotesChange(int value)
{
    if ((m_show == nullptr) || m_show->ratings().isEmpty()) {
        return;
    }
    m_show->ratings().back().voteCount = value;
    ui->buttonRevert->setVisible(true);
    m_show->setChanged(true);
}

void TvShowWidgetTvShow::onTop250Change(int value)
{
    if (m_show == nullptr) {
        return;
    }
    m_show->setTop250(value);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onShowScraperProgress(TvShow* show, int current, int max)
{
    if (!show->property("progressBarId").isValid()) {
        return;
    }
    int id = show->property("progressBarId").toInt();
    NotificationBox::instance()->progressBarProgress(current, max, id);
}

void TvShowWidgetTvShow::onStatusChange(int index)
{
    if (m_show == nullptr) {
        return;
    }
    m_show->setStatus(ui->comboStatus->itemData(index).toString());
    ui->buttonRevert->setVisible(true);
}
