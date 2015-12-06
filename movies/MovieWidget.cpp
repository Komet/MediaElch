#include "MovieWidget.h"
#include "ui_MovieWidget.h"

#include <QtCore/qmath.h>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QIntValidator>
#include <QMovie>
#include <QPainter>
#include <QPixmapCache>
#include <QScrollBar>
#include "data/ImageCache.h"
#include "globals/ComboDelegate.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "globals/TrailerDialog.h"
#include "notifications/NotificationBox.h"
#include "main/MainWindow.h"
#include "movies/FilesWidget.h"
#include "movies/MovieSearch.h"

/**
 * @brief MovieWidget::MovieWidget
 * @param parent
 */
MovieWidget::MovieWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MovieWidget)
{
    ui->setupUi(this);
    m_backgroundLabel = new QLabel(this);
    m_backgroundLabel->show();
    m_backgroundLabel->lower();
    ui->movieName->clear();

    ui->actors->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->subtitles->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->artStackedWidget->setAnimation(QEasingCurve::OutCubic);
    ui->artStackedWidget->setSpeed(300);
    ui->localTrailer->setBadgeType(Badge::LabelSuccess);
    ui->localTrailer->setVisible(false);
    ui->badgeWatched->setBadgeType(Badge::BadgeInfo);

#ifndef Q_OS_MAC
    QFont nameFont = ui->movieName->font();
    nameFont.setPointSize(nameFont.pointSize()-4);
    ui->movieName->setFont(nameFont);
#endif

    QFont font = ui->actorResolution->font();
    #ifndef Q_OS_MAC
        font.setPointSize(font.pointSize()-1);
    #else
        font.setPointSize(font.pointSize()-2);
    #endif
    ui->actorResolution->setFont(font);

    ui->labelBanner->setFont(font);
    ui->labelClearArt->setFont(font);
    ui->labelDiscArt->setFont(font);
    ui->labelFanart->setFont(font);
    ui->labelLogo->setFont(font);
    ui->labelPoster->setFont(font);
    ui->labelThumb->setFont(font);

    m_movie = 0;

    ui->poster->setDefaultPixmap(QPixmap(":/img/placeholders/poster.png"));
    ui->backdrop->setDefaultPixmap(QPixmap(":/img/placeholders/fanart.png"));
    ui->logo->setDefaultPixmap(QPixmap(":/img/placeholders/logo.png"));
    ui->clearArt->setDefaultPixmap(QPixmap(":/img/placeholders/clear_art.png"));
    ui->cdArt->setDefaultPixmap(QPixmap(":/img/placeholders/cd_art.png"));
    ui->thumb->setDefaultPixmap(QPixmap(":/img/placeholders/thumb.png"));
    ui->banner->setDefaultPixmap(QPixmap(":/img/placeholders/banner.png"));

    ui->buttonDownloadTrailer->setIcon(Manager::instance()->iconFont()->icon("download", QColor(150, 150, 150), "", -1, 1.0));
    ui->buttonYoutubeDummy->setIcon(Manager::instance()->iconFont()->icon("pen", QColor(150, 150, 150), "", -1, 1.0));
    ui->buttonPlayLocalTrailer->setIcon(Manager::instance()->iconFont()->icon("play", QColor(150, 150, 150), "", -1, 1.0));

    ui->genreCloud->setText(tr("Genres"));
    ui->genreCloud->setPlaceholder(tr("Add Genre"));
    connect(ui->genreCloud, SIGNAL(activated(QString)), this, SLOT(addGenre(QString)));
    connect(ui->genreCloud, SIGNAL(deactivated(QString)), this, SLOT(removeGenre(QString)));

    ui->tagCloud->setText(tr("Tags"));
    ui->tagCloud->setPlaceholder(tr("Add Tag"));
    connect(ui->tagCloud, SIGNAL(activated(QString)), this, SLOT(addTag(QString)));
    connect(ui->tagCloud, SIGNAL(deactivated(QString)), this, SLOT(removeTag(QString)));

    ui->countryCloud->setText(tr("Countries"));
    ui->countryCloud->setPlaceholder(tr("Add Country"));
    connect(ui->countryCloud, SIGNAL(activated(QString)), this, SLOT(addCountry(QString)));
    connect(ui->countryCloud, SIGNAL(deactivated(QString)), this, SLOT(removeCountry(QString)));

    ui->studioCloud->setText(tr("Studios"));
    ui->studioCloud->setPlaceholder(tr("Add Studio"));
    ui->studioCloud->setBadgeType(TagCloud::TypeSimpleLabel);
    connect(ui->studioCloud, SIGNAL(activated(QString)), this, SLOT(addStudio(QString)));
    connect(ui->studioCloud, SIGNAL(deactivated(QString)), this, SLOT(removeStudio(QString)));

    ui->labelSepFoldersWarning->setErrorMessage(ui->labelSepFoldersWarning->text());

    ui->poster->setImageType(ImageType::MoviePoster);
    ui->backdrop->setImageType(ImageType::MovieBackdrop);
    ui->logo->setImageType(ImageType::MovieLogo);
    ui->cdArt->setImageType(ImageType::MovieCdArt);
    ui->banner->setImageType(ImageType::MovieBanner);
    ui->thumb->setImageType(ImageType::MovieThumb);
    ui->clearArt->setImageType(ImageType::MovieClearArt);
    foreach (ClosableImage *image, ui->artStackedWidget->findChildren<ClosableImage*>()) {
        connect(image, &ClosableImage::clicked, this, &MovieWidget::onChooseImage);
        connect(image, &ClosableImage::sigClose, this, &MovieWidget::onDeleteImage);
        connect(image, &ClosableImage::sigImageDropped, this, &MovieWidget::onImageDropped);
    }

    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(movieNameChanged(QString)));
    connect(ui->buttonAddActor, SIGNAL(clicked()), this, SLOT(addActor()));
    connect(ui->buttonRemoveActor, SIGNAL(clicked()), this, SLOT(removeActor()));
    connect(ui->actors, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onActorEdited(QTableWidgetItem*)));
    connect(ui->actors, SIGNAL(itemSelectionChanged()), this, SLOT(onActorChanged()));
    connect(ui->actor, SIGNAL(clicked()), this, SLOT(onChangeActorImage()));
    connect(ui->subtitles, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onSubtitleEdited(QTableWidgetItem*)));
    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));
    connect(ui->buttonReloadStreamDetails, SIGNAL(clicked()), this, SLOT(onReloadStreamDetails()));

    connect(ui->buttonDownloadTrailer, &QToolButton::clicked, this, &MovieWidget::onDownloadTrailer);
    connect(ui->buttonYoutubeDummy, &QToolButton::clicked, this, &MovieWidget::onInsertYoutubeLink);
    connect(ui->buttonPlayLocalTrailer, &QToolButton::clicked, this, &MovieWidget::onPlayLocalTrailer);

    connect(ui->fanarts, SIGNAL(sigRemoveImage(QByteArray)), this, SLOT(onRemoveExtraFanart(QByteArray)));
    connect(ui->fanarts, SIGNAL(sigRemoveImage(QString)), this, SLOT(onRemoveExtraFanart(QString)));
    connect(ui->btnAddExtraFanart, SIGNAL(clicked()), this, SLOT(onAddExtraFanart()));
    connect(ui->fanarts, &ImageGallery::sigImageDropped, this, &MovieWidget::onExtraFanartDropped);

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();

    setDisabledTrue();
    clear();

    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    // Connect GUI change events to movie object
    connect(ui->imdbId, SIGNAL(textEdited(QString)), this, SLOT(onImdbIdChange(QString)));
    connect(ui->name, SIGNAL(textEdited(QString)), this, SLOT(onNameChange(QString)));
    connect(ui->originalName, SIGNAL(textEdited(QString)), this, SLOT(onOriginalNameChange(QString)));
    connect(ui->sortTitle, SIGNAL(textEdited(QString)), this, SLOT(onSortTitleChange(QString)));
    connect(ui->tagline, SIGNAL(textEdited(QString)), this, SLOT(onTaglineChange(QString)));
    connect(ui->rating, SIGNAL(valueChanged(double)), this, SLOT(onRatingChange(double)));
    connect(ui->votes, SIGNAL(valueChanged(int)), this, SLOT(onVotesChange(int)));
    connect(ui->top250, SIGNAL(valueChanged(int)), this, SLOT(onTop250Change(int)));
    connect(ui->trailer, SIGNAL(textEdited(QString)), this, SLOT(onTrailerChange(QString)));
    connect(ui->runtime, SIGNAL(valueChanged(int)), this, SLOT(onRuntimeChange(int)));
    connect(ui->playcount, SIGNAL(valueChanged(int)), this, SLOT(onPlayCountChange(int)));
    connect(ui->certification, SIGNAL(editTextChanged(QString)), this, SLOT(onCertificationChange(QString)));
    connect(ui->set, SIGNAL(editTextChanged(QString)), this, SLOT(onSetChange(QString)));
    connect(ui->badgeWatched, SIGNAL(clicked()), this, SLOT(onWatchedClicked()));
    connect(ui->released, SIGNAL(dateChanged(QDate)), this, SLOT(onReleasedChange(QDate)));
    connect(ui->lastPlayed, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(onLastWatchedChange(QDateTime)));
    connect(ui->overview, SIGNAL(textChanged()), this, SLOT(onOverviewChange()));
    connect(ui->outline, SIGNAL(textChanged()), this, SLOT(onOutlineChange()));
    connect(ui->director, SIGNAL(textEdited(QString)), this, SLOT(onDirectorChange(QString)));
    connect(ui->writer, SIGNAL(textEdited(QString)), this, SLOT(onWriterChange(QString)));
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
 * @brief MovieWidget::~MovieWidget
 */
