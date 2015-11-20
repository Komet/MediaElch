#include "TvShowWidgetTvShow.h"
#include "ui_TvShowWidgetTvShow.h"

#include <QFileDialog>
#include <QMovie>
#include <QPainter>
#include "data/ImageCache.h"
#include "globals/ComboDelegate.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "notifications/NotificationBox.h"
#include "tvShows/TvShowSearch.h"
#include "tvShows/TvTunesDialog.h"

/**
 * @brief TvShowWidgetTvShow::TvShowWidgetTvShow
 * @param parent
 */
TvShowWidgetTvShow::TvShowWidgetTvShow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TvShowWidgetTvShow)
{
    ui->setupUi(this);

    m_show = 0;

    ui->showTitle->clear();
    ui->actors->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->artStackedWidget->setAnimation(QEasingCurve::OutCubic);
    ui->artStackedWidget->setSpeed(300);

    QFont font = ui->labelClearArt->font();
    #ifdef Q_OS_WIN32
        font.setPointSize(font.pointSize()-1);
    #else
        font.setPointSize(font.pointSize()-2);
    #endif

#ifndef Q_OS_MAC
    QFont nameFont = ui->showTitle->font();
    nameFont.setPointSize(nameFont.pointSize()-4);
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
    #ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->actorResolution->setFont(font);

    ui->badgeTuneExisting->setBadgeType(Badge::LabelSuccess);
    ui->badgeTuneMissing->setBadgeType(Badge::LabelWarning);

    ui->btnDownloadTune->setIcon(Manager::instance()->iconFont()->icon("download", QColor(150, 150, 150), "", -1, 1.0));

    ui->genreCloud->setText(tr("Genres"));
    ui->genreCloud->setPlaceholder(tr("Add Genre"));
    connect(ui->genreCloud, SIGNAL(activated(QString)), this, SLOT(onAddGenre(QString)));
    connect(ui->genreCloud, SIGNAL(deactivated(QString)), this, SLOT(onRemoveGenre(QString)));

    ui->tagCloud->setText(tr("Tags"));
    ui->tagCloud->setPlaceholder(tr("Add Tag"));
    connect(ui->tagCloud, SIGNAL(activated(QString)), this, SLOT(onAddTag(QString)));
    connect(ui->tagCloud, SIGNAL(deactivated(QString)), this, SLOT(onRemoveTag(QString)));

    ui->comboStatus->setItemData(0, "");
    ui->comboStatus->setItemData(1, "Continuing");
    ui->comboStatus->setItemData(2, "Ended");

    m_loadingMovie = new QMovie(":/img/spinner.gif");
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
    foreach (ClosableImage *image, ui->artStackedWidget->findChildren<ClosableImage*>()) {
        connect(image, &ClosableImage::clicked, this, &TvShowWidgetTvShow::onChooseImage);
        connect(image, &ClosableImage::sigClose, this, &TvShowWidgetTvShow::onDeleteImage);
        connect(image, &ClosableImage::sigImageDropped, this, &TvShowWidgetTvShow::onImageDropped);
    }

    QPixmap pixmap(":/img/man.png");
    Helper::instance()->setDevicePixelRatio(pixmap, Helper::instance()->devicePixelRatio(this));
    ui->actor->setPixmap(pixmap);

    connect(ui->name, SIGNAL(textChanged(QString)), ui->showTitle, SLOT(setText(QString)));
    connect(ui->buttonAddActor, SIGNAL(clicked()), this, SLOT(onAddActor()));
    connect(ui->buttonRemoveActor, SIGNAL(clicked()), this, SLOT(onRemoveActor()));
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onPosterDownloadFinished(DownloadManagerElement)));
    connect(m_posterDownloadManager, SIGNAL(downloadsLeft(int,DownloadManagerElement)), this, SLOT(onDownloadsLeft(int,DownloadManagerElement)));
    connect(ui->actors, SIGNAL(itemSelectionChanged()), this, SLOT(onActorChanged()));
    connect(ui->actor, SIGNAL(clicked()), this, SLOT(onChangeActorImage()));
    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));
    connect(ui->btnDownloadTune, SIGNAL(clicked()), this, SLOT(onDownloadTune()));

    connect(ui->fanarts, SIGNAL(sigRemoveImage(QByteArray)), this, SLOT(onRemoveExtraFanart(QByteArray)));
    connect(ui->fanarts, SIGNAL(sigRemoveImage(QString)), this, SLOT(onRemoveExtraFanart(QString)));
    connect(ui->btnAddExtraFanart, SIGNAL(clicked()), this, SLOT(onAddExtraFanart()));
    connect(ui->fanarts, &ImageGallery::sigImageDropped, this, &TvShowWidgetTvShow::onExtraFanartDropped);

    onClear();

    // Connect GUI change events to movie object
    connect(ui->name, SIGNAL(textEdited(QString)), this, SLOT(onNameChange(QString)));
    connect(ui->imdbId, SIGNAL(textEdited(QString)), this, SLOT(onImdbIdChange(QString)));
    connect(ui->tvdbId, SIGNAL(textEdited(QString)), this, SLOT(onTvdbIdChange(QString)));
    connect(ui->sortTitle, SIGNAL(textEdited(QString)), this, SLOT(onSortTitleChange(QString)));
    connect(ui->certification, SIGNAL(editTextChanged(QString)), this, SLOT(onCertificationChange(QString)));
    connect(ui->rating, SIGNAL(valueChanged(double)), this, SLOT(onRatingChange(double)));
    connect(ui->votes, SIGNAL(valueChanged(int)), this, SLOT(onVotesChange(int)));
    connect(ui->top250, SIGNAL(valueChanged(int)), this, SLOT(onTop250Change(int)));
    connect(ui->firstAired, SIGNAL(dateChanged(QDate)), this, SLOT(onFirstAiredChange(QDate)));
    connect(ui->studio, SIGNAL(textEdited(QString)), this, SLOT(onStudioChange(QString)));
    connect(ui->overview, SIGNAL(textChanged()), this, SLOT(onOverviewChange()));
    connect(ui->actors, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onActorEdited(QTableWidgetItem*)));
    connect(ui->runtime, SIGNAL(valueChanged(int)), this, SLOT(onRuntimeChange(int)));
    connect(ui->comboStatus, SIGNAL(currentIndexChanged(int)), this, SLOT(onStatusChange(int)));

    onSetEnabled(false);

    connect(static_cast<TheTvDb*>(Manager::instance()->tvScrapers().at(0)), SIGNAL(sigLoadProgress(TvShow*,int,int)), this, SLOT(onShowScraperProgress(TvShow*,int,int)));

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
}

