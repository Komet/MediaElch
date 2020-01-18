#include "MovieWidget.h"
#include "ui_MovieWidget.h"

#include "data/ImageCache.h"
#include "globals/ComboDelegate.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"
#include "globals/TrailerDialog.h"
#include "scrapers/movie/CustomMovieScraper.h"
#include "ui/main/MainWindow.h"
#include "ui/movies/MovieFilesWidget.h"
#include "ui/movies/MovieSearch.h"
#include "ui/notifications/NotificationBox.h"
#include "ui/small_widgets/ClosableImage.h"

#include <QDoubleValidator>
#include <QFileDialog>
#include <QIntValidator>
#include <QMovie>
#include <QPainter>
#include <QPixmapCache>
#include <QScrollBar>
#include <QtCore/qmath.h>

MovieWidget::MovieWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MovieWidget)
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
    ui->localTrailer->setBadgeType(Badge::Type::LabelSuccess);
    ui->localTrailer->setVisible(false);
    ui->badgeWatched->setBadgeType(Badge::Type::BadgeInfo);

#ifndef Q_OS_MAC
    QFont nameFont = ui->movieName->font();
    nameFont.setPointSize(nameFont.pointSize() - 4);
    ui->movieName->setFont(nameFont);
#endif

    QFont font = ui->actorResolution->font();