MovieWidget::~MovieWidget()
{
    delete ui;
}

/**
 * @brief Repositions the saving widget
 * @param event
 */
void MovieWidget::resizeEvent(QResizeEvent *event)
{
    m_savingWidget->move(size().width()/2-m_savingWidget->width(), height()/2-m_savingWidget->height());
    QWidget::resizeEvent(event);
}

void MovieWidget::setBigWindow(bool bigWindow)
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
void MovieWidget::clear()
{
    qDebug() << "Entered";
    bool blocked;
    blocked = ui->set->blockSignals(true);
    ui->set->clear();
    ui->set->blockSignals(blocked);

    blocked = ui->certification->blockSignals(true);
    ui->certification->clear();
    ui->certification->blockSignals(blocked);

    blocked = ui->director->blockSignals(true);
    ui->director->clear();
    ui->director->blockSignals(blocked);

    blocked = ui->writer->blockSignals(true);
    ui->writer->clear();
    ui->writer->blockSignals(blocked);

    blocked = ui->movieName->blockSignals(true);
    ui->movieName->clear();
    ui->movieName->blockSignals(blocked);

    blocked = ui->files->blockSignals(true);
    ui->files->clear();
    ui->files->blockSignals(blocked);

    blocked = ui->imdbId->blockSignals(true);
    ui->imdbId->clear();
    ui->imdbId->blockSignals(blocked);

    blocked = ui->name->blockSignals(true);
    ui->name->clear();
    ui->name->blockSignals(blocked);

    blocked = ui->originalName->blockSignals(true);
    ui->originalName->clear();
    ui->originalName->blockSignals(blocked);

    blocked = ui->sortTitle->blockSignals(true);
    ui->sortTitle->clear();
    ui->sortTitle->blockSignals(blocked);

    blocked = ui->tagline->blockSignals(true);
    ui->tagline->clear();
    ui->tagline->blockSignals(blocked);

    blocked = ui->rating->blockSignals(true);
    ui->rating->clear();
    ui->rating->blockSignals(blocked);

    blocked = ui->votes->blockSignals(true);
    ui->votes->clear();
    ui->votes->blockSignals(blocked);

    blocked = ui->top250->blockSignals(true);
    ui->top250->clear();
    ui->top250->blockSignals(blocked);

    blocked = ui->released->blockSignals(true);
    ui->released->setDate(QDate::currentDate());
    ui->released->blockSignals(blocked);

    blocked = ui->runtime->blockSignals(true);
    ui->runtime->clear();
    ui->runtime->blockSignals(blocked);

    blocked = ui->trailer->blockSignals(true);
    ui->trailer->clear();
    ui->trailer->blockSignals(blocked);

    blocked = ui->playcount->blockSignals(true);
    ui->playcount->clear();
    ui->playcount->blockSignals(blocked);

    blocked = ui->lastPlayed->blockSignals(true);
    ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
    ui->lastPlayed->blockSignals(blocked);

    blocked = ui->overview->blockSignals(true);
    ui->overview->clear();
    ui->overview->blockSignals(blocked);

    blocked = ui->outline->blockSignals(true);
    ui->outline->clear();
    ui->outline->blockSignals(blocked);

    blocked = ui->actors->blockSignals(true);
    ui->actors->setRowCount(0);
    ui->actors->blockSignals(false);

    blocked = ui->subtitles->blockSignals(true);
    ui->subtitles->setRowCount(0);
    ui->subtitles->blockSignals(false);

    ui->videoCodec->clear();
    ui->videoScantype->clear();

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

    QPixmap pixmap(":/img/man.png");
    Helper::instance()->setDevicePixelRatio(pixmap, Helper::instance()->devicePixelRatio(this));
    ui->actor->setPixmap(pixmap);

    ui->actorResolution->setText("");
    ui->genreCloud->clear();
    ui->countryCloud->clear();
    ui->studioCloud->clear();
    ui->tagCloud->clear();
    ui->buttonRevert->setVisible(false);
    ui->localTrailer->setVisible(false);
    ui->fanarts->clear();

    ui->poster->clear();
    ui->backdrop->clear();
    ui->logo->clear();
    ui->clearArt->clear();
    ui->cdArt->clear();
    ui->banner->clear();
    ui->thumb->clear();
}

