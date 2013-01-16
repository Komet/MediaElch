#include "TvShowWidgetTvShow.h"
#include "ui_TvShowWidgetTvShow.h"

#include <QFileDialog>
#include <QMovie>
#include <QPainter>
#include "globals/ComboDelegate.h"
#include "globals/Globals.h"
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
    ui->posterResolution->clear();
    ui->backdropResolution->clear();
    ui->bannerResolution->clear();
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
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewPoster->setEnabled(false);
    ui->buttonPreviewBanner->setEnabled(false);
    ui->buttonPreviewLogo->setEnabled(false);
    ui->buttonPreviewClearArt->setEnabled(false);
    ui->buttonPreviewCharacterArt->setEnabled(false);
    ui->artStackedWidget->setAnimation(QEasingCurve::OutCubic);
    ui->artStackedWidget->setSpeed(300);

    font = ui->posterResolution->font();
    #ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->posterResolution->setFont(font);
    ui->backdropResolution->setFont(font);
    ui->bannerResolution->setFont(font);
    ui->actorResolution->setFont(font);
    ui->logoResolution->setFont(font);
    ui->clearArtResolution->setFont(font);
    ui->characterArtResolution->setFont(font);

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
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onPosterDownloadFinished(DownloadManagerElement)));
    connect(m_posterDownloadManager, SIGNAL(downloadsLeft(int,DownloadManagerElement)), this, SLOT(onDownloadsLeft(int,DownloadManagerElement)));
    connect(ui->buttonPreviewPoster, SIGNAL(clicked()), this, SLOT(onPreviewPoster()));
    connect(ui->buttonPreviewBackdrop, SIGNAL(clicked()), this, SLOT(onPreviewBackdrop()));
    connect(ui->buttonPreviewBanner, SIGNAL(clicked()), this, SLOT(onPreviewBanner()));
    connect(ui->buttonPreviewLogo, SIGNAL(clicked()), this, SLOT(onPreviewLogo()));
    connect(ui->buttonPreviewClearArt, SIGNAL(clicked()), this, SLOT(onPreviewClearArt()));
    connect(ui->buttonPreviewCharacterArt, SIGNAL(clicked()), this, SLOT(onPreviewCharacterArt()));
    connect(ui->actors, SIGNAL(itemSelectionChanged()), this, SLOT(onActorChanged()));
    connect(ui->actor, SIGNAL(clicked()), this, SLOT(onChangeActorImage()));
    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));

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

    QPixmap zoomIn(":/img/zoom_in.png");
    QPainter p;
    p.begin(&zoomIn);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(zoomIn.rect(), QColor(0, 0, 0, 150));
    p.end();
    ui->buttonPreviewBackdrop->setIcon(QIcon(zoomIn));
    ui->buttonPreviewPoster->setIcon(QIcon(zoomIn));
    ui->buttonPreviewBanner->setIcon(QIcon(zoomIn));
    ui->buttonPreviewLogo->setIcon(QIcon(zoomIn));
    ui->buttonPreviewClearArt->setIcon(QIcon(zoomIn));
    ui->buttonPreviewCharacterArt->setIcon(QIcon(zoomIn));

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
    ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
    ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->logo->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->clearArt->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->characterArt->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->tabWidget->setCurrentIndex(0);
    ui->posterResolution->clear();
    ui->backdropResolution->clear();
    ui->bannerResolution->clear();
    ui->logoResolution->clear();
    ui->clearArtResolution->clear();
    ui->characterArtResolution->clear();
    ui->genreCloud->clear();

    QMapIterator<int, QList<QWidget*> > it(m_seasonLayoutWidgets);
    while (it.hasNext()) {
        it.next();
        ui->seasonsLayout->removeWidget(it.value().at(0));
        ui->seasonsLayout->removeWidget(it.value().at(1));
        it.value().at(0)->deleteLater();
        it.value().at(1)->deleteLater();
    }
    m_seasonLayoutWidgets.clear();
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
    foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows())
        genres.append(show->genres());
    ui->genreCloud->setTags(genres, m_show->genres());

    QStringList certifications = m_show->certifications();
    certifications.prepend("");
    ui->certification->addItems(certifications);
    ui->certification->setCurrentIndex(certifications.indexOf(m_show->certification()));

    // Poster
    if (!m_show->posterImage()->isNull()) {
        ui->poster->setPixmap(QPixmap::fromImage(*m_show->posterImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(m_show->posterImage()->width()).arg(m_show->posterImage()->height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = *m_show->posterImage();
    } else if (!Manager::instance()->mediaCenterInterface()->posterImageName(m_show).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->posterImageName(m_show));
        ui->poster->setPixmap(p.scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = p.toImage();
    } else {
        ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
        ui->posterResolution->setText("");
        ui->buttonPreviewPoster->setEnabled(false);
    }

    // Backdrop
    if (!m_show->backdropImage()->isNull()) {
        ui->backdrop->setPixmap(QPixmap::fromImage(*m_show->backdropImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(m_show->backdropImage()->width()).arg(m_show->backdropImage()->height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = *m_show->backdropImage();
    } else if (!Manager::instance()->mediaCenterInterface()->backdropImageName(m_show).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->backdropImageName(m_show));
        ui->backdrop->setPixmap(p.scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = p.toImage();
    } else {
        ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText("");
        ui->buttonPreviewBackdrop->setEnabled(false);
    }

    // Banner
    if (!m_show->bannerImage()->isNull()) {
        ui->banner->setPixmap(QPixmap::fromImage(*m_show->bannerImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->bannerResolution->setText(QString("%1x%2").arg(m_show->bannerImage()->width()).arg(m_show->bannerImage()->height()));
        ui->buttonPreviewBanner->setEnabled(true);
        m_currentBanner = *m_show->bannerImage();
    } else if (!Manager::instance()->mediaCenterInterface()->bannerImageName(m_show).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->bannerImageName(m_show));
        ui->banner->setPixmap(p.scaledToWidth(200, Qt::SmoothTransformation));
        ui->bannerResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewBanner->setEnabled(true);
        m_currentBanner = p.toImage();
    } else {
        ui->banner->setPixmap(QPixmap(":/img/pictures_alt_small.png"));
        ui->bannerResolution->setText("");
        ui->buttonPreviewBanner->setEnabled(false);
    }

    // Logo
    if (!m_show->logoImage()->isNull()) {
        ui->logo->setPixmap(QPixmap::fromImage(*m_show->logoImage()).scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->logoResolution->setText(QString("%1x%2").arg(m_show->logoImage()->width()).arg(m_show->logoImage()->height()));
        ui->buttonPreviewLogo->setEnabled(true);
        m_currentLogo = *m_show->logoImage();
    } else if (!Manager::instance()->mediaCenterInterface()->logoImageName(m_show).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->logoImageName(m_show));
        ui->logo->setPixmap(p.scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->logoResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewLogo->setEnabled(true);
        m_currentLogo = p.toImage();
    } else {
        ui->logo->setPixmap(QPixmap(":/img/pictures_alt_small.png"));
        ui->logoResolution->setText("");
        ui->buttonPreviewBanner->setEnabled(false);
    }

    // Clear Art
    if (!m_show->clearArtImage()->isNull()) {
        ui->clearArt->setPixmap(QPixmap::fromImage(*m_show->clearArtImage()).scaled(200, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->clearArtResolution->setText(QString("%1x%2").arg(m_show->clearArtImage()->width()).arg(m_show->clearArtImage()->height()));
        ui->buttonPreviewClearArt->setEnabled(true);
        m_currentClearArt = *m_show->clearArtImage();
    } else if (!Manager::instance()->mediaCenterInterface()->clearArtImageName(m_show).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->clearArtImageName(m_show));
        ui->clearArt->setPixmap(p.scaled(200, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->clearArtResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewClearArt->setEnabled(true);
        m_currentClearArt = p.toImage();
    } else {
        ui->clearArt->setPixmap(QPixmap(":/img/pictures_alt_small.png"));
        ui->clearArtResolution->setText("");
        ui->buttonPreviewClearArt->setEnabled(false);
    }

    // Character Art
    if (!m_show->characterArtImage()->isNull()) {
        ui->characterArt->setPixmap(QPixmap::fromImage(*m_show->characterArtImage()).scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->characterArtResolution->setText(QString("%1x%2").arg(m_show->characterArtImage()->width()).arg(m_show->characterArtImage()->height()));
        ui->buttonPreviewCharacterArt->setEnabled(true);
        m_currentCharacterArt = *m_show->characterArtImage();
    } else if (!Manager::instance()->mediaCenterInterface()->characterArtImageName(m_show).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->characterArtImageName(m_show));
        ui->characterArt->setPixmap(p.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->characterArtResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewCharacterArt->setEnabled(true);
        m_currentCharacterArt = p.toImage();
    } else {
        ui->characterArt->setPixmap(QPixmap(":/img/pictures_alt_small.png"));
        ui->characterArtResolution->setText("");
        ui->buttonPreviewCharacterArt->setEnabled(false);
    }

    QMapIterator<int, QList<QWidget*> > it(m_seasonLayoutWidgets);
    while (it.hasNext()) {
        it.next();
        ui->seasonsLayout->removeWidget(it.value().at(0));
        ui->seasonsLayout->removeWidget(it.value().at(1));
        it.value().at(0)->deleteLater();
        it.value().at(1)->deleteLater();
    }
    m_seasonLayoutWidgets.clear();
    int row=0;
    foreach (int season, m_show->seasons()) {
        QLabel *label = new QLabel(tr("Season %1").arg(season));
        MyLabel *poster = new MyLabel;
        poster->setMinimumSize(150, 225);
        poster->setToolTip(tr("Click to Change"));
        poster->setCursor(Qt::PointingHandCursor);
        poster->setSeason(season);
        poster->setAlignment(Qt::AlignCenter);
        if (!Manager::instance()->mediaCenterInterface()->seasonPosterImageName(m_show, season).isEmpty()) {
            QPixmap p(Manager::instance()->mediaCenterInterface()->seasonPosterImageName(m_show, season));
            poster->setPixmap(p.scaledToWidth(150, Qt::SmoothTransformation));
        } else {
            poster->setPixmap(QPixmap(":/img/film_reel.png"));
        }
        connect(poster, SIGNAL(seasonClicked(int)), this, SLOT(onChooseSeasonPoster(int)));
        ui->seasonsLayout->setWidget(row, QFormLayout::LabelRole, label);
        ui->seasonsLayout->setWidget(row, QFormLayout::FieldRole, poster);
        row++;
        QList<QWidget*> widgets;
        widgets.append(label);
        widgets.append(poster);
        m_seasonLayoutWidgets.insert(season, widgets);
    }

    ui->certification->blockSignals(false);
    ui->rating->blockSignals(false);
    ui->firstAired->blockSignals(false);
    ui->overview->blockSignals(false);
    ui->buttonRevert->setVisible(m_show->hasChanged());
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
    TvShowSearch::instance()->setChkUpdateAllVisible(true);
    TvShowSearch::instance()->exec(m_show->name());
    if (TvShowSearch::instance()->result() == QDialog::Accepted) {
        onSetEnabled(false);
        m_show->loadData(TvShowSearch::instance()->scraperId(), Manager::instance()->tvScrapers().at(0), TvShowSearch::instance()->updateAll(), TvShowSearch::instance()->infosToLoad());
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
    if (!show->tvdbId().isEmpty() && !types.isEmpty()) {
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
    if (show->posters().size() > 0) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = show->posters().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show) {
            ui->poster->setPixmap(QPixmap());
            ui->poster->setMovie(m_loadingMovie);
        }
    }

    if (show->backdrops().size() > 0) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = show->backdrops().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show) {
            ui->backdrop->setPixmap(QPixmap());
            ui->backdrop->setMovie(m_loadingMovie);
        }
    }

    if (show->banners().size() > 0) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBanner;
        d.url = show->banners().at(0).originalUrl;
        d.show = show;
        m_posterDownloadManager->addDownload(d);
        downloadsSize++;
        if (m_show == show) {
            ui->banner->setPixmap(QPixmap());
            ui->banner->setMovie(m_loadingMovie);
        }
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
            if (m_show == show) {
                ui->clearArt->setPixmap(QPixmap());
                ui->clearArt->setMovie(m_loadingMovie);
            }
            downloadsSize++;
        } else if (it.key() == TypeCharacterArt && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = TypeCharacterArt;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show) {
                ui->characterArt->setPixmap(QPixmap());
                ui->characterArt->setMovie(m_loadingMovie);
            }
            downloadsSize++;
        } else if (it.key() == TypeLogo && !it.value().isEmpty()) {
            DownloadManagerElement d;
            d.imageType = TypeLogo;
            d.url = it.value().at(0).originalUrl;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            if (m_show == show) {
                ui->logo->setPixmap(QPixmap());
                ui->logo->setMovie(m_loadingMovie);
            }
            downloadsSize++;
        }
    }

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

    foreach (int season, show->seasons()) {
        if (!show->seasonPosters(season).isEmpty()) {
            emit sigSetActionSaveEnabled(false, WidgetTvShows);
            DownloadManagerElement d;
            d.imageType = TypeSeasonPoster;
            d.url = show->seasonPosters(season).at(0).originalUrl;
            d.season = season;
            d.show = show;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
            if (m_show == show && m_seasonLayoutWidgets.contains(season)) {
                static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setPixmap(QPixmap());
                static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setMovie(m_loadingMovie);
            }
        }
    }

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
        ui->poster->setPixmap(QPixmap());
        ui->poster->setMovie(m_loadingMovie);
        ui->buttonPreviewPoster->setEnabled(false);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the MovieImageDialog and after successful execution starts season poster download
 */
void TvShowWidgetTvShow::onChooseSeasonPoster(int season)
{
    qDebug() << "Entered";
    if (m_show == 0) {
        qDebug() << "My show is invalid";
        return;
    }

    ImageDialog::instance()->setImageType(TypePoster);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShow(m_show);
    ImageDialog::instance()->setSeason(season);
    ImageDialog::instance()->setDownloads(m_show->seasonPosters(season));
    ImageDialog::instance()->exec(ImageDialogType::TvShowSeason);
    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeSeasonPoster;
        d.url = ImageDialog::instance()->imageUrl();
        d.season = season;
        d.show = m_show;
        m_posterDownloadManager->addDownload(d);
        if (m_seasonLayoutWidgets.contains(season)) {
            static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setPixmap(QPixmap());
            static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setMovie(m_loadingMovie);
        }
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
        ui->backdrop->setPixmap(QPixmap());
        ui->backdrop->setMovie(m_loadingMovie);
        ui->buttonPreviewBackdrop->setEnabled(false);
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
        ui->banner->setPixmap(QPixmap());
        ui->banner->setMovie(m_loadingMovie);
        ui->buttonPreviewBanner->setEnabled(false);
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
        ui->logo->setPixmap(QPixmap());
        ui->logo->setMovie(m_loadingMovie);
        ui->buttonPreviewLogo->setEnabled(false);
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
        ui->clearArt->setPixmap(QPixmap());
        ui->clearArt->setMovie(m_loadingMovie);
        ui->buttonPreviewClearArt->setEnabled(false);
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
        ui->characterArt->setPixmap(QPixmap());
        ui->characterArt->setMovie(m_loadingMovie);
        ui->buttonPreviewCharacterArt->setEnabled(false);
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
    if (elem.imageType == TypePoster) {
        qDebug() << "Got a poster";
        if (m_show == elem.show) {
            ui->poster->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->posterResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewPoster->setEnabled(true);
            m_currentPoster = elem.image;
        }
        elem.show->setPosterImage(elem.image);
    } else if (elem.imageType == TypeBackdrop) {
        qDebug() << "Got a backdop";
        if ((elem.image.width() != 1920 || elem.image.height() != 1080) &&
            elem.image.width() > 1915 && elem.image.width() < 1925 && elem.image.height() > 1075 && elem.image.height() < 1085)
            elem.image = elem.image.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        if ((elem.image.width() != 1280 || elem.image.height() != 720) &&
            elem.image.width() > 1275 && elem.image.width() < 1285 && elem.image.height() > 715 && elem.image.height() < 725)
            elem.image = elem.image.scaled(1280, 720, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        if (m_show == elem.show) {
            ui->backdrop->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->backdropResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewBackdrop->setEnabled(true);
            m_currentBackdrop = elem.image;
        }
        elem.show->setBackdropImage(elem.image);
    } else if (elem.imageType == TypeBanner) {
        qDebug() << "Got a banner";
        if (m_show == elem.show) {
            ui->banner->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 37, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->bannerResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewBanner->setEnabled(true);
            m_currentBanner = elem.image;
        }
        elem.show->setBannerImage(elem.image);
    } else if (elem.imageType == TypeSeasonPoster) {
        qDebug() << "Got a season poster";
        int season = elem.season;
        elem.show->setSeasonPosterImage(season, elem.image);
        if (m_show == elem.show && m_seasonLayoutWidgets.contains(season)) {
            static_cast<MyLabel*>(m_seasonLayoutWidgets[season].at(1))->setPixmap(QPixmap::fromImage(elem.image).scaled(150, 225, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    } else if (elem.imageType == TypeLogo) {
        qDebug() << "Got a logo";
        if (m_show == elem.show) {
            ui->logo->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->logoResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewLogo->setEnabled(true);
            m_currentLogo = elem.image;
        }
        elem.show->setLogoImage(elem.image);
    } else if (elem.imageType == TypeCharacterArt) {
        qDebug() << "Got a character art";
        if (m_show == elem.show) {
            ui->characterArt->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->characterArtResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewCharacterArt->setEnabled(true);
            m_currentCharacterArt = elem.image;
        }
        elem.show->setCharacterArtImage(elem.image);
    } else if (elem.imageType == TypeClearArt) {
        qDebug() << "Got a clear art";
        if (m_show == elem.show) {
            ui->clearArt->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->clearArtResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewClearArt->setEnabled(true);
            m_currentClearArt = elem.image;
        }
        elem.show->setClearArtImage(elem.image);
    }

    if (m_posterDownloadManager->downloadsLeftForShow(m_show) == 0)
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
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
 * @brief Shows the full size backdrop image
 */
void TvShowWidgetTvShow::onPreviewBackdrop()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentBackdrop));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows the full size poster image
 */
void TvShowWidgetTvShow::onPreviewPoster()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentPoster));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows the full size banner image
 */
void TvShowWidgetTvShow::onPreviewBanner()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentBanner));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows the full size logo image
 */
void TvShowWidgetTvShow::onPreviewLogo()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentLogo));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows the full size clear art image
 */
void TvShowWidgetTvShow::onPreviewClearArt()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentClearArt));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows the full size character art image
 */
void TvShowWidgetTvShow::onPreviewCharacterArt()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentCharacterArt));
    ImagePreviewDialog::instance()->exec();
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
        ui->actor->setPixmap(QPixmap::fromImage(actor->image).scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->actorResolution->setText(QString("%1 x %2").arg(actor->image.width()).arg(actor->image.height()));
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
            Actor *actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
            actor->image.load(fileName);
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