/**
 * @brief TvShowWidgetTvShow::~TvShowWidgetTvShow
 */
TvShowWidgetTvShow::~TvShowWidgetTvShow()
{
    delete ui;
}

/**
 * @brief Repositions the saving widget
 * @param event
 */
void TvShowWidgetTvShow::resizeEvent(QResizeEvent *event)
{
    m_savingWidget->move(size().width()/2-m_savingWidget->width(), height()/2-m_savingWidget->height());
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
 * @brief Clears all contents of the widget
 */
void TvShowWidgetTvShow::onClear()
{
    qDebug() << "Entered";

    bool blocked;

    blocked = ui->certification->blockSignals(true);
    ui->certification->clear();
    ui->certification->blockSignals(blocked);

    blocked = ui->rating->blockSignals(true);
    ui->rating->clear();
    ui->rating->blockSignals(blocked);

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
    ui->tvdbId->clear();
    ui->actors->setRowCount(0);
    ui->dir->clear();
    ui->name->clear();
    ui->sortTitle->clear();
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
 * @brief Toggles the enabled state of the main groupbox
 * @param enabled Status
 */
void TvShowWidgetTvShow::onSetEnabled(bool enabled)
{
    qDebug() << "Entered, enabled=" << enabled;
    ui->groupBox_3->setEnabled(enabled);
}

/**
 * @brief Sets the current show and updates widgets contents
 * @param show Show object
 */
void TvShowWidgetTvShow::setTvShow(TvShow *show)
{
    qDebug() << "Entered, show=" << show->name();
    show->loadData(Manager::instance()->mediaCenterInterface());
    m_show = show;
    updateTvShowInfo();
    if (show->downloadsInProgress()) {
        onSetEnabled(false);
        emit sigSetActionSearchEnabled(false, WidgetTvShows);
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
    } else {
        onSetEnabled(true);
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
    }
}

/**
 * @brief Updates the widgets contents
 */
void TvShowWidgetTvShow::updateTvShowInfo()
{
    qDebug() << "Entered";
    if (m_show == 0) {
        qDebug() << "My show is invalid";
        return;
    }

    ui->certification->blockSignals(true);
    ui->rating->blockSignals(true);
    ui->votes->blockSignals(true);
    ui->top250->blockSignals(true);
    ui->firstAired->blockSignals(true);
    ui->overview->blockSignals(true);
    ui->runtime->blockSignals(true);
    ui->comboStatus->blockSignals(true);

    onClear();

    ui->dir->setText(m_show->dir());
    ui->name->setText(m_show->name());
    ui->imdbId->setText(m_show->imdbId());
    ui->tvdbId->setText(m_show->tvdbId());
    ui->sortTitle->setText(m_show->sortTitle());
    ui->rating->setValue(m_show->rating());
    ui->votes->setValue(m_show->votes());
    ui->top250->setValue(m_show->top250());
    ui->firstAired->setDate(m_show->firstAired());
    ui->studio->setText(m_show->network());
    ui->overview->setPlainText(m_show->overview());
    ui->runtime->setValue(m_show->runtime());
    if (m_show->status() == "Continuing")
        ui->comboStatus->setCurrentIndex(1);
    else if (m_show->status() == "Ended")
        ui->comboStatus->setCurrentIndex(2);
    else
        ui->comboStatus->setCurrentIndex(0);

    ui->actors->blockSignals(true);
    foreach (Actor *actor, m_show->actorsPointer()) {
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
    foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows()) {
        genres.append(show->genres());
        tags.append(show->tags());
    }
    ui->genreCloud->setTags(genres, m_show->genres());
    ui->tagCloud->setTags(tags, m_show->tags());

    QStringList certifications = m_show->certifications();
    certifications.prepend("");
    ui->certification->addItems(certifications);
    ui->certification->setCurrentIndex(certifications.indexOf(m_show->certification()));

    updateImages(QList<int>() << ImageType::TvShowPoster << ImageType::TvShowBackdrop << ImageType::TvShowBanner << ImageType::TvShowCharacterArt << ImageType::TvShowClearArt << ImageType::TvShowLogos << ImageType::TvShowThumb);
    ui->fanarts->setImages(m_show->extraFanarts(Manager::instance()->mediaCenterInterfaceTvShow()));

    ui->badgeTuneExisting->setVisible(m_show->hasTune());
    ui->badgeTuneMissing->setVisible(!m_show->hasTune());

    ui->certification->blockSignals(false);
    ui->rating->blockSignals(false);
    ui->votes->blockSignals(false);
    ui->top250->blockSignals(false);
    ui->firstAired->blockSignals(false);
    ui->overview->blockSignals(false);
    ui->runtime->blockSignals(false);
    ui->comboStatus->blockSignals(false);
    ui->buttonRevert->setVisible(m_show->hasChanged());
}

void TvShowWidgetTvShow::updateImages(QList<int> images)
{
    foreach (const int &imageType, images) {
        ClosableImage *image = 0;

        foreach (ClosableImage *cImage, ui->artStackedWidget->findChildren<ClosableImage*>()) {
            if (cImage->imageType() == imageType)
                image = cImage;
        }

        if (!image)
            continue;

        if (!m_show->image(imageType).isNull())
            image->setImage(m_show->image(imageType));
        else if (!m_show->imagesToRemove().contains(imageType) && !Manager::instance()->mediaCenterInterface()->imageFileName(m_show, imageType).isEmpty())
            image->setImage(Manager::instance()->mediaCenterInterface()->imageFileName(m_show, imageType));
    }
}

/**
 * @brief Saves the current show
 */
void TvShowWidgetTvShow::onSaveInformation()
{
    qDebug() << "Entered";
    if (m_show == 0) {
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
    NotificationBox::instance()->showMessage(tr("<b>\"%1\"</b> Saved").arg(m_show->name()));
}

/**
 * @brief Reverts changes made to the current show
 */
void TvShowWidgetTvShow::onRevertChanges()
{
    qDebug() << "Entered";
    m_show->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
    updateTvShowInfo();
}

/**
 * @brief Shows the search widget
 */
void TvShowWidgetTvShow::onStartScraperSearch()
{
    qDebug() << "Entered";
    if (m_show == 0) {
        qDebug() << "My show is invalid";
        return;
    }
    emit sigSetActionSaveEnabled(false, WidgetTvShows);
    emit sigSetActionSearchEnabled(false, WidgetTvShows);
    TvShowSearch::instance()->setSearchType(TypeTvShow);
    TvShowSearch::instance()->exec(m_show->name(), m_show->tvdbId());
    if (TvShowSearch::instance()->result() == QDialog::Accepted) {
        int id = NotificationBox::instance()->addProgressBar(tr("Please wait while your tv show is scraped"));
        m_show->setProperty("progressBarId", id);
        onSetEnabled(false);
        m_show->loadData(TvShowSearch::instance()->scraperId(), Manager::instance()->tvScrapers().at(0), TvShowSearch::instance()->updateType(), TvShowSearch::instance()->infosToLoad());
        connect(m_show, SIGNAL(sigLoaded(TvShow*)), this, SLOT(onInfoLoadDone(TvShow*)), Qt::UniqueConnection);
    } else {
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
    }
}

/**
 * @brief TvShowWidgetTvShow::onInfoLoadDone
 * @param show
 */
void TvShowWidgetTvShow::onInfoLoadDone(TvShow *show)
{
    if (show->showMissingEpisodes()) {
        show->clearMissingEpisodes();
        show->fillMissingEpisodes();
    }
    QList<int> types;
    types << ImageType::TvShowClearArt << ImageType::TvShowLogos << ImageType::TvShowCharacterArt << ImageType::TvShowThumb << ImageType::TvShowSeasonThumb;
    if (!show->tvdbId().isEmpty() && !types.isEmpty() && show->infosToLoad().contains(TvShowScraperInfos::ExtraArts)) {
        Manager::instance()->fanartTv()->tvShowImages(show, show->tvdbId(), types);
        connect(Manager::instance()->fanartTv(), SIGNAL(sigImagesLoaded(TvShow*,QMap<int,QList<Poster> >)), this, SLOT(onLoadDone(TvShow*,QMap<int,QList<Poster> >)), Qt::UniqueConnection);
    } else {
        QMap<int, QList<Poster> > map;
        onLoadDone(show, map);
    }
    NotificationBox::instance()->hideProgressBar(show->property("progressBarId").toInt());
}

/**
 * @brief Called when the search widget finishes
 * Updates infos and starts downloads
 * @param show Tv Show
 * @param posters
 */
void TvShowWidgetTvShow::onLoadDone(TvShow *show, QMap<int, QList<Poster> > posters)
{
    qDebug() << "Entered";
    if (m_show == 0) {
        qDebug() << "My show is invalid";
        return;
    }

    if (m_show == show)
        updateTvShowInfo();
    else
        qDebug() << "Show has changed";

    int downloadsSize = 0;
    if (show->posters().size() > 0 && show->infosToLoad().contains(TvShowScraperInfos::Poster)) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = ImageType::TvShowPoster;
        d.url = show->posters().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show)
            ui->poster->setLoading(true);
    }

    if (show->backdrops().size() > 0 && show->infosToLoad().contains(TvShowScraperInfos::Fanart)) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = ImageType::TvShowBackdrop;
        d.url = show->backdrops().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show)
            ui->backdrop->setLoading(true);
    }

    if (show->banners().size() > 0 && show->infosToLoad().contains(TvShowScraperInfos::Banner)) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = ImageType::TvShowBanner;
        d.url = show->banners().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show)
            ui->banner->setLoading(true);
    }

    QList<int> thumbsForSeasons;
    QMapIterator<int, QList<Poster> > it(posters);
    while (it.hasNext()) {
        it.next();
        if (it.key() == ImageType::TvShowClearArt && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowClearArt;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show)
                ui->clearArt->setLoading(true);
            downloadsSize++;
        } else if (it.key() == ImageType::TvShowCharacterArt && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowCharacterArt;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show)
                ui->characterArt->setLoading(true);
            downloadsSize++;
        } else if (it.key() == ImageType::TvShowLogos && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowLogos;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show)
                ui->logo->setLoading(true);
            downloadsSize++;
        } else if (it.key() == ImageType::TvShowThumb && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowThumb;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show)
                ui->thumb->setLoading(true);
            downloadsSize++;
        } else if (it.key() == ImageType::TvShowSeasonThumb && !it.value().isEmpty()) {
            foreach (Poster p, it.value()) {
                if (thumbsForSeasons.contains(p.season))
                    continue;
                if (!show->seasons().contains(p.season))
                    continue;

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

    if (show->infosToLoad().contains(TvShowScraperInfos::Actors) && Settings::instance()->downloadActorImages()) {
        QList<Actor*> actors = show->actorsPointer();
        for (int i=0, n=actors.size() ; i<n ; ++i) {
            if (actors.at(i)->thumb.isEmpty())
                continue;
            DownloadManagerElement d;
            d.imageType = ImageType::Actor;
            d.url = QUrl(actors.at(i)->thumb);
            d.actor = actors.at(i);
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
        }
    }

    foreach (int season, show->seasons()) {
        if (!show->seasonPosters(season).isEmpty() && show->infosToLoad().contains(TvShowScraperInfos::SeasonPoster)) {
            emit sigSetActionSaveEnabled(false, WidgetTvShows);
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowSeasonPoster;
            d.url = show->seasonPosters(season).at(0).originalUrl;
            d.season = season;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
        }
        if (!show->seasonBackdrops(season).isEmpty() && show->infosToLoad().contains(TvShowScraperInfos::SeasonBackdrop)) {
            emit sigSetActionSaveEnabled(false, WidgetTvShows);
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowSeasonBackdrop;
            d.url = show->seasonBackdrops(season).at(0).originalUrl;
            d.season = season;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
        }
        if (!show->seasonBanners(season).isEmpty() && show->infosToLoad().contains(TvShowScraperInfos::SeasonBanner)) {
            emit sigSetActionSaveEnabled(false, WidgetTvShows);
            DownloadManagerElement d;
            d.imageType = ImageType::TvShowSeasonBanner;
            d.url = show->seasonBanners(season).at(0).originalUrl;
            d.season = season;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
        }
    }

    if (show->infosToLoad().contains(TvShowScraperInfos::Thumbnail)) {
        foreach (TvShowEpisode *episode, show->episodes()) {
            if (episode->thumbnail().isEmpty() || !episode->hasChanged())
                continue;
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
        emit sigDownloadsStarted(tr("Downloading images..."), Constants::TvShowProgressMessageId+show->showId());
        connect(m_posterDownloadManager, SIGNAL(allDownloadsFinished(TvShow*)), this, SLOT(onDownloadsFinished(TvShow*)), Qt::UniqueConnection);
    } else if (show == m_show) {
        onSetEnabled(true);
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
    }
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Adjusts the size of the backdrop to common values (1080p or 720p) and shows the image
 * @param elem Downloaded element
 */
void TvShowWidgetTvShow::onPosterDownloadFinished(DownloadManagerElement elem)
{
    qDebug() << "Entered";

    if (TvShow::seasonImageTypes().contains(elem.imageType)) {
        if (elem.imageType == ImageType::TvShowSeasonBackdrop)
            Helper::instance()->resizeBackdrop(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->imageFileName(elem.show, elem.imageType, elem.season));
        elem.show->setSeasonImage(elem.season, elem.imageType, elem.data);
    } else if (elem.imageType == ImageType::TvShowExtraFanart) {
        Helper::instance()->resizeBackdrop(elem.data);
        elem.show->addExtraFanart(elem.data);
        if (elem.show == m_show)
            ui->fanarts->addImage(elem.data);
    } else {
        foreach (ClosableImage *image, ui->artStackedWidget->findChildren<ClosableImage*>()) {
            if (image->imageType() == elem.imageType) {
                if (elem.imageType == ImageType::TvShowBackdrop)
                    Helper::instance()->resizeBackdrop(elem.data);
                if (m_show == elem.show)
                    image->setImage(elem.data);
                ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->imageFileName(elem.show, elem.imageType));
                elem.show->setImage(elem.imageType, elem.data);
                break;
            }
        }
    }

    if (m_posterDownloadManager->downloadsLeftForShow(m_show) == 0) {
        ui->fanarts->setLoading(false);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
    }
}

/**
 * @brief Toggles the state of save and search buttons when downloads have finished
 * @param show Show for the download has finished
 */
void TvShowWidgetTvShow::onDownloadsFinished(TvShow *show)
{
    qDebug() << "Entered, show=" << show->name();
    emit sigDownloadsFinished(Constants::TvShowProgressMessageId+show->showId());
    if (show == m_show) {
        onSetEnabled(true);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
    }
    show->setDownloadsInProgress(false);
}

/**
 * @brief Emits the progress signal
 * @param left Number of downloads left
 * @param elem Current downloaded element
 */
void TvShowWidgetTvShow::onDownloadsLeft(int left, DownloadManagerElement elem)
{
    emit sigDownloadsProgress(elem.show->actors().size()+elem.show->episodes().size()-left, elem.show->actors().size()+elem.show->episodes().size(),
                              Constants::TvShowProgressMessageId+elem.show->showId());
}

/*** add/remove/edit Actors, Genres, Countries and Studios ***/

/**
 * @brief Adds a genre
 */
void TvShowWidgetTvShow::onAddGenre(QString genre)
{
    if (!m_show)
        return;
    m_show->addGenre(genre);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a genre
 */
void TvShowWidgetTvShow::onRemoveGenre(QString genre)
{
    if (!m_show)
        return;
    m_show->removeGenre(genre);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onAddTag(QString tag)
{
    if (!m_show)
        return;
    m_show->addTag(tag);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onRemoveTag(QString tag)
{
    if (!m_show)
        return;
    m_show->removeTag(tag);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Stores changed values for actors
 * @param item Edited item
 */
void TvShowWidgetTvShow::onActorEdited(QTableWidgetItem *item)
{
    Actor *actor = ui->actors->item(item->row(), 1)->data(Qt::UserRole).value<Actor*>();
    if (item->column() == 0)
        actor->name = item->text();
    else if (item->column() == 1)
        actor->role = item->text();
    m_show->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Adds an actor
 */
void TvShowWidgetTvShow::onAddActor()
{
    Actor a;
    a.name = tr("Unknown Actor");
    a.role = tr("Unknown Role");
    m_show->addActor(a);

    Actor *actor = m_show->actorsPointer().last();

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
 * @brief Removes an actor
 */
void TvShowWidgetTvShow::onRemoveActor()
{
    int row = ui->actors->currentRow();
    if (row < 0 || row >= ui->actors->rowCount() || !ui->actors->currentItem()->isSelected())
        return;

    Actor *actor = ui->actors->item(row, 1)->data(Qt::UserRole).value<Actor*>();
    m_show->removeActor(actor);
    ui->actors->blockSignals(true);
    ui->actors->removeRow(row);
    ui->actors->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Shows the image of the selected actor
 */
void TvShowWidgetTvShow::onActorChanged()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount() ||
        ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        QPixmap pixmap(":/img/man.png");
        Helper::instance()->setDevicePixelRatio(pixmap, Helper::instance()->devicePixelRatio(this));
        ui->actor->setPixmap(pixmap);
        ui->actorResolution->setText("");
        return;
    }

    Actor *actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
    if (!actor->image.isNull()) {
        QImage img = QImage::fromData(actor->image);
        ui->actorResolution->setText(QString("%1 x %2").arg(img.width()).arg(img.height()));
        QPixmap pixmap = QPixmap::fromImage(img).scaled(QSize(120, 180) * Helper::instance()->devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        Helper::instance()->setDevicePixelRatio(pixmap, Helper::instance()->devicePixelRatio(this));
        ui->actor->setPixmap(pixmap);
    } else if (!Manager::instance()->mediaCenterInterface()->actorImageName(m_show, *actor).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->actorImageName(m_show, *actor));
        ui->actorResolution->setText(QString("%1 x %2").arg(p.width()).arg(p.height()));
        p = p.scaled(QSize(120, 180) * Helper::instance()->devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        Helper::instance()->setDevicePixelRatio(p, Helper::instance()->devicePixelRatio(this));
        ui->actor->setPixmap(p);
    } else {
        QPixmap pixmap(":/img/man.png");
        Helper::instance()->setDevicePixelRatio(pixmap, Helper::instance()->devicePixelRatio(this));
        ui->actor->setPixmap(pixmap);
        ui->actorResolution->setText("");
    }
}

/**
 * @brief Stores the changed actor image
 */
void TvShowWidgetTvShow::onChangeActorImage()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount() ||
        ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(parentWidget(), tr("Choose Image"), QDir::homePath(), tr("Images (*.jpg *.jpeg)"));
    if (!fileName.isNull()) {
        QImage img(fileName);
        if (!img.isNull()) {
            QByteArray ba;
            QBuffer buffer(&ba);
            img.save(&buffer, "jpg", 100);
            Actor *actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
            actor->image = ba;
            actor->imageHasChanged = true;
            onActorChanged();
            m_show->setChanged(true);
        }
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the first page with movie art
 */
void TvShowWidgetTvShow::onArtPageOne()
{
    ui->artStackedWidget->slideInIdx(0);
    ui->buttonArtPageTwo->setChecked(false);
    ui->buttonArtPageOne->setChecked(true);
}

/**
 * @brief Shows the second page with movie art
 */
void TvShowWidgetTvShow::onArtPageTwo()
{
    ui->artStackedWidget->slideInIdx(1);
    ui->buttonArtPageOne->setChecked(false);
    ui->buttonArtPageTwo->setChecked(true);
}

/*** Pass GUI events to tv show object ***/

/**
 * @brief Marks the show as changed when the name has changed
 */
void TvShowWidgetTvShow::onNameChange(QString text)
{
    m_show->setName(text);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onImdbIdChange(QString text)
{
    m_show->setImdbId(text);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onTvdbIdChange(QString text)
{
    m_show->setTvdbId(text);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onSortTitleChange(QString text)
{
    m_show->setSortTitle(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the show as changed when the certification has changed
 */
void TvShowWidgetTvShow::onCertificationChange(QString text)
{
    m_show->setCertification(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the show as changed when the rating has changed
 */
void TvShowWidgetTvShow::onRatingChange(double value)
{
    m_show->setRating(value);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onRuntimeChange(int runtime)
{
    m_show->setRuntime(runtime);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the show as changed when the first aired date has changed
 */
void TvShowWidgetTvShow::onFirstAiredChange(QDate date)
{
    m_show->setFirstAired(date);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the show as changed when the studio has changed
 */
void TvShowWidgetTvShow::onStudioChange(QString studio)
{
    m_show->setNetwork(studio);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the show as changed when the overview has changed
 */
void TvShowWidgetTvShow::onOverviewChange()
{
    m_show->setOverview(ui->overview->toPlainText());
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onRemoveExtraFanart(const QByteArray &image)
{
    if (!m_show)
        return;
    m_show->removeExtraFanart(image);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onRemoveExtraFanart(const QString &file)
{
    if (!m_show)
        return;
    m_show->removeExtraFanart(file);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onAddExtraFanart()
{
    if (!m_show)
        return;

    ImageDialog::instance()->setImageType(ImageType::TvShowExtraFanart);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMultiSelection(true);
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setDownloads(m_show->backdrops());
    ImageDialog::instance()->exec(ImageType::TvShowExtraFanart);

    if (ImageDialog::instance()->result() == QDialog::Accepted && !ImageDialog::instance()->imageUrls().isEmpty()) {
        ui->fanarts->setLoading(true);
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        foreach (const QUrl &url, ImageDialog::instance()->imageUrls()) {
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
    if (!m_show)
        return;
    emit sigSetActionSaveEnabled(false, WidgetTvShows);
    DownloadManagerElement d;
    d.imageType = ImageType::TvShowExtraFanart;
    d.url = imageUrl;
    d.show = m_show;
    m_posterDownloadManager->addDownload(d);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onDownloadTune()
{
    TvTunesDialog::instance()->setTvShow(m_show);
    int result = TvTunesDialog::instance()->exec();
    if (result == QDialog::Accepted) {
        ui->badgeTuneExisting->setVisible(true);
        ui->badgeTuneMissing->setVisible(false);
    }
}

void TvShowWidgetTvShow::onChooseImage()
{
    if (m_show == 0)
        return;

    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    ImageDialog::instance()->setImageType(image->imageType());
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShow(m_show);
    switch (image->imageType()) {
    case ImageType::TvShowPoster:
        ImageDialog::instance()->setDownloads(m_show->posters());
        break;
    case ImageType::TvShowBackdrop:
        ImageDialog::instance()->setDownloads(m_show->backdrops());
        break;
    case ImageType::TvShowBanner:
        ImageDialog::instance()->setDownloads(m_show->banners());
        break;
    default:
        ImageDialog::instance()->setDownloads(QList<Poster>());
        break;
    }

    ImageDialog::instance()->exec(image->imageType());

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = image->imageType();
        d.url = ImageDialog::instance()->imageUrl();
        d.show = m_show;
        m_posterDownloadManager->addDownload(d);
        image->setLoading(true);
        ui->buttonRevert->setVisible(true);
    }
}

void TvShowWidgetTvShow::onDeleteImage()
{
    if (m_show == 0)
        return;

    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    m_show->removeImage(image->imageType());
    updateImages(QList<int>() << image->imageType());
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onImageDropped(int imageType, QUrl imageUrl)
{
    if (!m_show)
        return;
    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    emit sigSetActionSaveEnabled(false, WidgetTvShows);
    DownloadManagerElement d;
    d.imageType = imageType;
    d.url = imageUrl;
    d.show = m_show;
    m_posterDownloadManager->addDownload(d);
    image->setLoading(true);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onVotesChange(int value)
{
    if (!m_show)
        return;
    m_show->setVotes(value);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onTop250Change(int value)
{
    if (!m_show)
        return;
    m_show->setTop250(value);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onShowScraperProgress(TvShow *show, int current, int max)
{
    if (!show->property("progressBarId").isValid())
        return;
    int id = show->property("progressBarId").toInt();
    NotificationBox::instance()->progressBarProgress(current, max, id);
}

void TvShowWidgetTvShow::onStatusChange(int index)
{
    if (!m_show)
        return;
    m_show->setStatus(ui->comboStatus->itemData(index).toString());
    ui->buttonRevert->setVisible(true);
}