/**
 * @brief Updates the title text
 * @param text New text
 */
void MovieWidget::movieNameChanged(QString text)
{
    ui->movieName->setText(text);
}

/**
 * @brief Sets the state of the main groupbox to enabled
 * @param movie Current movie
 */
void MovieWidget::setEnabledTrue(Movie *movie)
{
    qDebug() << "Entered";
    if (movie)
        qDebug() << movie->name();
    if (movie && movie->controller()->downloadsInProgress()) {
        qDebug() << "Downloads are in progress";
        return;
    }
    ui->groupBox_3->setEnabled(true);
    emit setActionSaveEnabled(true, WidgetMovies);
    emit setActionSearchEnabled(true, WidgetMovies);
}

/**
 * @brief Sets the state of the main groupbox to disabled
 */
void MovieWidget::setDisabledTrue()
{
    qDebug() << "Entered";
    ui->groupBox_3->setDisabled(true);
    emit setActionSaveEnabled(false, WidgetMovies);
    emit setActionSearchEnabled(false, WidgetMovies);
}

/**
 * @brief Sets the current movie, tells the movie to load data and images and updates widgets contents
 * @param movie Current movie
 */
void MovieWidget::setMovie(Movie *movie)
{
    qDebug() << "Entered, movie=" << movie->name();
    movie->controller()->loadData(Manager::instance()->mediaCenterInterface());
    if (!movie->streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails()) {
        movie->controller()->loadStreamDetailsFromFile();
        if (movie->streamDetailsLoaded() && movie->streamDetails()->videoDetails().value("durationinseconds").toInt() != 0)
            movie->setRuntime(qFloor(movie->streamDetails()->videoDetails().value("durationinseconds").toInt()/60));
    }
    m_movie = movie;
    updateMovieInfo();

    connect(m_movie->controller(), SIGNAL(sigInfoLoadDone(Movie*)), this, SLOT(onInfoLoadDone(Movie*)), Qt::UniqueConnection);
    connect(m_movie->controller(), SIGNAL(sigLoadDone(Movie*)), this, SLOT(onLoadDone(Movie*)), Qt::UniqueConnection);
    connect(m_movie->controller(), SIGNAL(sigDownloadProgress(Movie*,int, int)), this, SLOT(onDownloadProgress(Movie*,int,int)), Qt::UniqueConnection);
    connect(m_movie->controller(), SIGNAL(sigLoadingImages(Movie*,QList<int>)), this, SLOT(onLoadingImages(Movie*,QList<int>)), Qt::UniqueConnection);
    connect(m_movie->controller(), SIGNAL(sigLoadImagesStarted(Movie*)), this, SLOT(onLoadImagesStarted(Movie*)), Qt::UniqueConnection);
    connect(m_movie->controller(), SIGNAL(sigImage(Movie*,int,QByteArray)), this, SLOT(onSetImage(Movie*,int,QByteArray)), Qt::UniqueConnection);

    ui->btnAddExtraFanart->setEnabled(movie->inSeparateFolder());
    ui->labelSepFoldersWarning->setVisible(!movie->inSeparateFolder());

    if (movie->controller()->downloadsInProgress())
        setDisabledTrue();
    else
        setEnabledTrue();
}

