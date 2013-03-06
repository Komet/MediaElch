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
#include "globals/Globals.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "main/MessageBox.h"
#include "globals/ImageDialog.h"
#include "globals/ComboDelegate.h"
#include "movies/MovieSearch.h"
#include "globals/TrailerDialog.h"

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

#if QT_VERSION >= 0x050000
    ui->actors->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    ui->actors->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    ui->buttonPreviewPoster->setEnabled(false);
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewLogo->setEnabled(false);
    ui->buttonPreviewClearArt->setEnabled(false);
    ui->buttonPreviewCdArt->setEnabled(false);
    ui->artStackedWidget->setAnimation(QEasingCurve::OutCubic);
    ui->artStackedWidget->setSpeed(300);
    ui->localTrailer->setBadgeType(Badge::LabelSuccess);
    ui->localTrailer->setVisible(false);

    QFont font = ui->movieName->font();
    font.setPointSize(font.pointSize()+4);
    ui->movieName->setFont(font);

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
    ui->actorResolution->setFont(font);

    m_movie = 0;

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

    connect(ui->poster, SIGNAL(clicked()), this, SLOT(chooseMoviePoster()));
    connect(ui->backdrop, SIGNAL(clicked()), this, SLOT(chooseMovieBackdrop()));
    connect(ui->logo, SIGNAL(clicked()), this, SLOT(chooseMovieLogo()));
    connect(ui->clearArt, SIGNAL(clicked()), this, SLOT(chooseMovieClearArt()));
    connect(ui->cdArt, SIGNAL(clicked()), this, SLOT(chooseMovieCdArt()));
    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(movieNameChanged(QString)));
    connect(ui->buttonAddActor, SIGNAL(clicked()), this, SLOT(addActor()));
    connect(ui->buttonRemoveActor, SIGNAL(clicked()), this, SLOT(removeActor()));
    connect(ui->actors, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onActorEdited(QTableWidgetItem*)));
    connect(ui->actors, SIGNAL(itemSelectionChanged()), this, SLOT(onActorChanged()));
    connect(ui->buttonPreviewPoster, SIGNAL(clicked()), this, SLOT(onPreviewPoster()));
    connect(ui->buttonPreviewBackdrop, SIGNAL(clicked()), this, SLOT(onPreviewBackdrop()));
    connect(ui->buttonPreviewLogo, SIGNAL(clicked()), this, SLOT(onPreviewLogo()));
    connect(ui->buttonPreviewClearArt, SIGNAL(clicked()), this, SLOT(onPreviewClearArt()));
    connect(ui->buttonPreviewCdArt, SIGNAL(clicked()), this, SLOT(onPreviewCdArt()));
    connect(ui->actor, SIGNAL(clicked()), this, SLOT(onChangeActorImage()));
    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));
    connect(ui->buttonReloadStreamDetails, SIGNAL(clicked()), this, SLOT(onReloadStreamDetails()));
    connect(ui->buttonDownloadTrailer, SIGNAL(clicked()), this, SLOT(onDownloadTrailer()));

    connect(ui->fanarts, SIGNAL(sigRemoveImage(QByteArray)), this, SLOT(onRemoveExtraFanart(QByteArray)));
    connect(ui->fanarts, SIGNAL(sigRemoveImage(QString)), this, SLOT(onRemoveExtraFanart(QString)));
    connect(ui->btnAddExtraFanart, SIGNAL(clicked()), this, SLOT(onAddExtraFanart()));

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();

    setDisabledTrue();
    clear();

    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    // Connect GUI change events to movie object
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
    connect(ui->watched, SIGNAL(stateChanged(int)), this, SLOT(onWatchedChange(int)));
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

    QPixmap revert(":/img/reload.png");
    p.begin(&revert);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(revert.rect(), QColor(0, 0, 0, 200));
    p.end();
    ui->buttonRevert->setIcon(QIcon(revert));
    ui->buttonRevert->setVisible(false);
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
    QTimer::singleShot(0, this, SLOT(updateBackgroundImage()));
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

    ui->poster->setPixmap(QPixmap(":/img/poster.png"));
    ui->backdrop->setPixmap(QPixmap(":/img/missing_art.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->logo->setPixmap(QPixmap(":/img/missing_art.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->clearArt->setPixmap(QPixmap(":/img/missing_art.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->cdArt->setPixmap(QPixmap(":/img/missing_art.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->actor->setPixmap(QPixmap(":/img/actor.png"));
    ui->posterResolution->setText("");
    ui->backdropResolution->setText("");
    ui->logoResolution->setText("");
    ui->clearArtResolution->setText("");
    ui->cdArtResolution->setText("");
    ui->actorResolution->setText("");
    ui->genreCloud->clear();
    ui->countryCloud->clear();
    ui->studioCloud->clear();
    ui->buttonRevert->setVisible(false);
    ui->localTrailer->setVisible(false);
    ui->fanarts->clear();

    m_currentBackdrop = QImage();
    QTimer::singleShot(0, this, SLOT(updateBackgroundImage()));
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
    MovieSearch::instance()->exec(m_movie->name());
    if (MovieSearch::instance()->result() == QDialog::Accepted) {
        setDisabledTrue();
        m_movie->controller()->loadData(MovieSearch::instance()->scraperId(), Manager::instance()->scrapers().at(MovieSearch::instance()->scraperNo()),
                                        MovieSearch::instance()->infosToLoad());
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

    if (imageTypes.contains(TypePoster)) {
        ui->poster->setPixmap(QPixmap());
        ui->poster->setMovie(m_loadingMovie);
        ui->buttonPreviewPoster->setEnabled(false);
    }
    if (imageTypes.contains(TypeBackdrop)) {
        ui->backdrop->setPixmap(QPixmap());
        ui->backdrop->setMovie(m_loadingMovie);
        ui->buttonPreviewBackdrop->setEnabled(false);
    }
    if (imageTypes.contains(TypeClearArt)) {
        ui->clearArt->setPixmap(QPixmap());
        ui->clearArt->setMovie(m_loadingMovie);
        ui->buttonPreviewClearArt->setEnabled(false);
    }
    if (imageTypes.contains(TypeCdArt)) {
        ui->cdArt->setPixmap(QPixmap());
        ui->cdArt->setMovie(m_loadingMovie);
        ui->buttonPreviewCdArt->setEnabled(false);
    }
    if (imageTypes.contains(TypeLogo)) {
        ui->logo->setPixmap(QPixmap());
        ui->logo->setMovie(m_loadingMovie);
        ui->buttonPreviewLogo->setEnabled(false);
    }
    if (imageTypes.contains(TypeExtraFanart)) {
        ui->fanarts->setLoading(true);
    }
    ui->groupBox_3->update();
}

void MovieWidget::onSetImage(Movie *movie, int type, QByteArray data)
{
    if (movie != m_movie)
        return;

    QImage image = QImage::fromData(data);
    switch (type) {
    case TypePoster:
        ui->poster->setPixmap(QPixmap::fromImage(image).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(image.width()).arg(image.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = image;
        break;
    case TypeBackdrop:
        ui->backdrop->setPixmap(QPixmap::fromImage(image).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(image.width()).arg(image.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = image;
        updateBackgroundImage();
        break;
    case TypeClearArt:
        ui->clearArt->setPixmap(QPixmap::fromImage(image).scaled(200, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->clearArtResolution->setText(QString("%1x%2").arg(image.width()).arg(image.height()));
        ui->buttonPreviewClearArt->setEnabled(true);
        m_currentClearArt = image;
        break;
    case TypeCdArt:
        ui->cdArt->setPixmap(QPixmap::fromImage(image).scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->cdArtResolution->setText(QString("%1x%2").arg(image.width()).arg(image.height()));
        ui->buttonPreviewCdArt->setEnabled(true);
        m_currentCdArt = image;
        break;
    case TypeLogo:
        ui->logo->setPixmap(QPixmap::fromImage(image).scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->logoResolution->setText(QString("%1x%2").arg(image.width()).arg(image.height()));
        ui->buttonPreviewLogo->setEnabled(true);
        m_currentLogo = image;
        break;
    case TypeExtraFanart:
        ui->fanarts->addImage(data);
        break;
    default:
        break;
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
    ui->watched->blockSignals(true);
    ui->released->blockSignals(true);
    ui->lastPlayed->blockSignals(true);
    ui->overview->blockSignals(true);
    ui->outline->blockSignals(true);
    ui->actors->blockSignals(true);

    clear();

    ui->files->setText(m_movie->files().join(", "));
    ui->files->setToolTip(m_movie->files().join("\n"));
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
    ui->watched->setChecked(m_movie->watched());
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
    sets.sort();
    certifications.sort();
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

    // Poster
    if (!m_movie->posterImage().isNull()) {
        QImage img = QImage::fromData(m_movie->posterImage());
        ui->poster->setPixmap(QPixmap::fromImage(img).scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = img;
    } else if (!Manager::instance()->mediaCenterInterface()->posterImageName(m_movie).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->posterImageName(m_movie));
        ui->poster->setPixmap(p.scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = p.toImage();
    } else {
        ui->poster->setPixmap(QPixmap(":/img/poster.png"));
        ui->posterResolution->setText("");
        ui->buttonPreviewPoster->setEnabled(false);
    }

    // Backdrop
    if (!m_movie->backdropImage().isNull()) {
        QImage img = QImage::fromData(m_movie->backdropImage());
        ui->backdrop->setPixmap(QPixmap::fromImage(img).scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = img;
        QTimer::singleShot(0, this, SLOT(updateBackgroundImage()));
    } else if (!Manager::instance()->mediaCenterInterface()->backdropImageName(m_movie).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->backdropImageName(m_movie));
        ui->backdrop->setPixmap(p.scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = p.toImage();
        QTimer::singleShot(0, this, SLOT(updateBackgroundImage()));
    } else {
        ui->backdrop->setPixmap(QPixmap(":/img/missing_art.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText("");
        ui->buttonPreviewBackdrop->setEnabled(false);
    }

    // Logo
    if (!m_movie->logoImage().isNull()) {
        QImage img = QImage::fromData(m_movie->logoImage());
        ui->logo->setPixmap(QPixmap::fromImage(img).scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->logoResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewLogo->setEnabled(true);
        m_currentLogo = img;
    } else if (!Manager::instance()->mediaCenterInterface()->logoImageName(m_movie).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->logoImageName(m_movie));
        ui->logo->setPixmap(p.scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->logoResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewLogo->setEnabled(true);
        m_currentLogo = p.toImage();
    } else {
        ui->logo->setPixmap(QPixmap(":/img/missing_art.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->logoResolution->setText("");
        ui->buttonPreviewLogo->setEnabled(false);
    }

    // Clear art
    if (!m_movie->clearArtImage().isNull()) {
        QImage img = QImage::fromData(m_movie->clearArtImage());
        ui->clearArt->setPixmap(QPixmap::fromImage(img).scaled(200, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->clearArtResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewClearArt->setEnabled(true);
        m_currentClearArt = img;
    } else if (!Manager::instance()->mediaCenterInterface()->clearArtImageName(m_movie).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->clearArtImageName(m_movie));
        ui->clearArt->setPixmap(p.scaled(200, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->clearArtResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewClearArt->setEnabled(true);
        m_currentClearArt = p.toImage();
    } else {
        ui->clearArt->setPixmap(QPixmap(":/img/missing_art.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->clearArtResolution->setText("");
        ui->buttonPreviewClearArt->setEnabled(false);
    }

    // CD Art
    if (!m_movie->cdArtImage().isNull()) {
        QImage img = QImage::fromData(m_movie->cdArtImage());
        ui->cdArt->setPixmap(QPixmap::fromImage(img).scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->cdArtResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
        ui->buttonPreviewCdArt->setEnabled(true);
        m_currentCdArt = img;
    } else if (!Manager::instance()->mediaCenterInterface()->cdArtImageName(m_movie).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->cdArtImageName(m_movie));
        ui->cdArt->setPixmap(p.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->cdArtResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewCdArt->setEnabled(true);
        m_currentCdArt = p.toImage();
    } else {
        ui->cdArt->setPixmap(QPixmap(":/img/missing_art.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->cdArtResolution->setText("");
        ui->buttonPreviewCdArt->setEnabled(false);
    }

    ui->fanarts->setImages(m_movie->extraFanarts(Manager::instance()->mediaCenterInterface()));

    ui->rating->blockSignals(false);
    ui->votes->blockSignals(false);
    ui->top250->blockSignals(false);
    ui->runtime->blockSignals(false);
    ui->playcount->blockSignals(false);
    ui->watched->blockSignals(false);
    ui->released->blockSignals(false);
    ui->lastPlayed->blockSignals(false);
    ui->overview->blockSignals(false);
    ui->outline->blockSignals(false);
    ui->actors->blockSignals(false);

    emit setActionSaveEnabled(true, WidgetMovies);

    ui->buttonRevert->setVisible(m_movie->hasChanged());
    ui->localTrailer->setVisible(m_movie->hasLocalTrailer());
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

    if (reloadFromFile)
        m_movie->controller()->loadStreamDetailsFromFile();

    StreamDetails *streamDetails = m_movie->streamDetails();
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
void MovieWidget::onReloadStreamDetails()
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
 * @brief Shows the MovieImageDialog and after successful execution starts poster download
 */
void MovieWidget::chooseMoviePoster()
{
    qDebug() << "Entered";
    if (m_movie == 0) {
        qDebug() << "My movie is invalid";
        return;
    }

    ImageDialog::instance()->setImageType(TypePoster);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMovie(m_movie);
    ImageDialog::instance()->setDownloads(m_movie->posters());
    ImageDialog::instance()->exec(ImageDialogType::MoviePoster);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetMovies);
        m_movie->controller()->loadImage(TypePoster, ImageDialog::instance()->imageUrl());
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the MovieImageDialog and after successful execution starts backdrop download
 */
void MovieWidget::chooseMovieBackdrop()
{
    if (m_movie == 0)
        return;

    ImageDialog::instance()->setImageType(TypeBackdrop);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMovie(m_movie);
    ImageDialog::instance()->setDownloads(m_movie->backdrops());
    ImageDialog::instance()->exec(ImageDialogType::MovieBackdrop);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetMovies);
        m_movie->controller()->loadImage(TypeBackdrop, ImageDialog::instance()->imageUrl());
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the MovieImageDialog and after successful execution starts logo download
 */
void MovieWidget::chooseMovieLogo()
{
    if (m_movie == 0)
        return;

    ImageDialog::instance()->setImageType(TypeLogo);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMovie(m_movie);
    ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(ImageDialogType::MovieLogo);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetMovies);
        m_movie->controller()->loadImage(TypeLogo, ImageDialog::instance()->imageUrl());
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the MovieImageDialog and after successful execution starts clear art download
 */
void MovieWidget::chooseMovieClearArt()
{
    if (m_movie == 0)
        return;

    ImageDialog::instance()->setImageType(TypeClearArt);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMovie(m_movie);
    ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(ImageDialogType::MovieClearArt);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetMovies);
        m_movie->controller()->loadImage(TypeClearArt, ImageDialog::instance()->imageUrl());
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the MovieImageDialog and after successful execution starts cd art download
 */
void MovieWidget::chooseMovieCdArt()
{
    if (m_movie == 0)
        return;

    ImageDialog::instance()->setImageType(TypeCdArt);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMovie(m_movie);
    ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(ImageDialogType::MovieCdArt);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetMovies);
        m_movie->controller()->loadImage(TypeCdArt, ImageDialog::instance()->imageUrl());
        ui->buttonRevert->setVisible(true);
    }
}

void MovieWidget::onDownloadTrailer()
{
    if (!m_movie)
        return;
    TrailerDialog::instance()->exec(m_movie);
    ui->localTrailer->setVisible(m_movie->hasLocalTrailer());
}

/**
 * @brief Saves movie information
 */
void MovieWidget::saveInformation()
{
    qDebug() << "Entered";
    setDisabledTrue();
    int id = MessageBox::instance()->showMessage(tr("Saving movie..."));
    m_savingWidget->show();
    m_movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
    m_movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
    updateMovieInfo();
    setEnabledTrue();
    m_savingWidget->hide();
    MessageBox::instance()->removeMessage(id);
    MessageBox::instance()->showMessage(tr("<b>\"%1\"</b> Saved").arg(m_movie->name()));
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

    MessageBox::instance()->showProgressBar(tr("Saving movies..."), Constants::MovieWidgetProgressMessageId);
    MessageBox::instance()->progressBarProgress(0, moviesToSave, Constants::MovieWidgetProgressMessageId);
    qApp->processEvents();
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->hasChanged()) {
            MessageBox::instance()->progressBarProgress(counter++, moviesToSave, Constants::MovieWidgetProgressMessageId);
            qApp->processEvents();
            movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
            movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
            if (m_movie == movie)
                updateMovieInfo();
        }
    }
    setEnabledTrue();
    m_savingWidget->hide();
    MessageBox::instance()->hideProgressBar(Constants::MovieWidgetProgressMessageId);
    MessageBox::instance()->showMessage(tr("All Movies Saved"));
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
 * @brief Shows a full size image of the backdrop
 */
void MovieWidget::onPreviewBackdrop()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentBackdrop));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows a full size image of the poster
 */
void MovieWidget::onPreviewPoster()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentPoster));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows a full size image of the logo
 */
void MovieWidget::onPreviewLogo()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentLogo));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows a full size image of the clear art
 */
void MovieWidget::onPreviewClearArt()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentClearArt));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows a full size image of the cd art
 */
void MovieWidget::onPreviewCdArt()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentCdArt));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows the image of the selected actor
 */
void MovieWidget::onActorChanged()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount() ||
        ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        ui->actor->setPixmap(QPixmap(":/img/actor.png"));
        ui->actorResolution->setText("");
        return;
    }

    Actor *actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
    if (!actor->image.isNull()) {
        QImage img = QImage::fromData(actor->image);
        ui->actor->setPixmap(QPixmap::fromImage(img).scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->actorResolution->setText(QString("%1 x %2").arg(img.width()).arg(img.height()));
    } else if (!Manager::instance()->mediaCenterInterface()->actorImageName(m_movie, *actor).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->actorImageName(m_movie, *actor));
        ui->actor->setPixmap(p.scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->actorResolution->setText(QString("%1 x %2").arg(p.width()).arg(p.height()));
    } else {
        ui->actor->setPixmap(QPixmap(":/img/actor.png"));
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

    QString fileName = QFileDialog::getOpenFileName(parentWidget(), tr("Choose Image"), QDir::homePath(), tr("Images (*.jpg *.jpeg)"));
    if (!fileName.isNull()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            Actor *actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
            actor->image = file.readAll();
            actor->imageHasChanged = true;
            onActorChanged();
            m_movie->setChanged(true);
            file.close();
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

/**
 * @brief Marks the movie as changed when the watched state has changed
 */
void MovieWidget::onWatchedChange(int state)
{
    if (!m_movie)
        return;
    m_movie->setWatched(state == Qt::Checked);
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

void MovieWidget::updateBackgroundImage()
{
    // @todo: add config option to disable background image
    // @todo: make ui->movieName more readable, maybe with text shadow
    return;
    m_backgroundLabel->setFixedSize(size());
    if (m_currentBackdrop.isNull())
        m_backgroundLabel->setPixmap(QPixmap());
    else
        m_backgroundLabel->setPixmap(QPixmap::fromImage(m_currentBackdrop.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
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

    ImageDialog::instance()->setImageType(TypeExtraFanart);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMultiSelection(true);
    ImageDialog::instance()->setMovie(m_movie);
    ImageDialog::instance()->setDownloads(m_movie->backdrops());
    ImageDialog::instance()->exec(ImageDialogType::MovieBackdrop);

    if (ImageDialog::instance()->result() == QDialog::Accepted && !ImageDialog::instance()->imageUrls().isEmpty()) {
        ui->fanarts->setLoading(true);
        emit setActionSaveEnabled(false, WidgetMovies);
        m_movie->controller()->loadImages(TypeExtraFanart, ImageDialog::instance()->imageUrls());
        ui->buttonRevert->setVisible(true);
    }
}
