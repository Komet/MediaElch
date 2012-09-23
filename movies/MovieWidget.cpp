#include "MovieWidget.h"
#include "ui_MovieWidget.h"

#include <QDoubleValidator>
#include <QFileDialog>
#include <QIntValidator>
#include <QMovie>
#include <QPainter>
#include <QScrollBar>
#include "globals/Globals.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "main/MessageBox.h"
#include "globals/ImageDialog.h"
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
    ui->movieName->clear();
    ui->actors->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->genres->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->countries->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->studios->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->buttonPreviewPoster->setEnabled(false);
    ui->buttonPreviewBackdrop->setEnabled(false);

    QFont font = ui->movieName->font();
    font.setPointSize(font.pointSize()+4);
    ui->movieName->setFont(font);

    font = ui->posterResolution->font();
    #ifdef Q_WS_WIN
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->posterResolution->setFont(font);
    ui->backdropResolution->setFont(font);
    ui->actorResolution->setFont(font);

    m_movie = 0;
    m_posterDownloadManager = new DownloadManager(this);

    connect(ui->poster, SIGNAL(clicked()), this, SLOT(chooseMoviePoster()));
    connect(ui->backdrop, SIGNAL(clicked()), this, SLOT(chooseMovieBackdrop()));
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(posterDownloadFinished(DownloadManagerElement)));
    connect(m_posterDownloadManager, SIGNAL(downloadsLeft(int, DownloadManagerElement)), this, SLOT(actorDownloadsLeft(int, DownloadManagerElement)));
    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(movieNameChanged(QString)));
    connect(ui->buttonAddActor, SIGNAL(clicked()), this, SLOT(addActor()));
    connect(ui->buttonRemoveActor, SIGNAL(clicked()), this, SLOT(removeActor()));
    connect(ui->buttonAddCountry, SIGNAL(clicked()), this, SLOT(addCountry()));
    connect(ui->buttonRemoveCountry, SIGNAL(clicked()), this, SLOT(removeCountry()));
    connect(ui->buttonAddGenre, SIGNAL(clicked()), this, SLOT(addGenre()));
    connect(ui->buttonRemoveGenre, SIGNAL(clicked()), this, SLOT(removeGenre()));
    connect(ui->buttonAddStudio, SIGNAL(clicked()), this, SLOT(addStudio()));
    connect(ui->buttonRemoveStudio, SIGNAL(clicked()), this, SLOT(removeStudio()));
    connect(ui->actors, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onActorEdited(QTableWidgetItem*)));
    connect(ui->actors, SIGNAL(itemSelectionChanged()), this, SLOT(onActorChanged()));
    connect(ui->genres, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onGenreEdited(QTableWidgetItem*)));
    connect(ui->studios, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onStudioEdited(QTableWidgetItem*)));
    connect(ui->countries, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onCountryEdited(QTableWidgetItem*)));
    connect(ui->buttonPreviewPoster, SIGNAL(clicked()), this, SLOT(onPreviewPoster()));
    connect(ui->buttonPreviewBackdrop, SIGNAL(clicked()), this, SLOT(onPreviewBackdrop()));
    connect(ui->actor, SIGNAL(clicked()), this, SLOT(onChangeActorImage()));
    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();

    setDisabledTrue();
    this->clear();

    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    // Connect GUI change events to movie object
    connect(ui->name, SIGNAL(textEdited(QString)), this, SLOT(onNameChange(QString)));
    connect(ui->originalName, SIGNAL(textEdited(QString)), this, SLOT(onOriginalNameChange(QString)));
    connect(ui->tagline, SIGNAL(textEdited(QString)), this, SLOT(onTaglineChange(QString)));
    connect(ui->rating, SIGNAL(valueChanged(double)), this, SLOT(onRatingChange(double)));
    connect(ui->trailer, SIGNAL(textEdited(QString)), this, SLOT(onTrailerChange(QString)));
    connect(ui->runtime, SIGNAL(valueChanged(int)), this, SLOT(onRuntimeChange(int)));
    connect(ui->playcount, SIGNAL(valueChanged(int)), this, SLOT(onPlayCountChange(int)));
    connect(ui->certification, SIGNAL(editTextChanged(QString)), this, SLOT(onCertificationChange(QString)));
    connect(ui->set, SIGNAL(editTextChanged(QString)), this, SLOT(onSetChange(QString)));
    connect(ui->watched, SIGNAL(stateChanged(int)), this, SLOT(onWatchedChange(int)));
    connect(ui->released, SIGNAL(dateChanged(QDate)), this, SLOT(onReleasedChange(QDate)));
    connect(ui->lastPlayed, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(onLastWatchedChange(QDateTime)));
    connect(ui->overview, SIGNAL(textChanged()), this, SLOT(onOverviewChange()));

    QPixmap zoomIn(":/img/zoom_in.png");
    QPainter p;
    p.begin(&zoomIn);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(zoomIn.rect(), QColor(0, 0, 0, 150));
    p.end();
    ui->buttonPreviewBackdrop->setIcon(QIcon(zoomIn));
    ui->buttonPreviewPoster->setIcon(QIcon(zoomIn));

    QPixmap revert(":/img/arrow_circle_left.png");
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
    m_savingWidget->move(size().width()/2-m_savingWidget->width(), height()/2-m_savingWidget->height());
    QWidget::resizeEvent(event);
}