/**
 * @brief Shows the search widget
 */
void MovieWidget::startScraperSearch()
{
    qDebug() << "Entered";
    if (m_movie == 0) {
        qDebug() << "My movie is invalid";
        return;
    }
    emit setActionSearchEnabled(false, WidgetMovies);
    emit setActionSaveEnabled(false, WidgetMovies);
    MovieSearch::instance()->exec(m_movie->name(), m_movie->id(), m_movie->tmdbId());
    if (MovieSearch::instance()->result() == QDialog::Accepted) {
        setDisabledTrue();
        QMap<ScraperInterface*, QString> ids;
        QList<int> infosToLoad;
        if (MovieSearch::instance()->scraperId() == "custom-movie") {
            ids = MovieSearch::instance()->customScraperIds();
            infosToLoad = Settings::instance()->scraperInfos(WidgetMovies, "custom-movie");
        } else {
            ids.insert(0, MovieSearch::instance()->scraperMovieId());
            infosToLoad = MovieSearch::instance()->infosToLoad();
        }
        m_movie->controller()->loadData(ids, Manager::instance()->scraper(MovieSearch::instance()->scraperId()),
                                        infosToLoad);
    } else {
        emit setActionSearchEnabled(true, WidgetMovies);
        emit setActionSaveEnabled(true, WidgetMovies);
    }
}

void MovieWidget::onInfoLoadDone(Movie *movie)
{
    if (m_movie == 0)
        return;
    if (m_movie == movie) {
        updateMovieInfo();
        ui->buttonRevert->setVisible(true);
        emit setActionSaveEnabled(false, WidgetMovies);
    }
}

void MovieWidget::onLoadDone(Movie *movie)
{
    emit actorDownloadFinished(Constants::MovieProgressMessageId+movie->movieId());
    if (m_movie == 0 || m_movie != movie)
        return;
    setEnabledTrue();
    ui->fanarts->setLoading(false);
}

void MovieWidget::onLoadImagesStarted(Movie *movie)
{
    emit actorDownloadStarted(tr("Downloading images..."), Constants::MovieProgressMessageId+movie->movieId());
}

void MovieWidget::onLoadingImages(Movie *movie, QList<int> imageTypes)
{
    if (movie != m_movie)
        return;

    foreach (const int &imageType, imageTypes) {
        foreach (ClosableImage *cImage, ui->artStackedWidget->findChildren<ClosableImage*>()) {
            if (cImage->imageType() == imageType)
                cImage->setLoading(true);
        }
    }

    if (imageTypes.contains(ImageType::MovieExtraFanart))
        ui->fanarts->setLoading(true);
    ui->groupBox_3->update();
}

