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
#include "main/MessageBox.h"
#include "tvShows/TvShowSearch.h"

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
#if QT_VERSION >= 0x050000
    ui->actors->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    ui->actors->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    QFont font = ui->showTitle->font();
    font.setPointSize(font.pointSize()+4);
    ui->showTitle->setFont(font);
    ui->artStackedWidget->setAnimation(QEasingCurve::OutCubic);
    ui->artStackedWidget->setSpeed(300);

    ui->poster->setDefaultPixmap(QPixmap(":/img/film_reel.png"));
    ui->backdrop->setDefaultPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->banner->setDefaultPixmap(QPixmap());
    ui->logo->setDefaultPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->clearArt->setDefaultPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->characterArt->setDefaultPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    font = ui->actorResolution->font();
    #ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->actorResolution->setFont(font);

    ui->genreCloud->setText(tr("Genres"));
    ui->genreCloud->setPlaceholder(tr("Add Genre"));
    connect(ui->genreCloud, SIGNAL(activated(QString)), this, SLOT(onAddGenre(QString)));
    connect(ui->genreCloud, SIGNAL(deactivated(QString)), this, SLOT(onRemoveGenre(QString)));

    ui->tagCloud->setText(tr("Tags"));
    ui->tagCloud->setPlaceholder(tr("Add Tag"));
    connect(ui->tagCloud, SIGNAL(activated(QString)), this, SLOT(onAddTag(QString)));
    connect(ui->tagCloud, SIGNAL(deactivated(QString)), this, SLOT(onRemoveTag(QString)));

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();
    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    m_posterDownloadManager = new DownloadManager(this);

    connect(ui->name, SIGNAL(textChanged(QString)), ui->showTitle, SLOT(setText(QString)));
    connect(ui->buttonAddActor, SIGNAL(clicked()), this, SLOT(onAddActor()));
    connect(ui->buttonRemoveActor, SIGNAL(clicked()), this, SLOT(onRemoveActor()));
    connect(ui->poster, SIGNAL(clicked()), this, SLOT(onChoosePoster()));
    connect(ui->backdrop, SIGNAL(clicked()), this, SLOT(onChooseBackdrop()));
    connect(ui->banner, SIGNAL(clicked()), this, SLOT(onChooseBanner()));
    connect(ui->clearArt, SIGNAL(clicked()), this, SLOT(onChooseClearArt()));
    connect(ui->characterArt, SIGNAL(clicked()), this, SLOT(onChooseCharacterArt()));
    connect(ui->logo, SIGNAL(clicked()), this, SLOT(onChooseLogo()));
    connect(ui->poster, SIGNAL(sigClose()), this, SLOT(onDeletePoster()));
    connect(ui->backdrop, SIGNAL(sigClose()), this, SLOT(onDeleteBackdrop()));
    connect(ui->banner, SIGNAL(sigClose()), this, SLOT(onDeleteBanner()));
    connect(ui->clearArt, SIGNAL(sigClose()), this, SLOT(onDeleteClearArt()));
    connect(ui->characterArt, SIGNAL(sigClose()), this, SLOT(onDeleteCharacterArt()));
    connect(ui->logo, SIGNAL(sigClose()), this, SLOT(onDeleteLogo()));
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onPosterDownloadFinished(DownloadManagerElement)));
    connect(m_posterDownloadManager, SIGNAL(downloadsLeft(int,DownloadManagerElement)), this, SLOT(onDownloadsLeft(int,DownloadManagerElement)));
    connect(ui->actors, SIGNAL(itemSelectionChanged()), this, SLOT(onActorChanged()));
    connect(ui->actor, SIGNAL(clicked()), this, SLOT(onChangeActorImage()));
    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));

    connect(ui->fanarts, SIGNAL(sigRemoveImage(QByteArray)), this, SLOT(onRemoveExtraFanart(QByteArray)));
    connect(ui->fanarts, SIGNAL(sigRemoveImage(QString)), this, SLOT(onRemoveExtraFanart(QString)));
    connect(ui->btnAddExtraFanart, SIGNAL(clicked()), this, SLOT(onAddExtraFanart()));

    onClear();

    // Connect GUI change events to movie object
    connect(ui->name, SIGNAL(textEdited(QString)), this, SLOT(onNameChange(QString)));
    connect(ui->certification, SIGNAL(editTextChanged(QString)), this, SLOT(onCertificationChange(QString)));
    connect(ui->rating, SIGNAL(valueChanged(double)), this, SLOT(onRatingChange(double)));
    connect(ui->firstAired, SIGNAL(dateChanged(QDate)), this, SLOT(onFirstAiredChange(QDate)));
    connect(ui->studio, SIGNAL(textEdited(QString)), this, SLOT(onStudioChange(QString)));
    connect(ui->overview, SIGNAL(textChanged()), this, SLOT(onOverviewChange()));
    connect(ui->actors, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onActorEdited(QTableWidgetItem*)));

    onSetEnabled(false);

    QPainter p;
    QPixmap revert(":/img/arrow_circle_left.png");
    p.begin(&revert);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(revert.rect(), QColor(0, 0, 0, 200));
    p.end();
    ui->buttonRevert->setIcon(QIcon(revert));
    ui->buttonRevert->setVisible(false);
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
    ui->showTitle->clear();
    ui->actors->setRowCount(0);
    ui->dir->clear();
    ui->name->clear();
    ui->rating->clear();
    ui->certification->clear();
    ui->firstAired->setDate(QDate::currentDate());
    ui->studio->clear();
    ui->overview->clear();
    ui->genreCloud->clear();
    ui->fanarts->clear();
    ui->poster->clear();
    ui->backdrop->clear();
    ui->banner->clear();
    ui->logo->clear();
    ui->clearArt->clear();
    ui->characterArt->clear();
    ui->tagCloud->clear();
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
    ui->firstAired->blockSignals(true);
    ui->overview->blockSignals(true);

    onClear();

    ui->dir->setText(m_show->dir());
    ui->name->setText(m_show->name());
    ui->rating->setValue(m_show->rating());
    ui->firstAired->setDate(m_show->firstAired());
    ui->studio->setText(m_show->network());
    ui->overview->setPlainText(m_show->overview());

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

    updateImages(QList<ImageType>() << TypePoster << TypeBackdrop << TypeBanner << TypeCharacterArt << TypeClearArt << TypeLogo);
    ui->fanarts->setImages(m_show->extraFanarts(Manager::instance()->mediaCenterInterfaceTvShow()));

    ui->certification->blockSignals(false);
    ui->rating->blockSignals(false);
    ui->firstAired->blockSignals(false);
    ui->overview->blockSignals(false);
    ui->buttonRevert->setVisible(m_show->hasChanged());
}