/**
 * @brief Clears all contents of the widget
 */
void MovieWidget::clear()
{
    qDebug() << "Entered";
    ui->set->clear();
    ui->certification->clear();
    ui->movieName->clear();
    ui->files->clear();
    ui->name->clear();
    ui->originalName->clear();
    ui->tagline->clear();
    ui->rating->clear();
    ui->released->setDate(QDate::currentDate());
    ui->runtime->clear();
    ui->trailer->clear();
    ui->playcount->clear();
    ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
    ui->overview->clear();
    ui->actors->setRowCount(0);
    ui->genres->setRowCount(0);
    ui->studios->setRowCount(0);
    ui->countries->setRowCount(0);
    ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
    ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->actor->setPixmap(QPixmap(":/img/man.png"));
    ui->tabWidget->setCurrentIndex(0);
    ui->posterResolution->setText("");
    ui->backdropResolution->setText("");
    ui->actorResolution->setText("");
    ui->buttonRevert->setVisible(false);
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
    if (movie && movie->downloadsInProgress()) {
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
    movie->loadData(Manager::instance()->mediaCenterInterface());
    m_movie = movie;
    movie->loadImages(Manager::instance()->mediaCenterInterface());
    updateMovieInfo();
    if (movie->downloadsInProgress())
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
        m_movie->loadData(MovieSearch::instance()->scraperId(), Manager::instance()->scrapers().at(MovieSearch::instance()->scraperNo()),
                          MovieSearch::instance()->infosToLoad());
        connect(this->m_movie, SIGNAL(loaded(Movie*)), this, SLOT(loadDone(Movie*)), Qt::UniqueConnection);
    } else {
        emit setActionSearchEnabled(true, WidgetMovies);
        emit setActionSaveEnabled(true, WidgetMovies);
    }
}

/**
 * @brief Called when the search widget finishes
 * Updates infos and starts downloads
 * @param movie Movie
 */