#ifndef Q_OS_MAC
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->actorResolution->setFont(font);

    ui->labelBanner->setFont(font);
    ui->labelClearArt->setFont(font);
    ui->labelDiscArt->setFont(font);
    ui->labelFanart->setFont(font);
    ui->labelLogo->setFont(font);
    ui->labelPoster->setFont(font);
    ui->labelThumb->setFont(font);

    m_movie = nullptr;

    ui->poster->setDefaultPixmap(QPixmap(":/img/placeholders/poster.png"));
    ui->backdrop->setDefaultPixmap(QPixmap(":/img/placeholders/fanart.png"));
    ui->logo->setDefaultPixmap(QPixmap(":/img/placeholders/logo.png"));
    ui->clearArt->setDefaultPixmap(QPixmap(":/img/placeholders/clear_art.png"));
    ui->cdArt->setDefaultPixmap(QPixmap(":/img/placeholders/cd_art.png"));
    ui->thumb->setDefaultPixmap(QPixmap(":/img/placeholders/thumb.png"));
    ui->banner->setDefaultPixmap(QPixmap(":/img/placeholders/banner.png"));

    ui->buttonDownloadTrailer->setIcon(
        Manager::instance()->iconFont()->icon("download", QColor(150, 150, 150), "", -1, 1.0));
    ui->buttonYoutubeDummy->setIcon(Manager::instance()->iconFont()->icon("pen", QColor(150, 150, 150), "", -1, 1.0));
    ui->buttonPlayLocalTrailer->setIcon(
        Manager::instance()->iconFont()->icon("play", QColor(150, 150, 150), "", -1, 1.0));

    ui->genreCloud->setText(tr("Genres"));
    ui->genreCloud->setPlaceholder(tr("Add Genre"));
    connect(ui->genreCloud, &TagCloud::activated, this, &MovieWidget::addGenre);
    connect(ui->genreCloud, &TagCloud::deactivated, this, &MovieWidget::removeGenre);

    ui->tagCloud->setText(tr("Tags"));
    ui->tagCloud->setPlaceholder(tr("Add Tag"));
    connect(ui->tagCloud, &TagCloud::activated, this, &MovieWidget::addTag);
    connect(ui->tagCloud, &TagCloud::deactivated, this, &MovieWidget::removeTag);

    ui->countryCloud->setText(tr("Countries"));
    ui->countryCloud->setPlaceholder(tr("Add Country"));
    connect(ui->countryCloud, &TagCloud::activated, this, &MovieWidget::addCountry);
    connect(ui->countryCloud, &TagCloud::deactivated, this, &MovieWidget::removeCountry);

    ui->studioCloud->setText(tr("Studios"));
    ui->studioCloud->setPlaceholder(tr("Add Studio"));
    ui->studioCloud->setBadgeType(TagCloud::BadgeType::SimpleLabel);
    connect(ui->studioCloud, &TagCloud::activated, this, &MovieWidget::addStudio);
    connect(ui->studioCloud, &TagCloud::deactivated, this, &MovieWidget::removeStudio);

    ui->labelSepFoldersWarning->setErrorMessage(ui->labelSepFoldersWarning->text());

    ui->poster->setImageType(ImageType::MoviePoster);
    ui->backdrop->setImageType(ImageType::MovieBackdrop);
    ui->logo->setImageType(ImageType::MovieLogo);
    ui->cdArt->setImageType(ImageType::MovieCdArt);
    ui->banner->setImageType(ImageType::MovieBanner);
    ui->thumb->setImageType(ImageType::MovieThumb);
    ui->clearArt->setImageType(ImageType::MovieClearArt);
    for (ClosableImage* image : ui->artStackedWidget->findChildren<ClosableImage*>()) {
        connect(image, &ClosableImage::clicked, this, &MovieWidget::onChooseImage);
        connect(image, &ClosableImage::sigClose, this, &MovieWidget::onDeleteImage);
        connect(image, &ClosableImage::sigImageDropped, this, &MovieWidget::onImageDropped);
    }

    // clang-format off
    connect(ui->name,              &QLineEdit::textChanged,             this, &MovieWidget::movieNameChanged);
    connect(ui->buttonAddActor,    &QAbstractButton::clicked,           this, &MovieWidget::addActor);
    connect(ui->buttonRemoveActor, &QAbstractButton::clicked,           this, &MovieWidget::removeActor);
    connect(ui->actors,            &QTableWidget::itemChanged,          this, &MovieWidget::onActorEdited);
    connect(ui->actors,            &QTableWidget::itemSelectionChanged, this, &MovieWidget::onActorChanged);
    connect(ui->actor,             &MyLabel::clicked,                   this, &MovieWidget::onChangeActorImage);
    connect(ui->subtitles,         &QTableWidget::itemChanged,          this, &MovieWidget::onSubtitleEdited);
    connect(ui->buttonRevert,      &QAbstractButton::clicked,           this, &MovieWidget::onRevertChanges);

    connect(ui->buttonReloadStreamDetails, &QAbstractButton::clicked, this, &MovieWidget::onReloadStreamDetails);

    connect(ui->buttonDownloadTrailer,  &QToolButton::clicked, this, &MovieWidget::onDownloadTrailer);
    connect(ui->buttonYoutubeDummy,     &QToolButton::clicked, this, &MovieWidget::onInsertYoutubeLink);
    connect(ui->buttonPlayLocalTrailer, &QToolButton::clicked, this, &MovieWidget::onPlayLocalTrailer);

    connect(ui->fanarts, SIGNAL(sigRemoveImage(QByteArray)), this, SLOT(onRemoveExtraFanart(QByteArray)));
    connect(ui->fanarts, SIGNAL(sigRemoveImage(QString)),    this, SLOT(onRemoveExtraFanart(QString)));
    connect(ui->btnAddExtraFanart, &QAbstractButton::clicked,      this, &MovieWidget::onAddExtraFanart);
    connect(ui->fanarts,           &ImageGallery::sigImageDropped, this, &MovieWidget::onExtraFanartDropped);
    // clang-format on

    m_loadingMovie = new QMovie(":/img/spinner.gif", QByteArray(), this);
    m_loadingMovie->start();

    setDisabledTrue();
    clear();

    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    // Connect GUI change events to movie object
    // clang-format off
    connect(ui->imdbId,           &QLineEdit::textEdited,           this, &MovieWidget::onImdbIdChange);
    connect(ui->name,             &QLineEdit::textEdited,           this, &MovieWidget::onNameChange);
    connect(ui->originalName,     &QLineEdit::textEdited,           this, &MovieWidget::onOriginalNameChange);
    connect(ui->sortTitle,        &QLineEdit::textEdited,           this, &MovieWidget::onSortTitleChange);
    connect(ui->tagline,          &QLineEdit::textEdited,           this, &MovieWidget::onTaglineChange);
    connect(ui->rating,           SIGNAL(valueChanged(double)),     this, SLOT(onRatingChange(double)));
    connect(ui->userRating,       SIGNAL(valueChanged(double)),     this, SLOT(onUserRatingChange(double)));
    connect(ui->votes,            SIGNAL(valueChanged(int)),        this, SLOT(onVotesChange(int)));
    connect(ui->top250,           SIGNAL(valueChanged(int)),        this, SLOT(onTop250Change(int)));
    connect(ui->trailer,          &QLineEdit::textEdited,           this, &MovieWidget::onTrailerChange);
    connect(ui->runtime,          SIGNAL(valueChanged(int)),        this, SLOT(onRuntimeChange(int)));
    connect(ui->playcount,        SIGNAL(valueChanged(int)),        this, SLOT(onPlayCountChange(int)));
    connect(ui->certification,    &QComboBox::editTextChanged,      this, &MovieWidget::onCertificationChange);
    connect(ui->set,              &QComboBox::editTextChanged,      this, &MovieWidget::onSetChange);
    connect(ui->badgeWatched,     &Badge::clicked,                  this, &MovieWidget::onWatchedClicked);
    connect(ui->released,         &QDateTimeEdit::dateChanged,      this, &MovieWidget::onReleasedChange);
    connect(ui->lastPlayed,       &QDateTimeEdit::dateTimeChanged,  this, &MovieWidget::onLastWatchedChange);
    connect(ui->overview,         &QTextEdit::textChanged,          this, &MovieWidget::onOverviewChange);
    connect(ui->outline,          &QTextEdit::textChanged,          this, &MovieWidget::onOutlineChange);
    connect(ui->director,         &QLineEdit::textEdited,           this, &MovieWidget::onDirectorChange);
    connect(ui->writer,           &QLineEdit::textEdited,           this, &MovieWidget::onWriterChange);
    connect(ui->videoAspectRatio, SIGNAL(valueChanged(double)),     this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoCodec,       &QLineEdit::textEdited,           this, &MovieWidget::onStreamDetailsEdited);
    connect(ui->videoDuration,    &QDateTimeEdit::timeChanged,      this, &MovieWidget::onStreamDetailsEdited);
    connect(ui->videoHeight,      SIGNAL(valueChanged(int)),        this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoWidth,       SIGNAL(valueChanged(int)),        this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoScantype,    &QLineEdit::textEdited,           this, &MovieWidget::onStreamDetailsEdited);
    connect(ui->stereoMode,       SIGNAL(currentIndexChanged(int)), this, SLOT(onStreamDetailsEdited()));
    // clang-format on

    ui->rating->setSingleStep(0.1);
    ui->rating->setMinimum(0.0);
    ui->userRating->setSingleStep(0.1);
    ui->userRating->setMinimum(0.0);

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
    helper::fillStereoModeCombo(ui->stereoMode);
}