void TvShowWidgetTvShow::updateImages(QList<ImageType> images)
{
    if (images.contains(TypePoster)) {
        if (!m_show->posterImage().isNull())
            ui->poster->setImage(m_show->posterImage());
        else if (!m_show->imagesToRemove().contains(TypePoster) && !Manager::instance()->mediaCenterInterface()->posterImageName(m_show).isEmpty())
            ui->poster->setImage(Manager::instance()->mediaCenterInterface()->posterImageName(m_show));
    }

    if (images.contains(TypeBackdrop)) {
        if (!m_show->backdropImage().isNull())
            ui->backdrop->setImage(m_show->backdropImage());
        else if (!m_show->imagesToRemove().contains(TypeBackdrop) && !Manager::instance()->mediaCenterInterface()->backdropImageName(m_show).isEmpty())
            ui->backdrop->setImage(Manager::instance()->mediaCenterInterface()->backdropImageName(m_show));
    }

    if (images.contains(TypeBanner)) {
        if (!m_show->bannerImage().isNull())
            ui->banner->setImage(m_show->bannerImage());
        else if (!m_show->imagesToRemove().contains(TypeBanner) && !Manager::instance()->mediaCenterInterface()->bannerImageName(m_show).isEmpty())
            ui->banner->setImage(Manager::instance()->mediaCenterInterface()->bannerImageName(m_show));
    }

    if (images.contains(TypeLogo)) {
        if (!m_show->logoImage().isNull())
            ui->logo->setImage(m_show->logoImage());
        else if (!m_show->imagesToRemove().contains(TypeLogo) && !Manager::instance()->mediaCenterInterface()->logoImageName(m_show).isEmpty())
            ui->logo->setImage(Manager::instance()->mediaCenterInterface()->logoImageName(m_show));
    }

    if (images.contains(TypeClearArt)) {
        if (!m_show->clearArtImage().isNull())
            ui->clearArt->setImage(m_show->clearArtImage());
        else if (!m_show->imagesToRemove().contains(TypeClearArt) && !Manager::instance()->mediaCenterInterface()->clearArtImageName(m_show).isEmpty())
            ui->clearArt->setImage(Manager::instance()->mediaCenterInterface()->clearArtImageName(m_show));
    }

    if (images.contains(TypeCharacterArt)) {
        if (!m_show->characterArtImage().isNull())
            ui->characterArt->setImage(m_show->characterArtImage());
        else if (!m_show->imagesToRemove().contains(TypeCharacterArt) && !Manager::instance()->mediaCenterInterface()->characterArtImageName(m_show).isEmpty())
            ui->characterArt->setImage(Manager::instance()->mediaCenterInterface()->characterArtImageName(m_show));
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
    MessageBox::instance()->showMessage(tr("<b>\"%1\"</b> Saved").arg(m_show->name()));
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
    TvShowSearch::instance()->exec(m_show->name());
    if (TvShowSearch::instance()->result() == QDialog::Accepted) {
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
    QList<int> types;
    types << TypeClearArt << TypeLogo << TypeCharacterArt;
    if (!show->tvdbId().isEmpty() && !types.isEmpty() && show->infosToLoad().contains(TvShowScraperInfos::ExtraArts)) {
        Manager::instance()->fanartTv()->tvShowImages(show, show->tvdbId(), types);
        connect(Manager::instance()->fanartTv(), SIGNAL(sigImagesLoaded(TvShow*,QMap<int,QList<Poster> >)), this, SLOT(onLoadDone(TvShow*,QMap<int,QList<Poster> >)), Qt::UniqueConnection);
    } else {
        QMap<int, QList<Poster> > map;
        onLoadDone(show, map);
    }
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
        d.imageType = TypePoster;
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
        d.imageType = TypeBackdrop;
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
        d.imageType = TypeBanner;
        d.url = show->banners().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show)
            ui->banner->setLoading(true);
    }

    QMapIterator<int, QList<Poster> > it(posters);
    while (it.hasNext()) {
        it.next();
        if (it.key() == TypeClearArt && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = TypeClearArt;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show)
                ui->clearArt->setLoading(true);
            downloadsSize++;
        } else if (it.key() == TypeCharacterArt && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = TypeCharacterArt;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show)
                ui->characterArt->setLoading(true);
            downloadsSize++;
        } else if (it.key() == TypeLogo && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = TypeLogo;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show)
                ui->logo->setLoading(true);
            downloadsSize++;
        }
    }

    if (show->infosToLoad().contains(TvShowScraperInfos::Actors) && Settings::instance()->downloadActorImages()) {
        QList<Actor*> actors = show->actorsPointer();
        for (int i=0, n=actors.size() ; i<n ; ++i) {
            if (actors.at(i)->thumb.isEmpty())
                continue;
            DownloadManagerElement d;
            d.imageType = TypeActor;
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
            d.imageType = TypeSeasonPoster;
            d.url = show->seasonPosters(season).at(0).originalUrl;
            d.season = season;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
        }
        if (!show->seasonBackdrops(season).isEmpty() && show->infosToLoad().contains(TvShowScraperInfos::SeasonBackdrop)) {
            emit sigSetActionSaveEnabled(false, WidgetTvShows);
            DownloadManagerElement d;
            d.imageType = TypeSeasonBackdrop;
            d.url = show->seasonBackdrops(season).at(0).originalUrl;
            d.season = season;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
        }
        if (!show->seasonBanners(season).isEmpty() && show->infosToLoad().contains(TvShowScraperInfos::SeasonBanner)) {
            emit sigSetActionSaveEnabled(false, WidgetTvShows);
            DownloadManagerElement d;
            d.imageType = TypeSeasonBanner;
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
            d.imageType = TypeShowThumbnail;
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
 * @brief Shows the MovieImageDialog and after successful execution starts poster download
 */
void TvShowWidgetTvShow::onChoosePoster()
{
    qDebug() << "Entered";
    if (m_show == 0) {
        qDebug() << "My show is invalid";
        return;
    }

    ImageDialog::instance()->setImageType(TypePoster);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setDownloads(m_show->posters());
    ImageDialog::instance()->exec(ImageDialogType::TvShowPoster);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = ImageDialog::instance()->imageUrl();
        d.show = m_show;
        m_posterDownloadManager->addDownload(d);
        ui->poster->setLoading(true);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the MovieImageDialog and after successful execution starts backdrop download
 */
void TvShowWidgetTvShow::onChooseBackdrop()
{
    qDebug() << "Entered";
    if (m_show == 0) {
        qDebug() << "My show is invalid";
        return;
    }
    ImageDialog::instance()->setImageType(TypeBackdrop);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setDownloads(m_show->backdrops());
    ImageDialog::instance()->exec(ImageDialogType::TvShowBackdrop);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = ImageDialog::instance()->imageUrl();
        d.show = m_show;
        m_posterDownloadManager->addDownload(d);
        ui->backdrop->setLoading(true);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the MovieImageDialog and after successful execution starts banner download
 */
void TvShowWidgetTvShow::onChooseBanner()
{
    qDebug() << "Entered";
    if (m_show == 0) {
        qDebug() << "My show is invalid";
        return;
    }

    ImageDialog::instance()->setImageType(TypeBanner);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setDownloads(m_show->banners());
    ImageDialog::instance()->exec(ImageDialogType::TvShowBanner);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBanner;
        d.url = ImageDialog::instance()->imageUrl();
        d.show = m_show;
        m_posterDownloadManager->addDownload(d);
        ui->banner->setLoading(true);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the ImageDialog and after successful execution starts logo download
 */
void TvShowWidgetTvShow::onChooseLogo()
{
    qDebug() << "Entered";
    if (m_show == 0) {
        qDebug() << "My show is invalid";
        return;
    }

    ImageDialog::instance()->setImageType(TypeLogo);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(ImageDialogType::TvShowLogos);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeLogo;
        d.url = ImageDialog::instance()->imageUrl();
        d.show = m_show;
        m_posterDownloadManager->addDownload(d);
        ui->logo->setLoading(true);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the ImageDialog and after successful execution starts clear art download
 */
void TvShowWidgetTvShow::onChooseClearArt()
{
    qDebug() << "Entered";
    if (m_show == 0) {
        qDebug() << "My show is invalid";
        return;
    }

    ImageDialog::instance()->setImageType(TypeClearArt);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(ImageDialogType::TvShowClearArt);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeClearArt;
        d.url = ImageDialog::instance()->imageUrl();
        d.show = m_show;
        m_posterDownloadManager->addDownload(d);
        ui->clearArt->setLoading(true);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the ImageDialog and after successful execution starts character art download
 */
void TvShowWidgetTvShow::onChooseCharacterArt()
{
    qDebug() << "Entered";
    if (m_show == 0) {
        qDebug() << "My show is invalid";
        return;
    }

    ImageDialog::instance()->setImageType(TypeCharacterArt);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(ImageDialogType::TvShowCharacterArt);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeCharacterArt;
        d.url = ImageDialog::instance()->imageUrl();
        d.show = m_show;
        m_posterDownloadManager->addDownload(d);
        ui->characterArt->setLoading(true);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Adjusts the size of the backdrop to common values (1080p or 720p) and shows the image
 * @param elem Downloaded element
 */
void TvShowWidgetTvShow::onPosterDownloadFinished(DownloadManagerElement elem)
{
    qDebug() << "Entered";

    switch (elem.imageType) {
    case TypePoster:
        if (m_show == elem.show)
            ui->poster->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->posterImageName(elem.show));
        elem.show->setPosterImage(elem.data);
        break;
    case TypeBackdrop:
        Helper::resizeBackdrop(elem.data);
        if (m_show == elem.show)
            ui->backdrop->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->backdropImageName(elem.show));
        elem.show->setBackdropImage(elem.data);
        break;
    case TypeBanner:
        if (m_show == elem.show)
            ui->banner->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->bannerImageName(elem.show));
        elem.show->setBannerImage(elem.data);
        break;
    case TypeLogo:
        if (m_show == elem.show)
            ui->logo->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->logoImageName(elem.show));
        elem.show->setLogoImage(elem.data);
        break;
    case TypeCharacterArt:
        if (m_show == elem.show)
            ui->characterArt->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->characterArtImageName(elem.show));
        elem.show->setCharacterArtImage(elem.data);
        break;
    case TypeClearArt:
        if (m_show == elem.show)
            ui->clearArt->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->clearArtImageName(elem.show));
        elem.show->setClearArtImage(elem.data);
        break;
    case TypeExtraFanart:
        Helper::resizeBackdrop(elem.data);
        elem.show->addExtraFanart(elem.data);
        if (elem.show == m_show)
            ui->fanarts->addImage(elem.data);
        break;
    case TypeSeasonPoster:
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->seasonPosterImageName(elem.show, elem.season));
        elem.show->setSeasonPosterImage(elem.season, elem.data);
        break;
    case TypeSeasonBackdrop:
        Helper::resizeBackdrop(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->seasonBackdropImageName(elem.show, elem.season));
        elem.show->setSeasonBackdropImage(elem.season, elem.data);
        break;
    case TypeSeasonBanner:
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->seasonBannerImageName(elem.show, elem.season));
        elem.show->setSeasonBannerImage(elem.season, elem.data);
        break;
    default:
        break;
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

void TvShowWidgetTvShow::onDeleteBackdrop()
{
    m_show->removeImage(TypeBackdrop);
    updateImages(QList<ImageType>() << TypeBackdrop);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onDeletePoster()
{
    m_show->removeImage(TypePoster);
    updateImages(QList<ImageType>() << TypePoster);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onDeleteBanner()
{
    m_show->removeImage(TypeBanner);
    updateImages(QList<ImageType>() << TypeBanner);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onDeleteLogo()
{
    m_show->removeImage(TypeLogo);
    updateImages(QList<ImageType>() << TypeLogo);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onDeleteClearArt()
{
    m_show->removeImage(TypeClearArt);
    updateImages(QList<ImageType>() << TypeClearArt);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetTvShow::onDeleteCharacterArt()
{
    m_show->removeImage(TypeCharacterArt);
    updateImages(QList<ImageType>() << TypeCharacterArt);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Shows the image of the selected actor
 */
void TvShowWidgetTvShow::onActorChanged()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount() ||
        ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        ui->actor->setPixmap(QPixmap(":/img/man.png"));
        ui->actorResolution->setText("");
        return;
    }

    Actor *actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
    if (!actor->image.isNull()) {
        QImage img = QImage::fromData(actor->image);
        ui->actor->setPixmap(QPixmap::fromImage(img).scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->actorResolution->setText(QString("%1 x %2").arg(img.width()).arg(img.height()));
    } else if (!Manager::instance()->mediaCenterInterface()->actorImageName(m_show, *actor).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->actorImageName(m_show, *actor));
        ui->actor->setPixmap(p.scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->actorResolution->setText(QString("%1 x %2").arg(p.width()).arg(p.height()));
    } else {
        ui->actor->setPixmap(QPixmap(":/img/man.png"));
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

    ImageDialog::instance()->setImageType(TypeExtraFanart);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMultiSelection(true);
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setDownloads(m_show->backdrops());
    ImageDialog::instance()->exec(ImageDialogType::TvShowBackdrop);

    if (ImageDialog::instance()->result() == QDialog::Accepted && !ImageDialog::instance()->imageUrls().isEmpty()) {
        ui->fanarts->setLoading(true);
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        foreach (const QUrl &url, ImageDialog::instance()->imageUrls()) {
            DownloadManagerElement d;
            d.imageType = TypeExtraFanart;
            d.url = url;
            d.show = m_show;
            m_posterDownloadManager->addDownload(d);
        }
        ui->buttonRevert->setVisible(true);
    }
}