void MovieWidget::loadDone(Movie *movie)
{
    qDebug() << "Entered";
    if (m_movie == 0) {
        qDebug() << "My movie is invalid";
        return;
    }

    if (m_movie == movie)
        updateMovieInfo();
    else
        qDebug() << "Movie has changed";
    int downloadsSize = 0;

    if (MovieSearch::instance()->infosToLoad().contains(MovieScraperInfos::Poster) && movie->posters().size() > 0) {
        emit setActionSaveEnabled(false, WidgetMovies);
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = movie->posters().at(0).originalUrl;
        d.movie = movie;
        m_posterDownloadManager->addDownload(d);
        if (m_movie == movie) {
            ui->poster->setPixmap(QPixmap());
            ui->poster->setMovie(m_loadingMovie);
        }
        downloadsSize++;
    }

    if (MovieSearch::instance()->infosToLoad().contains(MovieScraperInfos::Backdrop) &&movie->backdrops().size() > 0) {
        emit setActionSaveEnabled(false, WidgetMovies);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = movie->backdrops().at(0).originalUrl;
        d.movie = movie;
        m_posterDownloadManager->addDownload(d);
        if (m_movie == movie) {
            ui->backdrop->setPixmap(QPixmap());
            ui->backdrop->setMovie(m_loadingMovie);
        }
        downloadsSize++;
    }

    if (MovieSearch::instance()->infosToLoad().contains(MovieScraperInfos::Actors)) {
        QList<Actor*> actors = movie->actorsPointer();
        for (int i=0, n=actors.size() ; i<n ; i++) {
            if (actors.at(i)->thumb.isEmpty())
                continue;
            DownloadManagerElement d;
            d.imageType = TypeActor;
            d.url = QUrl(actors.at(i)->thumb);
            d.actor = actors.at(i);
            d.movie = movie;
            m_posterDownloadManager->addDownload(d);
            downloadsSize++;
        }
    }
    if (downloadsSize > 0)
        emit actorDownloadStarted(tr("Downloading Missing Actor Images..."), Constants::MovieProgressMessageId+movie->movieId());
    else if (m_movie == movie)
        setEnabledTrue();

    movie->setDownloadsInProgress(downloadsSize > 0);
    movie->setDownloadsSize(downloadsSize);
    ui->buttonRevert->setVisible(true);

    connect(m_posterDownloadManager, SIGNAL(allDownloadsFinished(Movie*)), this, SLOT(downloadActorsFinished(Movie*)), Qt::UniqueConnection);
}

/**
 * @brief Updates the contents of the widget with the current movie infos
 */