void MovieWidget::onSetImage(Movie *movie, int type, QByteArray data)
{
    if (movie != m_movie)
        return;

    if (type == ImageType::MovieExtraFanart) {
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

void MovieWidget::onDownloadProgress(Movie *movie, int current, int maximum)
{
    emit actorDownloadProgress(maximum-current, maximum, Constants::MovieProgressMessageId+movie->movieId());
}

/**
 * @brief Updates the contents of the widget with the current movie infos
 */
void MovieWidget::updateMovieInfo()
{
    if (m_movie == 0)
        return;

    ui->rating->blockSignals(true);
    ui->votes->blockSignals(true);
    ui->top250->blockSignals(true);
    ui->runtime->blockSignals(true);
    ui->playcount->blockSignals(true);
    ui->set->blockSignals(true);
    ui->certification->blockSignals(true);
    ui->released->blockSignals(true);
    ui->lastPlayed->blockSignals(true);
    ui->overview->blockSignals(true);
    ui->outline->blockSignals(true);
    ui->actors->blockSignals(true);
    ui->subtitles->blockSignals(true);

    clear();

    ui->files->setText(m_movie->files().join(", "));
    ui->files->setToolTip(m_movie->files().join("\n"));
    ui->imdbId->setText(m_movie->id());
    ui->name->setText(m_movie->name());
    ui->movieName->setText(m_movie->name());
    ui->originalName->setText(m_movie->originalName());
    ui->sortTitle->setText(m_movie->sortTitle());
    ui->tagline->setText(m_movie->tagline());
    ui->rating->setValue(m_movie->rating());
    ui->votes->setValue(m_movie->votes());
    ui->top250->setValue(m_movie->top250());
    ui->released->setDate(m_movie->released());
    ui->runtime->setValue(m_movie->runtime());
    ui->trailer->setText(m_movie->trailer().toString());
    ui->playcount->setValue(m_movie->playcount());
    ui->lastPlayed->setDateTime(m_movie->lastPlayed());
    ui->overview->setPlainText(m_movie->overview());
    ui->outline->setPlainText(m_movie->outline());
    ui->badgeWatched->setActive(m_movie->watched());
    ui->writer->setText(m_movie->writer());
    ui->director->setText(m_movie->director());

    QStringList certifications;
    QStringList sets;
    sets.append("");
    certifications.append("");
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (!sets.contains(movie->set()) && !movie->set().isEmpty())
            sets.append(movie->set());
        if (!certifications.contains(movie->certification()) && !movie->certification().isEmpty())
            certifications.append(movie->certification());
    }
    qSort(sets.begin(), sets.end(), LocaleStringCompare());
    qSort(certifications.begin(), certifications.end(), LocaleStringCompare());
    ui->certification->addItems(certifications);
    ui->set->addItems(sets);

    ui->certification->setCurrentIndex(certifications.indexOf(m_movie->certification()));
    ui->set->setCurrentIndex(sets.indexOf(m_movie->set()));

    ui->set->blockSignals(false);
    ui->certification->blockSignals(false);

    ui->actors->blockSignals(true);
    foreach (Actor *actor, m_movie->actorsPointer()) {
        int row = ui->actors->rowCount();
        ui->actors->insertRow(row);
        ui->actors->setItem(row, 0, new QTableWidgetItem(actor->name));
        ui->actors->setItem(row, 1, new QTableWidgetItem(actor->role));
        ui->actors->item(row, 0)->setData(Qt::UserRole, actor->thumb);
        ui->actors->item(row, 1)->setData(Qt::UserRole, QVariant::fromValue(actor));
    }
    ui->actors->blockSignals(false);

    ui->subtitles->blockSignals(true);
    foreach (Subtitle *subtitle, m_movie->subtitles()) {
        int row = ui->subtitles->rowCount();
        ui->subtitles->insertRow(row);

        QTableWidgetItem *item0 = new QTableWidgetItem(subtitle->files().join(", "));
        item0->setFlags(Qt::ItemIsSelectable);
        item0->setData(Qt::UserRole, QVariant::fromValue(subtitle));
        ui->subtitles->setItem(row, 0, item0);

        QTableWidgetItem *item1 = new QTableWidgetItem(subtitle->language());
        item1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        ui->subtitles->setItem(row, 1, item1);

        QTableWidgetItem *item2 = new QTableWidgetItem;
        item2->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        item2->setCheckState(subtitle->forced() ? Qt::Checked : Qt::Unchecked);
        ui->subtitles->setItem(row, 2, item2);
    }

    ui->subtitles->blockSignals(false);

    QStringList genres;
    QStringList tags;
    QStringList countries;
    QStringList studios;
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        genres << movie->genres();
        tags << movie->tags();
        countries << movie->countries();
        studios << movie->studios();
    }
    studios.removeDuplicates();
    ui->genreCloud->setTags(genres, m_movie->genres());
    ui->tagCloud->setTags(tags, m_movie->tags());
    ui->countryCloud->setTags(countries, m_movie->countries());
    ui->studioCloud->setTags(m_movie->studios(), m_movie->studios());
    QCompleter *completer = new QCompleter(studios, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->studioCloud->setCompleter(completer);

    // Streamdetails
    updateStreamDetails();
    ui->videoAspectRatio->setEnabled(m_movie->streamDetailsLoaded());
    ui->videoCodec->setEnabled(m_movie->streamDetailsLoaded());
    ui->videoDuration->setEnabled(m_movie->streamDetailsLoaded());
    ui->videoHeight->setEnabled(m_movie->streamDetailsLoaded());
    ui->videoWidth->setEnabled(m_movie->streamDetailsLoaded());
    ui->videoScantype->setEnabled(m_movie->streamDetailsLoaded());
    ui->stereoMode->setEnabled(m_movie->streamDetailsLoaded());

    updateImages(QList<int>() << ImageType::MoviePoster << ImageType::MovieBackdrop << ImageType::MovieLogo << ImageType::MovieCdArt << ImageType::MovieClearArt << ImageType::MovieBanner << ImageType::MovieThumb);

    ui->fanarts->setImages(m_movie->extraFanarts(Manager::instance()->mediaCenterInterface()));

    ui->rating->blockSignals(false);
    ui->votes->blockSignals(false);
    ui->top250->blockSignals(false);
    ui->runtime->blockSignals(false);
    ui->playcount->blockSignals(false);
    ui->released->blockSignals(false);
    ui->lastPlayed->blockSignals(false);
    ui->overview->blockSignals(false);
    ui->outline->blockSignals(false);
    ui->actors->blockSignals(false);

    emit setActionSaveEnabled(true, WidgetMovies);

    ui->buttonRevert->setVisible(m_movie->hasChanged());
    ui->localTrailer->setVisible(m_movie->hasLocalTrailer());
}

void MovieWidget::updateImages(QList<int> images)
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

void MovieWidget::updateImage(const int &imageType, ClosableImage *image)
{
    if (!m_movie->image(imageType).isNull()) {
        image->setImage(m_movie->image(imageType));
    } else if (!m_movie->imagesToRemove().contains(imageType) && m_movie->hasImage(imageType)) {
        QString imgFileName = Manager::instance()->mediaCenterInterface()->imageFileName(m_movie, imageType);
        if (!imgFileName.isEmpty())
            image->setImage(imgFileName);
    }
}

/**
 * @brief Fills the widget with streamdetails
 * @param reloadFromFile If true forces a reload of streamdetails from the file
 */
void MovieWidget::updateStreamDetails(bool reloadFromFile)
{
    ui->videoAspectRatio->blockSignals(true);
    ui->videoDuration->blockSignals(true);
    ui->videoWidth->blockSignals(true);
    ui->videoHeight->blockSignals(true);
    ui->stereoMode->blockSignals(true);

    if (reloadFromFile)
        m_movie->controller()->loadStreamDetailsFromFile();

    StreamDetails *streamDetails = m_movie->streamDetails();
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
        label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
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
        label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        QFont f = ui->labelStreamDetailsAudio->font();
        f.setBold(true);
        label->setFont(f);
        ui->streamDetails->addWidget(label, 8+audioTracks, 0);
        m_streamDetailsWidgets << label;

        for (int i=0, n=streamDetails->subtitleDetails().count() ; i<n ; ++i) {
            QLabel *label = new QLabel(tr("Track %1").arg(i+1));
            label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
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
void MovieWidget::onReloadStreamDetails()
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

void MovieWidget::onDownloadTrailer()
{
    if (!m_movie)
        return;
    TrailerDialog::instance()->exec(m_movie);
    ui->localTrailer->setVisible(m_movie->hasLocalTrailer());
}

void MovieWidget::onPlayLocalTrailer()
{
    if (!m_movie && !m_movie->hasLocalTrailer())
        return;

    QDesktopServices::openUrl(QUrl::fromLocalFile(m_movie->localTrailerFileName()));
}

/**
 * @brief Saves movie information
 */
void MovieWidget::saveInformation()
{
    qDebug() << "Entered";
    setDisabledTrue();

    QList<Movie*> movies = FilesWidget::instance()->selectedMovies();
    if (movies.isEmpty())
        movies.append(m_movie);

    m_savingWidget->show();
    if (movies.count() > 0) {
        int counter = 0;
        int moviesToSave = movies.count();

        NotificationBox::instance()->showProgressBar(tr("Saving movies..."), Constants::MovieWidgetProgressMessageId);
        NotificationBox::instance()->progressBarProgress(0, moviesToSave, Constants::MovieWidgetProgressMessageId);
        qApp->processEvents();
        foreach (Movie *movie, movies) {
            counter++;
            if (movie->hasChanged()) {
                NotificationBox::instance()->progressBarProgress(counter, moviesToSave, Constants::MovieWidgetProgressMessageId);
                qApp->processEvents();
                movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
                movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
                if (m_movie == movie)
                    updateMovieInfo();
            }
        }
        NotificationBox::instance()->hideProgressBar(Constants::MovieWidgetProgressMessageId);
        NotificationBox::instance()->showMessage(tr("Movies Saved"));
    } else {
        int id = NotificationBox::instance()->showMessage(tr("Saving movie..."));
        m_movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
        m_movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
        updateMovieInfo();
        NotificationBox::instance()->removeMessage(id);
        NotificationBox::instance()->showMessage(tr("<b>\"%1\"</b> Saved").arg(m_movie->name()));
    }
    setEnabledTrue();
    m_savingWidget->hide();
    ui->buttonRevert->setVisible(false);
}

/**
 * @brief Saves all changed movies
 */
void MovieWidget::saveAll()
{
    qDebug() << "Entered";
    setDisabledTrue();
    m_savingWidget->show();

    int counter = 0;
    int moviesToSave = 0;
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->hasChanged())
            moviesToSave++;
    }

    NotificationBox::instance()->showProgressBar(tr("Saving movies..."), Constants::MovieWidgetProgressMessageId);
    NotificationBox::instance()->progressBarProgress(0, moviesToSave, Constants::MovieWidgetProgressMessageId);
    qApp->processEvents();
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->hasChanged()) {
            NotificationBox::instance()->progressBarProgress(counter++, moviesToSave, Constants::MovieWidgetProgressMessageId);
            qApp->processEvents();
            movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
            movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
            if (m_movie == movie)
                updateMovieInfo();
        }
    }
    setEnabledTrue();
    m_savingWidget->hide();
    NotificationBox::instance()->hideProgressBar(Constants::MovieWidgetProgressMessageId);
    NotificationBox::instance()->showMessage(tr("All Movies Saved"));
    ui->buttonRevert->setVisible(false);
}