MovieWidget::~MovieWidget()
{
    delete ui;
}

/**
 * @brief Repositions the saving widget
 */
void MovieWidget::resizeEvent(QResizeEvent* event)
{
    m_savingWidget->move(size().width() / 2 - m_savingWidget->width(), height() / 2 - m_savingWidget->height());
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

    blocked = ui->userRating->blockSignals(true);
    ui->userRating->clear();
    ui->userRating->blockSignals(blocked);

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
    ui->actors->blockSignals(blocked);

    blocked = ui->subtitles->blockSignals(true);
    ui->subtitles->setRowCount(0);
    ui->subtitles->blockSignals(blocked);

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
    helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
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
void MovieWidget::setEnabledTrue(Movie* movie)
{
    qDebug() << "Entered";
    if (movie != nullptr) {
        qDebug() << movie->name();
    }
    if ((movie != nullptr) && movie->controller()->downloadsInProgress()) {
        qDebug() << "Downloads are in progress";
        return;
    }
    ui->groupBox_3->setEnabled(true);
    emit setActionSaveEnabled(true, MainWidgets::Movies);
    emit setActionSearchEnabled(true, MainWidgets::Movies);
}

/**
 * @brief Sets the state of the main groupbox to disabled
 */
void MovieWidget::setDisabledTrue()
{
    qDebug() << "Entered";
    ui->groupBox_3->setDisabled(true);
    emit setActionSaveEnabled(false, MainWidgets::Movies);
    emit setActionSearchEnabled(false, MainWidgets::Movies);
}

/**
 * @brief Sets the current movie, tells the movie to load data and images and updates widgets contents
 * @param movie Current movie
 */
void MovieWidget::setMovie(Movie* movie)
{
    using namespace std::chrono;
    qDebug() << "Entered, movie=" << movie->name();
    movie->controller()->loadData(Manager::instance()->mediaCenterInterface());
    if (!movie->streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails()) {
        movie->controller()->loadStreamDetailsFromFile();
        const seconds durationInSeconds = seconds(
            movie->streamDetails()->videoDetails().value(StreamDetails::VideoDetails::DurationInSeconds).toInt());
        if (movie->streamDetailsLoaded() && durationInSeconds > 0s) {
            movie->setRuntime(duration_cast<minutes>(durationInSeconds));
        }
    }
    m_movie = movie;
    updateMovieInfo();

    connect(m_movie->controller(),
        &MovieController::sigInfoLoadDone,
        this,
        &MovieWidget::onInfoLoadDone,
        Qt::UniqueConnection);
    connect(m_movie->controller(), &MovieController::sigLoadDone, this, &MovieWidget::onLoadDone, Qt::UniqueConnection);
    connect(m_movie->controller(),
        &MovieController::sigDownloadProgress,
        this,
        &MovieWidget::onDownloadProgress,
        Qt::UniqueConnection);
    connect(m_movie->controller(),
        &MovieController::sigLoadingImages,
        this,
        &MovieWidget::onLoadingImages,
        Qt::UniqueConnection);
    connect(m_movie->controller(),
        &MovieController::sigLoadImagesStarted,
        this,
        &MovieWidget::onLoadImagesStarted,
        Qt::UniqueConnection);
    connect(m_movie->controller(), &MovieController::sigImage, this, &MovieWidget::onSetImage, Qt::UniqueConnection);

    ui->btnAddExtraFanart->setEnabled(movie->inSeparateFolder());
    ui->labelSepFoldersWarning->setVisible(!movie->inSeparateFolder());

    if (movie->controller()->downloadsInProgress()) {
        setDisabledTrue();
    } else {
        setEnabledTrue();
    }
}

/**
 * @brief Shows the search widget
 */
void MovieWidget::startScraperSearch()
{
    qDebug() << "Entered";
    if (m_movie == nullptr) {
        qDebug() << "My movie is invalid";
        return;
    }
    emit setActionSearchEnabled(false, MainWidgets::Movies);
    emit setActionSaveEnabled(false, MainWidgets::Movies);
    MovieSearch::instance()->exec(m_movie->name(), m_movie->imdbId(), m_movie->tmdbId());
    if (MovieSearch::instance()->result() == QDialog::Accepted) {
        setDisabledTrue();
        QMap<MovieScraperInterface*, QString> ids;
        QVector<MovieScraperInfos> infosToLoad;
        if (MovieSearch::instance()->scraperId() == CustomMovieScraper::scraperIdentifier) {
            ids = MovieSearch::instance()->customScraperIds();
            infosToLoad = Settings::instance()->scraperInfos<MovieScraperInfos>(CustomMovieScraper::scraperIdentifier);
        } else {
            ids.insert(0, MovieSearch::instance()->scraperMovieId());
            infosToLoad = MovieSearch::instance()->infosToLoad();
        }
        m_movie->controller()->loadData(
            ids, Manager::instance()->scraper(MovieSearch::instance()->scraperId()), infosToLoad);
    } else {
        emit setActionSearchEnabled(true, MainWidgets::Movies);
        emit setActionSaveEnabled(true, MainWidgets::Movies);
    }
}

void MovieWidget::onInfoLoadDone(Movie* movie)
{
    if (m_movie == nullptr) {
        return;
    }
    if (m_movie == movie) {
        updateMovieInfo();
        ui->buttonRevert->setVisible(true);
        emit setActionSaveEnabled(false, MainWidgets::Movies);
    }
}

void MovieWidget::onLoadDone(Movie* movie)
{
    emit actorDownloadFinished(Constants::MovieProgressMessageId + movie->movieId());
    if (m_movie == nullptr || m_movie != movie) {
        return;
    }
    setEnabledTrue();
    ui->fanarts->setLoading(false);
}

void MovieWidget::onLoadImagesStarted(Movie* movie)
{
    emit actorDownloadStarted(tr("Downloading images..."), Constants::MovieProgressMessageId + movie->movieId());
}

void MovieWidget::onLoadingImages(Movie* movie, QVector<ImageType> imageTypes)
{
    if (movie != m_movie) {
        return;
    }

    for (const auto imageType : imageTypes) {
        for (ClosableImage* cImage : ui->artStackedWidget->findChildren<ClosableImage*>()) {
            if (cImage->imageType() == imageType) {
                cImage->setLoading(true);
            }
        }
    }

    if (imageTypes.contains(ImageType::MovieExtraFanart)) {
        ui->fanarts->setLoading(true);
    }
    ui->groupBox_3->update();
}

void MovieWidget::onSetImage(Movie* movie, ImageType type, QByteArray imageData)
{
    if (movie != m_movie) {
        return;
    }

    if (type == ImageType::MovieExtraFanart) {
        ui->fanarts->addImage(imageData);
        return;
    }

    for (auto image : ui->artStackedWidget->findChildren<ClosableImage*>()) {
        if (image->imageType() == type) {
            image->setLoading(false);
            image->setImage(imageData);
        }
    }
}

void MovieWidget::onDownloadProgress(Movie* movie, int current, int maximum)
{
    emit actorDownloadProgress(maximum - current, maximum, Constants::MovieProgressMessageId + movie->movieId());
}

/**
 * @brief Updates the contents of the widget with the current movie infos
 */
void MovieWidget::updateMovieInfo()
{
    if (m_movie == nullptr) {
        return;
    }

    ui->rating->blockSignals(true);
    ui->userRating->blockSignals(true);
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
    ui->imdbId->setText(m_movie->imdbId().toString());
    ui->name->setText(m_movie->name());
    ui->movieName->setText(m_movie->name());
    ui->originalName->setText(m_movie->originalName());
    ui->sortTitle->setText(m_movie->sortTitle());
    ui->tagline->setText(m_movie->tagline());
    // TODO: multiple ratings
    if (!m_movie->ratings().isEmpty()) {
        ui->rating->setValue(m_movie->ratings().back().rating);
        ui->votes->setValue(m_movie->ratings().back().voteCount);
    }
    ui->userRating->setValue(m_movie->userRating());
    ui->top250->setValue(m_movie->top250());
    ui->released->setDate(m_movie->released());
    ui->runtime->setValue(static_cast<int>(m_movie->runtime().count()));
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
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (!sets.contains(movie->set().name) && !movie->set().name.isEmpty()) {
            sets.append(movie->set().name);
        }
        const QString certStr = movie->certification().toString();
        if (!certifications.contains(certStr) && movie->certification().isValid()) {
            certifications.append(certStr);
        }
    }
    std::sort(sets.begin(), sets.end(), LocaleStringCompare());
    std::sort(certifications.begin(), certifications.end(), LocaleStringCompare());
    ui->certification->addItems(certifications);
    ui->set->addItems(sets);

    ui->certification->setCurrentIndex(certifications.indexOf(m_movie->certification().toString()));
    ui->set->setCurrentIndex(sets.indexOf(m_movie->set().name));

    ui->set->blockSignals(false);
    ui->certification->blockSignals(false);

    ui->actors->blockSignals(true);
    for (Actor* actor : m_movie->actors()) {
        const int row = ui->actors->rowCount();
        ui->actors->insertRow(row);
        ui->actors->setItem(row, 0, new QTableWidgetItem(actor->name));
        ui->actors->setItem(row, 1, new QTableWidgetItem(actor->role));
        ui->actors->item(row, 0)->setData(Qt::UserRole, actor->thumb);
        ui->actors->item(row, 1)->setData(Qt::UserRole, QVariant::fromValue(actor));
    }
    ui->actors->blockSignals(false);

    ui->subtitles->blockSignals(true);
    for (Subtitle* subtitle : m_movie->subtitles()) {
        int row = ui->subtitles->rowCount();
        ui->subtitles->insertRow(row);

        QTableWidgetItem* item0 = new QTableWidgetItem(subtitle->files().join(", "));
        item0->setFlags(Qt::ItemIsSelectable);
        item0->setData(Qt::UserRole, QVariant::fromValue(subtitle));
        ui->subtitles->setItem(row, 0, item0);

        QTableWidgetItem* item1 = new QTableWidgetItem(subtitle->language());
        item1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        ui->subtitles->setItem(row, 1, item1);

        auto item2 = new QTableWidgetItem;
        item2->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        item2->setCheckState(subtitle->forced() ? Qt::Checked : Qt::Unchecked);
        ui->subtitles->setItem(row, 2, item2);
    }

    ui->subtitles->blockSignals(false);

    QStringList genres;
    QStringList tags;
    QStringList countries;
    QStringList studios;
    for (const Movie* movie : Manager::instance()->movieModel()->movies()) {
        genres << movie->genres();
        tags << movie->tags();
        countries << movie->countries();
        studios << movie->studios();
    }

    // `setTags` requires distinct lists
    genres.removeDuplicates();
    tags.removeDuplicates();
    countries.removeDuplicates();
    studios.removeDuplicates();

    ui->genreCloud->setTags(genres, m_movie->genres());
    ui->tagCloud->setTags(tags, m_movie->tags());
    ui->countryCloud->setTags(countries, m_movie->countries());
    ui->studioCloud->setTags(m_movie->studios(), m_movie->studios());
    auto completer = new QCompleter(studios, this);
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

    updateImages({ImageType::MoviePoster,
        ImageType::MovieBackdrop,
        ImageType::MovieLogo,
        ImageType::MovieCdArt,
        ImageType::MovieClearArt,
        ImageType::MovieBanner,
        ImageType::MovieThumb});

    ui->fanarts->setImages(m_movie->images().extraFanarts(Manager::instance()->mediaCenterInterface()));

    ui->rating->blockSignals(false);
    ui->userRating->blockSignals(false);
    ui->votes->blockSignals(false);
    ui->top250->blockSignals(false);
    ui->runtime->blockSignals(false);
    ui->playcount->blockSignals(false);
    ui->released->blockSignals(false);
    ui->lastPlayed->blockSignals(false);
    ui->overview->blockSignals(false);
    ui->outline->blockSignals(false);
    ui->actors->blockSignals(false);

    emit setActionSaveEnabled(true, MainWidgets::Movies);

    ui->buttonRevert->setVisible(m_movie->hasChanged());
    ui->localTrailer->setVisible(m_movie->hasLocalTrailer());
}