void MovieWidget::updateMovieInfo()
{
    qDebug() << "Entered";
    if (m_movie == 0) {
        qDebug() << "My movie is invalid";
        return;
    }

    ui->rating->blockSignals(true);
    ui->runtime->blockSignals(true);
    ui->playcount->blockSignals(true);
    ui->set->blockSignals(true);
    ui->certification->blockSignals(true);
    ui->watched->blockSignals(true);
    ui->released->blockSignals(true);
    ui->lastPlayed->blockSignals(true);
    ui->overview->blockSignals(true);
    ui->actors->blockSignals(true);
    ui->genres->blockSignals(true);
    ui->studios->blockSignals(true);
    ui->countries->blockSignals(true);

    clear();

    ui->files->setText(m_movie->files().join(", "));
    ui->files->setToolTip(m_movie->files().join("\n"));
    ui->name->setText(m_movie->name());
    ui->movieName->setText(m_movie->name());
    ui->originalName->setText(m_movie->originalName());
    ui->tagline->setText(m_movie->tagline());
    ui->rating->setValue(m_movie->rating());
    ui->released->setDate(m_movie->released());
    ui->runtime->setValue(m_movie->runtime());
    ui->trailer->setText(m_movie->trailer().toString());
    ui->playcount->setValue(m_movie->playcount());
    ui->lastPlayed->setDateTime(m_movie->lastPlayed());
    ui->overview->setPlainText(m_movie->overview());
    ui->watched->setChecked(m_movie->watched());

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

    ui->genres->blockSignals(true);
    foreach (QString *genre, m_movie->genresPointer()) {
        int row = ui->genres->rowCount();
        ui->genres->insertRow(row);
        ui->genres->setItem(row, 0, new QTableWidgetItem(*genre));
        ui->genres->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(genre));
    }
    ui->genres->blockSignals(false);

    ui->studios->blockSignals(true);
    foreach (QString *studio, m_movie->studiosPointer()) {
        int row = ui->studios->rowCount();
        ui->studios->insertRow(row);
        ui->studios->setItem(row, 0, new QTableWidgetItem(*studio));
        ui->studios->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(studio));
    }
    ui->studios->blockSignals(false);

    ui->countries->blockSignals(true);
    foreach (QString *country, m_movie->countriesPointer()) {
        int row = ui->countries->rowCount();
        ui->countries->insertRow(row);
        ui->countries->setItem(row, 0, new QTableWidgetItem(*country));
        ui->countries->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(country));
    }
    ui->countries->blockSignals(false);

    if (!m_movie->posterImage()->isNull()) {
        ui->poster->setPixmap(QPixmap::fromImage(*m_movie->posterImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(m_movie->posterImage()->width()).arg(m_movie->posterImage()->height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = *m_movie->posterImage();
    } else {
        ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
        ui->posterResolution->setText("");
        ui->buttonPreviewPoster->setEnabled(false);
    }

    if (!m_movie->backdropImage()->isNull()) {
        ui->backdrop->setPixmap(QPixmap::fromImage(*m_movie->backdropImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(m_movie->backdropImage()->width()).arg(m_movie->backdropImage()->height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = *m_movie->backdropImage();
    } else {
        ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText("");
        ui->buttonPreviewBackdrop->setEnabled(false);
    }

    ui->rating->blockSignals(false);
    ui->runtime->blockSignals(false);
    ui->playcount->blockSignals(false);
    ui->watched->blockSignals(false);
    ui->released->blockSignals(false);
    ui->lastPlayed->blockSignals(false);
    ui->overview->blockSignals(false);
    ui->actors->blockSignals(false);
    ui->genres->blockSignals(false);
    ui->studios->blockSignals(false);
    ui->countries->blockSignals(false);

    emit setActionSaveEnabled(true, WidgetMovies);

    ui->buttonRevert->setVisible(m_movie->hasChanged());
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
    ImageDialog::instance()->setDownloads(m_movie->posters());
    ImageDialog::instance()->exec(ImageDialogType::MoviePoster);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetMovies);
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = ImageDialog::instance()->imageUrl();
        d.movie = m_movie;
        m_posterDownloadManager->addDownload(d);
        ui->poster->setPixmap(QPixmap());
        ui->poster->setMovie(m_loadingMovie);
        ui->buttonPreviewPoster->setEnabled(false);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Shows the MovieImageDialog and after successful execution starts backdrop download
 */
void MovieWidget::chooseMovieBackdrop()
{
    qDebug() << "Entered";
    if (m_movie == 0) {
        qDebug() << "My movie is invalid";
        return;
    }

    ImageDialog::instance()->setImageType(TypeBackdrop);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setDownloads(m_movie->backdrops());
    ImageDialog::instance()->exec(ImageDialogType::MovieBackdrop);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false, WidgetMovies);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = ImageDialog::instance()->imageUrl();
        d.movie = m_movie;
        m_posterDownloadManager->addDownload(d);
        ui->backdrop->setPixmap(QPixmap());
        ui->backdrop->setMovie(m_loadingMovie);
        ui->buttonPreviewBackdrop->setEnabled(false);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Adjusts the size of the backdrop to common values (1080p or 720p) and shows the image
 * @param elem Downloaded element
 */
void MovieWidget::posterDownloadFinished(DownloadManagerElement elem)
{
    qDebug() << "Entered";
    if (elem.imageType == TypePoster) {
        qDebug() << "Got a poster";
        if (m_movie == elem.movie) {
            ui->poster->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->posterResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewPoster->setEnabled(true);
            m_currentPoster = elem.image;
        }
        elem.movie->setPosterImage(elem.image);
    } else if (elem.imageType == TypeBackdrop) {
        qDebug() << "Got a backdrop";
        if ((elem.image.width() != 1920 || elem.image.height() != 1080) &&
            elem.image.width() > 1915 && elem.image.width() < 1925 && elem.image.height() > 1075 && elem.image.height() < 1085)
            elem.image = elem.image.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        if ((elem.image.width() != 1280 || elem.image.height() != 720) &&
            elem.image.width() > 1275 && elem.image.width() < 1285 && elem.image.height() > 715 && elem.image.height() < 725)
            elem.image = elem.image.scaled(1280, 720, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        if (m_movie == elem.movie) {
            ui->backdrop->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->backdropResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewBackdrop->setEnabled(true);
            m_currentBackdrop = elem.image;
        }
        elem.movie->setBackdropImage(elem.image);
    }
    if (m_posterDownloadManager->downloadQueueSize() == 0) {
        emit setActionSaveEnabled(true, WidgetMovies);
        elem.movie->setDownloadsInProgress(false);
    }
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Saves movie information
 */
void MovieWidget::saveInformation()
{
    qDebug() << "Entered";
    setDisabledTrue();
    m_savingWidget->show();
    m_movie->saveData(Manager::instance()->mediaCenterInterface());
    m_movie->loadData(Manager::instance()->mediaCenterInterface(), true);
    updateMovieInfo();
    setEnabledTrue();
    m_savingWidget->hide();
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

    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->hasChanged()) {
            movie->saveData(Manager::instance()->mediaCenterInterface());
            movie->loadData(Manager::instance()->mediaCenterInterface(), true);
            if (m_movie == movie)
                updateMovieInfo();
        }
    }
    setEnabledTrue();
    m_savingWidget->hide();
    MessageBox::instance()->showMessage(tr("All Movies Saved"));
    ui->buttonRevert->setVisible(false);
}

/**
 * @brief Revert changes for current movie
 */
void MovieWidget::onRevertChanges()
{
    qDebug() << "Entered";
    m_movie->loadData(Manager::instance()->mediaCenterInterface(), true);
    m_movie->loadImages(Manager::instance()->mediaCenterInterface(), true);
    updateMovieInfo();
}

/**
 * @brief Toggles enabled state of the widget
 * @param movie
 */
void MovieWidget::downloadActorsFinished(Movie *movie)
{
    qDebug() << "Entered, movie=" << movie->name();
    emit actorDownloadFinished(Constants::MovieProgressMessageId+movie->movieId());
    if (movie == m_movie)
        setEnabledTrue();
    else
        qDebug() << "Movie has changed";
    movie->setDownloadsInProgress(false);
}

/**
 * @brief Emits the progress signal
 * @param left Number of downloads left
 * @param elem Current downloaded element
 */
void MovieWidget::actorDownloadsLeft(int left, DownloadManagerElement elem)
{
    emit actorDownloadProgress(elem.movie->downloadsSize()-left, elem.movie->downloadsSize(), Constants::MovieProgressMessageId+elem.movie->movieId());
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
 * @brief Adds a genre
 */
void MovieWidget::addGenre()
{
    QString g = tr("Unknown Genre");
    m_movie->addGenre(g);
    QString *genre = m_movie->genresPointer().last();

    ui->genres->blockSignals(true);
    int row = ui->genres->rowCount();
    ui->genres->insertRow(row);
    ui->genres->setItem(row, 0, new QTableWidgetItem(g));
    ui->genres->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(genre));
    ui->genres->scrollToBottom();
    ui->genres->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a genre
 */
void MovieWidget::removeGenre()
{
    int row = ui->genres->currentRow();
    if (row < 0 || row >= ui->genres->rowCount() || !ui->genres->currentItem()->isSelected())
        return;

    QString *genre = ui->genres->item(row, 0)->data(Qt::UserRole).value<QString*>();
    m_movie->removeGenre(genre);
    ui->genres->blockSignals(true);
    ui->genres->removeRow(row);
    ui->genres->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Stores changed values for a genre
 * @param item Edited item
 */
void MovieWidget::onGenreEdited(QTableWidgetItem *item)
{
    QString *genre = ui->genres->item(item->row(), 0)->data(Qt::UserRole).value<QString*>();
    genre->clear();
    genre->append(item->text());
    m_movie->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Adds a studio
 */
void MovieWidget::addStudio()
{
    QString s = tr("Unknown Studio");
    m_movie->addStudio(s);
    QString *studio = m_movie->studiosPointer().last();

    ui->studios->blockSignals(true);
    int row = ui->studios->rowCount();
    ui->studios->insertRow(row);
    ui->studios->setItem(row, 0, new QTableWidgetItem(s));
    ui->studios->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(studio));
    ui->studios->scrollToBottom();
    ui->studios->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a studio
 */
void MovieWidget::removeStudio()
{
    int row = ui->studios->currentRow();
    if (row < 0 || row >= ui->studios->rowCount() || !ui->studios->currentItem()->isSelected())
        return;

    QString *studio = ui->studios->item(row, 0)->data(Qt::UserRole).value<QString*>();
    m_movie->removeStudio(studio);
    ui->studios->blockSignals(true);
    ui->studios->removeRow(row);
    ui->studios->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Stores changed values for a studio
 * @param item Edited item
 */
void MovieWidget::onStudioEdited(QTableWidgetItem *item)
{
    QString *studio = ui->studios->item(item->row(), 0)->data(Qt::UserRole).value<QString*>();
    studio->clear();
    studio->append(item->text());
    m_movie->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Adds a country
 */
void MovieWidget::addCountry()
{
    QString c = tr("Unknown Country");
    m_movie->addCountry(c);
    QString *country = m_movie->countriesPointer().last();

    ui->countries->blockSignals(true);
    int row = ui->countries->rowCount();
    ui->countries->insertRow(row);
    ui->countries->setItem(row, 0, new QTableWidgetItem(c));
    ui->countries->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(country));
    ui->countries->scrollToBottom();
    ui->countries->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a country
 */
void MovieWidget::removeCountry()
{
    int row = ui->countries->currentRow();
    if (row < 0 || row >= ui->countries->rowCount() || !ui->countries->currentItem()->isSelected())
        return;

    QString *country = ui->countries->item(row, 0)->data(Qt::UserRole).value<QString*>();
    m_movie->removeCountry(country);
    ui->countries->blockSignals(true);
    ui->countries->removeRow(row);
    ui->countries->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Stores changed values for a country
 * @param item Edited item
 */
void MovieWidget::onCountryEdited(QTableWidgetItem *item)
{
    QString *country = ui->countries->item(item->row(), 0)->data(Qt::UserRole).value<QString*>();
    country->clear();
    country->append(item->text());
    m_movie->setChanged(true);
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
 * @brief Shows the image of the selected actor
 */
void MovieWidget::onActorChanged()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount() ||
        ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        ui->actor->setPixmap(QPixmap(":/img/man.png"));
        ui->actorResolution->setText("");
        return;
    }

    Actor *actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
    if (actor->image.isNull()) {
        ui->actor->setPixmap(QPixmap(":/img/man.png"));
        ui->actorResolution->setText("");
        return;
    }
    ui->actor->setPixmap(QPixmap::fromImage(actor->image).scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->actorResolution->setText(QString("%1 x %2").arg(actor->image.width()).arg(actor->image.height()));
    ui->buttonRevert->setVisible(true);
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
        QImage img(fileName);
        if (!img.isNull()) {
            Actor *actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
            actor->image.load(fileName);
            actor->imageHasChanged = true;
            onActorChanged();
            m_movie->setChanged(true);
        }
    }
    ui->buttonRevert->setVisible(true);
}

/*** Pass GUI events to movie object ***/

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