/**
 * @brief Revert changes for current movie
 */
void MovieWidget::onRevertChanges()
{
    m_movie->clearImages();
    m_movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
    updateMovieInfo();
}

/*** add/remove/edit Actors, Genres, Countries and Studios ***/

/**
 * @brief Adds an actor
 */
void MovieWidget::addActor()
{
    Actor a;
    a.name = tr("Unknown Actor");
    a.role = tr("Unknown Role");
    m_movie->addActor(a);

    Actor *actor = m_movie->actorsPointer().last();

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
void MovieWidget::removeActor()
{
    int row = ui->actors->currentRow();
    if (row < 0 || row >= ui->actors->rowCount() || !ui->actors->currentItem()->isSelected())
        return;

    Actor *actor = ui->actors->item(row, 1)->data(Qt::UserRole).value<Actor*>();
    m_movie->removeActor(actor);
    ui->actors->blockSignals(true);
    ui->actors->removeRow(row);
    ui->actors->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Stores changed values for an actor
 * @param item Edited item
 */
void MovieWidget::onActorEdited(QTableWidgetItem *item)
{
    Actor *actor = ui->actors->item(item->row(), 1)->data(Qt::UserRole).value<Actor*>();
    if (item->column() == 0)
        actor->name = item->text();
    else if (item->column() == 1)
        actor->role = item->text();
    m_movie->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onSubtitleEdited(QTableWidgetItem *item)
{
    Subtitle *subtitle = ui->subtitles->item(item->row(), 0)->data(Qt::UserRole).value<Subtitle*>();
    if (!subtitle)
        return;
    if (item->column() == 1)
        subtitle->setLanguage(item->text());
    else if (item->column() == 2)
        subtitle->setForced(item->checkState() == Qt::Checked);
    m_movie->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Adds a studio
 */
void MovieWidget::addStudio(QString studio)
{
    if (!m_movie)
        return;
    m_movie->addStudio(studio);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a studio
 */
void MovieWidget::removeStudio(QString studio)
{
    if (!m_movie)
        return;
    m_movie->removeStudio(studio);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Adds a country
 */
void MovieWidget::addCountry(QString country)
{
    if (!m_movie)
        return;
    m_movie->addCountry(country);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a country
 */
void MovieWidget::removeCountry(QString country)
{
    if (!m_movie)
        return;
    m_movie->removeCountry(country);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Shows the image of the selected actor
 */
void MovieWidget::onActorChanged()
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
        QPixmap p = QPixmap::fromImage(QImage::fromData(actor->image));
        ui->actorResolution->setText(QString("%1 x %2").arg(p.width()).arg(p.height()));
        p = p.scaled(QSize(120, 180) * Helper::instance()->devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        Helper::instance()->setDevicePixelRatio(p, Helper::instance()->devicePixelRatio(this));
        ui->actor->setPixmap(p);
    } else if (!Manager::instance()->mediaCenterInterface()->actorImageName(m_movie, *actor).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->actorImageName(m_movie, *actor));
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
void MovieWidget::onChangeActorImage()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount() ||
        ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        return;
    }

    static QString lastPath = QDir::homePath();

    QString fileName = QFileDialog::getOpenFileName(parentWidget(), tr("Choose Image"), lastPath, tr("Images (*.jpg *.jpeg)"));
    if (!fileName.isNull()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            Actor *actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
            actor->image = file.readAll();
            actor->imageHasChanged = true;
            onActorChanged();
            m_movie->setChanged(true);
            file.close();
            QFileInfo fi(fileName);
            lastPath = fi.absolutePath();
        }
    }
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Shows the first page with movie art
 */
void MovieWidget::onArtPageOne()
{
    ui->artStackedWidget->slideInIdx(0);
    ui->buttonArtPageTwo->setChecked(false);
    ui->buttonArtPageOne->setChecked(true);
}

/**
 * @brief Shows the second page with movie art
 */
void MovieWidget::onArtPageTwo()
{
    ui->artStackedWidget->slideInIdx(1);
    ui->buttonArtPageOne->setChecked(false);
    ui->buttonArtPageTwo->setChecked(true);
}

/*** Pass GUI events to movie object ***/

void MovieWidget::addGenre(QString genre)
{
    if (!m_movie)
        return;
    m_movie->addGenre(genre);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::removeGenre(QString genre)
{
    if (!m_movie)
        return;
    m_movie->removeGenre(genre);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::addTag(QString tag)
{
    if (!m_movie)
        return;
    m_movie->addTag(tag);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::removeTag(QString tag)
{
    if (!m_movie)
        return;
    m_movie->removeTag(tag);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the name has changed
 */
void MovieWidget::onNameChange(QString text)
{
    if (!m_movie)
        return;
    m_movie->setName(text);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onImdbIdChange(QString text)
{
    if (!m_movie)
        return;
    m_movie->setId(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the original name has changed
 */
void MovieWidget::onOriginalNameChange(QString text)
{
    if (!m_movie)
        return;
    m_movie->setOriginalName(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the sorttitle has changed
 */
void MovieWidget::onSortTitleChange(QString text)
{
    if (!m_movie)
        return;
    m_movie->setSortTitle(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the set has changed
 */
void MovieWidget::onSetChange(QString text)
{
    if (!m_movie)
        return;
    m_movie->setSet(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the tagline has changed
 */
void MovieWidget::onTaglineChange(QString text)
{
    if (!m_movie)
        return;
    m_movie->setTagline(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the rating has changed
 */
void MovieWidget::onRatingChange(double value)
{
    if (!m_movie)
        return;
    m_movie->setRating(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the votes has changed
 */
void MovieWidget::onVotesChange(int value)
{
    if (!m_movie)
        return;
    m_movie->setVotes(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the top 250 position has changed
 */
void MovieWidget::onTop250Change(int value)
{
    if (!m_movie)
        return;
    m_movie->setTop250(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the release date has changed
 */
void MovieWidget::onReleasedChange(QDate date)
{
    if (!m_movie)
        return;
    m_movie->setReleased(date);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the runtime has changed
 */
void MovieWidget::onRuntimeChange(int value)
{
    if (!m_movie)
        return;
    m_movie->setRuntime(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the certification has changed
 */
void MovieWidget::onCertificationChange(QString text)
{
    if (!m_movie)
        return;
    m_movie->setCertification(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the writer has changed
 */
void MovieWidget::onWriterChange(QString text)
{
    if (!m_movie)
        return;
    m_movie->setWriter(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the director has changed
 */
void MovieWidget::onDirectorChange(QString text)
{
    if (!m_movie)
        return;
    m_movie->setDirector(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the trailer has changed
 */
void MovieWidget::onTrailerChange(QString text)
{
    if (!m_movie)
        return;
    m_movie->setTrailer(text);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onWatchedClicked()
{
    if (!m_movie)
        return;

    bool active = !ui->badgeWatched->isActive();
    ui->badgeWatched->setActive(active);
    m_movie->setWatched(active);
    if (active) {
        if (m_movie->playcount() < 1)
            ui->playcount->setValue(1);
        if (!m_movie->lastPlayed().isValid())
            ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
    }
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the play count has changed
 */
void MovieWidget::onPlayCountChange(int value)
{
    if (!m_movie)
        return;
    m_movie->setPlayCount(value);
    ui->badgeWatched->setActive(value > 0);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the last watched date has changed
 */
void MovieWidget::onLastWatchedChange(QDateTime dateTime)
{
    if (!m_movie)
        return;
    m_movie->setLastPlayed(dateTime);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the overview has changed
 */
void MovieWidget::onOverviewChange()
{
    if (!m_movie)
        return;
    m_movie->setOverview(ui->overview->toPlainText());
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the outline has changed
 */
void MovieWidget::onOutlineChange()
{
    if (!m_movie)
        return;
    m_movie->setOutline(ui->outline->toPlainText());
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Updates all stream details for this movie with values from the widget
 */
void MovieWidget::onStreamDetailsEdited()
{
    StreamDetails *details = m_movie->streamDetails();
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

    m_movie->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onRemoveExtraFanart(const QByteArray &image)
{
    if (!m_movie)
        return;
    m_movie->removeExtraFanart(image);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onRemoveExtraFanart(const QString &file)
{
    if (!m_movie)
        return;
    m_movie->removeExtraFanart(file);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onAddExtraFanart()
{
    if (!m_movie)
        return;

    ImageDialog::instance()->setImageType(ImageType::MovieExtraFanart);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMultiSelection(true);
    ImageDialog::instance()->setMovie(m_movie);
    ImageDialog::instance()->setDownloads(m_movie->backdrops());
    ImageDialog::instance()->exec(ImageType::MovieBackdrop);

    if (ImageDialog::instance()->result() == QDialog::Accepted && !ImageDialog::instance()->imageUrls().isEmpty()) {
        ui->fanarts->setLoading(true);
        emit setActionSaveEnabled(false, WidgetMovies);
        m_movie->controller()->loadImages(ImageType::MovieExtraFanart, ImageDialog::instance()->imageUrls());
        ui->buttonRevert->setVisible(true);
    }
}

void MovieWidget::onExtraFanartDropped(QUrl imageUrl)
{
    if (!m_movie)
        return;
    ui->fanarts->setLoading(true);
    emit setActionSaveEnabled(false, WidgetMovies);
    m_movie->controller()->loadImages(ImageType::MovieExtraFanart, QList<QUrl>() << imageUrl);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onInsertYoutubeLink()
{
    if (Settings::instance()->useYoutubePluginUrls())
        ui->trailer->setText("plugin://plugin.video.youtube/?action=play_video&videoid=");
    else
        ui->trailer->setText("http://www.youtube.com/watch?v=");
    ui->trailer->setFocus();
}

void MovieWidget::onChooseImage()
{
    if (m_movie == 0)
        return;

    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    ImageDialog::instance()->setImageType(image->imageType());
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMovie(m_movie);
    if (image->imageType() == ImageType::MoviePoster)
        ImageDialog::instance()->setDownloads(m_movie->posters());
    else if (image->imageType() == ImageType::MovieBackdrop)
        ImageDialog::instance()->setDownloads(m_movie->backdrops());
    else if (image->imageType() == ImageType::MovieCdArt && !m_movie->discArts().isEmpty())
        ImageDialog::instance()->setDownloads(m_movie->discArts());
    else
        ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(image->imageType());

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetMovies);
        m_movie->controller()->loadImage(image->imageType(), ImageDialog::instance()->imageUrl());
        ui->buttonRevert->setVisible(true);
    }
}

void MovieWidget::onImageDropped(int imageType, QUrl imageUrl)
{
    if (!m_movie)
        return;
    emit setActionSaveEnabled(false, WidgetMovies);
    m_movie->controller()->loadImage(imageType, imageUrl);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onDeleteImage()
{
    if (m_movie == 0)
        return;

    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    m_movie->removeImage(image->imageType());
    updateImages(QList<int>() << image->imageType());
    ui->buttonRevert->setVisible(true);
}