void MovieWidget::updateImages(QVector<ImageType> images)
{
    for (const auto imageType : images) {
        for (auto cImage : ui->artStackedWidget->findChildren<ClosableImage*>()) {
            if (cImage->imageType() == imageType) {
                updateImage(imageType, cImage);
                break;
            }
        }
    }
}

void MovieWidget::updateImage(ImageType imageType, ClosableImage* image)
{
    if (!m_movie->images().image(imageType).isNull()) {
        image->setImage(m_movie->images().image(imageType));
    } else if (!m_movie->images().imagesToRemove().contains(imageType) && m_movie->hasImage(imageType)) {
        QString imgFileName = Manager::instance()->mediaCenterInterface()->imageFileName(m_movie, imageType);
        if (!imgFileName.isEmpty()) {
            image->setImage(imgFileName);
        }
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

    if (reloadFromFile) {
        m_movie->controller()->loadStreamDetailsFromFile();
    }

    StreamDetails* streamDetails = m_movie->streamDetails();
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
        ui->runtime->setValue(
            qFloor(streamDetails->videoDetails().value(StreamDetails::VideoDetails::DurationInSeconds).toInt() / 60.0));
    }

    for (QWidget* widget : m_streamDetailsWidgets) {
        widget->deleteLater();
    }
    m_streamDetailsWidgets.clear();
    m_streamDetailsAudio.clear();
    m_streamDetailsSubtitles.clear();

    int audioTracks = streamDetails->audioDetails().count();
    const auto audioDetails = streamDetails->audioDetails();
    for (int i = 0; i < audioTracks; ++i) {
        QLabel* label = new QLabel(tr("Track %1").arg(i + 1));
        label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ui->streamDetails->addWidget(label, 8 + i, 0);
        QLineEdit* edit1 = new QLineEdit(audioDetails.at(i).value(StreamDetails::AudioDetails::Language));
        QLineEdit* edit2 = new QLineEdit(audioDetails.at(i).value(StreamDetails::AudioDetails::Codec));
        QLineEdit* edit3 = new QLineEdit(audioDetails.at(i).value(StreamDetails::AudioDetails::Channels));
        edit3->setMaximumWidth(50);
        edit1->setToolTip(tr("Language"));
        edit2->setToolTip(tr("Codec"));
        edit3->setToolTip(tr("Channels"));
        edit1->setPlaceholderText(tr("Language"));
        edit2->setPlaceholderText(tr("Codec"));
        edit3->setPlaceholderText(tr("Channels"));
        auto layout = new QHBoxLayout();
        layout->addWidget(edit1);
        layout->addWidget(edit2);
        layout->addWidget(edit3);
        layout->addStretch(10);
        ui->streamDetails->addLayout(layout, 8 + i, 1);
        m_streamDetailsWidgets << label << edit1 << edit2 << edit3;
        m_streamDetailsAudio << (QVector<QLineEdit*>() << edit1 << edit2 << edit3);
        connect(edit1, &QLineEdit::textEdited, this, &MovieWidget::onStreamDetailsEdited);
        connect(edit2, &QLineEdit::textEdited, this, &MovieWidget::onStreamDetailsEdited);
        connect(edit3, &QLineEdit::textEdited, this, &MovieWidget::onStreamDetailsEdited);
    }

    if (!streamDetails->subtitleDetails().isEmpty()) {
        QLabel* subtitleLabel = new QLabel(tr("Subtitles"));
        subtitleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        QFont f = ui->labelStreamDetailsAudio->font();
        f.setBold(true);
        subtitleLabel->setFont(f);
        ui->streamDetails->addWidget(subtitleLabel, 8 + audioTracks, 0);
        m_streamDetailsWidgets << subtitleLabel;

        for (int i = 0, n = streamDetails->subtitleDetails().count(); i < n; ++i) {
            QLabel* trackLabel = new QLabel(tr("Track %1").arg(i + 1));
            trackLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            ui->streamDetails->addWidget(trackLabel, 9 + audioTracks + i, 0);
            QLineEdit* edit1 =
                new QLineEdit(streamDetails->subtitleDetails().at(i).value(StreamDetails::SubtitleDetails::Language));
            edit1->setToolTip(tr("Language"));
            edit1->setPlaceholderText(tr("Language"));
            auto layout = new QHBoxLayout();
            layout->addWidget(edit1);
            layout->addStretch(10);
            ui->streamDetails->addLayout(layout, 9 + audioTracks + i, 1);
            m_streamDetailsWidgets << trackLabel << edit1;
            m_streamDetailsSubtitles << (QVector<QLineEdit*>() << edit1);
            connect(edit1, &QLineEdit::textEdited, this, &MovieWidget::onStreamDetailsEdited);
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
    if (m_movie == nullptr) {
        return;
    }
    TrailerDialog::instance()->exec(m_movie);
    ui->localTrailer->setVisible(m_movie->hasLocalTrailer());
}

void MovieWidget::onPlayLocalTrailer()
{
    if ((m_movie == nullptr) && !m_movie->hasLocalTrailer()) {
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(m_movie->localTrailerFileName()));
}

/**
 * @brief Saves movie information
 */
void MovieWidget::saveInformation()
{
    qDebug() << "Entered";
    setDisabledTrue();

    QVector<Movie*> movies = MovieFilesWidget::instance()->selectedMovies();
    if (movies.isEmpty()) {
        movies.append(m_movie);
    }

    m_savingWidget->show();
    if (movies.count() > 0) {
        int counter = 0;
        int moviesToSave = movies.count();

        NotificationBox::instance()->showProgressBar(tr("Saving movies..."), Constants::MovieWidgetProgressMessageId);
        NotificationBox::instance()->progressBarProgress(0, moviesToSave, Constants::MovieWidgetProgressMessageId);
        QApplication::processEvents();
        for (Movie* movie : movies) {
            counter++;
            if (movie->hasChanged()) {
                NotificationBox::instance()->progressBarProgress(
                    counter, moviesToSave, Constants::MovieWidgetProgressMessageId);
                QApplication::processEvents();
                movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
                movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
                if (m_movie == movie) {
                    updateMovieInfo();
                }
            }
        }
        NotificationBox::instance()->hideProgressBar(Constants::MovieWidgetProgressMessageId);
        NotificationBox::instance()->showSuccess(tr("Movies Saved"));
    } else {
        int id = NotificationBox::instance()->showMessage(tr("Saving movie..."));
        m_movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
        m_movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
        updateMovieInfo();
        NotificationBox::instance()->removeMessage(id);
        NotificationBox::instance()->showSuccess(tr("<b>\"%1\"</b> Saved").arg(m_movie->name()));
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
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (movie->hasChanged()) {
            moviesToSave++;
        }
    }

    NotificationBox::instance()->showProgressBar(tr("Saving movies..."), Constants::MovieWidgetProgressMessageId);
    NotificationBox::instance()->progressBarProgress(0, moviesToSave, Constants::MovieWidgetProgressMessageId);
    QApplication::processEvents();
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (movie->hasChanged()) {
            NotificationBox::instance()->progressBarProgress(
                counter++, moviesToSave, Constants::MovieWidgetProgressMessageId);
            QApplication::processEvents();
            movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
            movie->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
            if (m_movie == movie) {
                updateMovieInfo();
            }
        }
    }
    setEnabledTrue();
    m_savingWidget->hide();
    NotificationBox::instance()->hideProgressBar(Constants::MovieWidgetProgressMessageId);
    NotificationBox::instance()->showSuccess(tr("All Movies Saved"));
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

    Actor* actor = m_movie->actors().back();

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
    const int row = ui->actors->currentRow();
    const int rowCount = ui->actors->rowCount();
    if (row < 0 || row >= rowCount) {
        qCritical() << "[MovieWidget] Selected actor index is out of range:" << row << "/" << rowCount;
        return;
    }
    if (!ui->actors->currentItem()->isSelected()) {
        qInfo() << "[MovieWidget] Cannot remove actor because none is selected!";
        return;
    }

    auto* actor = ui->actors->item(row, 1)->data(Qt::UserRole).value<Actor*>();
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
void MovieWidget::onActorEdited(QTableWidgetItem* item)
{
    auto actor = ui->actors->item(item->row(), 1)->data(Qt::UserRole).value<Actor*>();
    if (item->column() == 0) {
        actor->name = item->text();
    } else if (item->column() == 1) {
        actor->role = item->text();
    }
    m_movie->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onSubtitleEdited(QTableWidgetItem* item)
{
    auto subtitle = ui->subtitles->item(item->row(), 0)->data(Qt::UserRole).value<Subtitle*>();
    if (subtitle == nullptr) {
        return;
    }
    if (item->column() == 1) {
        subtitle->setLanguage(item->text());
    } else if (item->column() == 2) {
        subtitle->setForced(item->checkState() == Qt::Checked);
    }
    m_movie->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Adds a studio
 */
void MovieWidget::addStudio(QString studio)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->addStudio(studio);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a studio
 */
void MovieWidget::removeStudio(QString studio)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->removeStudio(studio);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Adds a country
 */
void MovieWidget::addCountry(QString country)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->addCountry(country);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a country
 */
void MovieWidget::removeCountry(QString country)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->removeCountry(country);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Shows the image of the selected actor
 */
void MovieWidget::onActorChanged()
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
        QPixmap p = QPixmap::fromImage(QImage::fromData(actor->image));
        ui->actorResolution->setText(QString("%1 x %2").arg(p.width()).arg(p.height()));
        p = p.scaled(QSize(120, 180) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        helper::setDevicePixelRatio(p, helper::devicePixelRatio(this));
        ui->actor->setPixmap(p);
    } else if (!Manager::instance()->mediaCenterInterface()->actorImageName(m_movie, *actor).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->actorImageName(m_movie, *actor));
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
 * @brief Stores the changed actor image
 */
void MovieWidget::onChangeActorImage()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount()
        || ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        return;
    }

    static QString lastPath = QDir::homePath();

    QString fileName =
        QFileDialog::getOpenFileName(parentWidget(), tr("Choose Image"), lastPath, tr("Images (*.jpg *.jpeg)"));
    if (!fileName.isNull()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            auto actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
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
    if (m_movie == nullptr) {
        return;
    }
    m_movie->addGenre(genre);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::removeGenre(QString genre)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->removeGenre(genre);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::addTag(QString tag)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->addTag(tag);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::removeTag(QString tag)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->removeTag(tag);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the name has changed
 */
void MovieWidget::onNameChange(QString text)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setName(text);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onImdbIdChange(QString text)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setId(ImdbId(text));
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the original name has changed
 */
void MovieWidget::onOriginalNameChange(QString text)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setOriginalName(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the sorttitle has changed
 */
void MovieWidget::onSortTitleChange(QString text)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setSortTitle(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the set has changed
 */
void MovieWidget::onSetChange(QString text)
{
    if (m_movie == nullptr) {
        return;
    }
    MovieSet set;
    set.name = text;
    m_movie->setSet(set);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the tagline has changed
 */
void MovieWidget::onTaglineChange(QString text)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setTagline(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the rating has changed
 */
void MovieWidget::onRatingChange(double value)
{
    if ((m_movie == nullptr) || m_movie->ratings().isEmpty()) {
        return;
    }
    m_movie->ratings().back().rating = value;
    ui->buttonRevert->setVisible(true);
    m_movie->setChanged(true);
}

/// @brief Marks the movie as changed when the userrating has changed
void MovieWidget::onUserRatingChange(double value)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setUserRating(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the votes has changed
 */
void MovieWidget::onVotesChange(int value)
{
    if ((m_movie == nullptr) || m_movie->ratings().isEmpty()) {
        return;
    }
    m_movie->ratings().back().voteCount = value;
    ui->buttonRevert->setVisible(true);
    m_movie->setChanged(true);
}

/**
 * @brief Marks the movie as changed when the top 250 position has changed
 */
void MovieWidget::onTop250Change(int value)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setTop250(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the release date has changed
 */
void MovieWidget::onReleasedChange(QDate date)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setReleased(date);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the runtime has changed
 */
void MovieWidget::onRuntimeChange(int value)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setRuntime(std::chrono::minutes(value));
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the certification has changed
 */
void MovieWidget::onCertificationChange(QString text)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setCertification(Certification(text));
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the writer has changed
 */
void MovieWidget::onWriterChange(QString text)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setWriter(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the director has changed
 */
void MovieWidget::onDirectorChange(QString text)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setDirector(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the trailer has changed
 */
void MovieWidget::onTrailerChange(QString text)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setTrailer(text);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onWatchedClicked()
{
    if (m_movie == nullptr) {
        return;
    }

    const bool active = !ui->badgeWatched->isActive();
    ui->badgeWatched->setActive(active);

    if (active) {
        m_movie->setPlayCount(std::max(1, m_movie->playcount()));
        ui->playcount->setValue(m_movie->playcount());

        if (!m_movie->lastPlayed().isValid()) {
            ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
        }
    } else {
        m_movie->setPlayCount(0);
        m_movie->setLastPlayed(QDateTime{});
        ui->playcount->setValue(0);
    }
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the play count has changed
 */
void MovieWidget::onPlayCountChange(int value)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setPlayCount(value);
    ui->badgeWatched->setActive(value > 0);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the last watched date has changed
 */
void MovieWidget::onLastWatchedChange(QDateTime dateTime)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setLastPlayed(dateTime);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the overview has changed
 */
void MovieWidget::onOverviewChange()
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setOverview(ui->overview->toPlainText());
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the movie as changed when the outline has changed
 */
void MovieWidget::onOutlineChange()
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->setOutline(ui->outline->toPlainText());
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Updates all stream details for this movie with values from the widget
 */
void MovieWidget::onStreamDetailsEdited()
{
    StreamDetails* details = m_movie->streamDetails();
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

    m_movie->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onRemoveExtraFanart(const QByteArray& image)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->images().removeExtraFanart(image);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onRemoveExtraFanart(const QString& file)
{
    if (m_movie == nullptr) {
        return;
    }
    m_movie->images().removeExtraFanart(file);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onAddExtraFanart()
{
    if (m_movie == nullptr) {
        return;
    }

    ImageDialog::instance()->setImageType(ImageType::MovieExtraFanart);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMultiSelection(true);
    ImageDialog::instance()->setMovie(m_movie);
    ImageDialog::instance()->setDownloads(m_movie->images().backdrops());
    ImageDialog::instance()->exec(ImageType::MovieBackdrop);

    if (ImageDialog::instance()->result() == QDialog::Accepted && !ImageDialog::instance()->imageUrls().isEmpty()) {
        ui->fanarts->setLoading(true);
        emit setActionSaveEnabled(false, MainWidgets::Movies);
        m_movie->controller()->loadImages(ImageType::MovieExtraFanart, ImageDialog::instance()->imageUrls());
        ui->buttonRevert->setVisible(true);
    }
}

void MovieWidget::onExtraFanartDropped(QUrl imageUrl)
{
    if (m_movie == nullptr) {
        return;
    }
    ui->fanarts->setLoading(true);
    emit setActionSaveEnabled(false, MainWidgets::Movies);
    m_movie->controller()->loadImages(ImageType::MovieExtraFanart, QVector<QUrl>() << imageUrl);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onInsertYoutubeLink()
{
    if (Settings::instance()->useYoutubePluginUrls()) {
        ui->trailer->setText("plugin://plugin.video.youtube/?action=play_video&videoid=");
    } else {
        ui->trailer->setText("https://www.youtube.com/watch?v=");
    }
    ui->trailer->setFocus();
}

void MovieWidget::onChooseImage()
{
    if (m_movie == nullptr) {
        return;
    }

    auto image = dynamic_cast<ClosableImage*>(QObject::sender());
    if (image == nullptr) {
        return;
    }

    ImageDialog::instance()->setImageType(image->imageType());
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMovie(m_movie);
    if (image->imageType() == ImageType::MoviePoster) {
        ImageDialog::instance()->setDownloads(m_movie->images().posters());
    } else if (image->imageType() == ImageType::MovieBackdrop) {
        ImageDialog::instance()->setDownloads(m_movie->images().backdrops());
    } else if (image->imageType() == ImageType::MovieCdArt && !m_movie->images().discArts().isEmpty()) {
        ImageDialog::instance()->setDownloads(m_movie->images().discArts());
    } else {
        ImageDialog::instance()->setDownloads(QVector<Poster>());
    }
    ImageDialog::instance()->exec(image->imageType());

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, MainWidgets::Movies);
        m_movie->controller()->loadImage(image->imageType(), ImageDialog::instance()->imageUrl());
        ui->buttonRevert->setVisible(true);
    }
}

void MovieWidget::onImageDropped(ImageType imageType, QUrl imageUrl)
{
    if (m_movie == nullptr) {
        return;
    }
    emit setActionSaveEnabled(false, MainWidgets::Movies);
    m_movie->controller()->loadImage(imageType, imageUrl);
    ui->buttonRevert->setVisible(true);
}

void MovieWidget::onDeleteImage()
{
    if (m_movie == nullptr) {
        return;
    }

    auto image = dynamic_cast<ClosableImage*>(QObject::sender());
    if (image == nullptr) {
        return;
    }

    m_movie->images().removeImage(image->imageType());
    updateImages({image->imageType()});
    ui->buttonRevert->setVisible(true);
}
