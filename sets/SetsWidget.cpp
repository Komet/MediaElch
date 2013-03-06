#include "SetsWidget.h"
#include "ui_SetsWidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include "globals/Globals.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "main/MessageBox.h"
#include "sets/MovieListDialog.h"

/**
 * @brief SetsWidget::SetsWidget
 * @param parent
 */
SetsWidget::SetsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SetsWidget)
{
    ui->setupUi(this);

#if QT_VERSION >= 0x050000
    ui->sets->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->movies->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->movies->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
    ui->sets->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->movies->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->movies->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewPoster->setEnabled(false);

    QFont font = ui->setName->font();
    font.setPointSize(font.pointSize()+4);
    ui->setName->setFont(font);
#ifdef Q_OS_MAC
    QFont setsFont = ui->sets->font();
    setsFont.setPointSize(setsFont.pointSize()-2);
    ui->sets->setFont(setsFont);
#endif

    font = ui->posterResolution->font();
    #ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->posterResolution->setFont(font);
    ui->backdropResolution->setFont(font);

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();
    m_downloadManager = new DownloadManager(this);

    connect(ui->sets, SIGNAL(itemSelectionChanged()), this, SLOT(onSetSelected()));
    connect(ui->sets, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onSetNameChanged(QTableWidgetItem*)));
    connect(ui->movies, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onSortTitleChanged(QTableWidgetItem*)));
    connect(ui->buttonAddMovie, SIGNAL(clicked()), this, SLOT(onAddMovie()));
    connect(ui->buttonRemoveMovie, SIGNAL(clicked()), this, SLOT(onRemoveMovie()));
    connect(ui->poster, SIGNAL(clicked()), this, SLOT(chooseSetPoster()));
    connect(ui->backdrop, SIGNAL(clicked()), this, SLOT(chooseSetBackdrop()));
    connect(ui->buttonPreviewPoster, SIGNAL(clicked()), this, SLOT(onPreviewPoster()));
    connect(ui->buttonPreviewBackdrop, SIGNAL(clicked()), this, SLOT(onPreviewBackdrop()));
    connect(m_downloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onDownloadFinished(DownloadManagerElement)));

    ui->sets->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableContextMenu = new QMenu(ui->sets);
    QAction *actionAddSet = new QAction(tr("Add Movie Set"), this);
    QAction *actionDeleteSet = new QAction(tr("Delete Movie Set"), this);
    m_tableContextMenu->addAction(actionAddSet);
    m_tableContextMenu->addAction(actionDeleteSet);
    connect(actionAddSet, SIGNAL(triggered()), this, SLOT(onAddMovieSet()));
    connect(actionDeleteSet, SIGNAL(triggered()), this, SLOT(onRemoveMovieSet()));
    connect(ui->sets, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showSetsContextMenu(QPoint)));

    clear();
}

/**
 * @brief SetsWidget::~SetsWidget
 */
SetsWidget::~SetsWidget()
{
    delete ui;
}

/**
 * @brief Returns the splitter
 * @return The splitter
 */
QSplitter *SetsWidget::splitter()
{
    return ui->splitter;
}

void SetsWidget::showSetsContextMenu(QPoint point)
{
    m_tableContextMenu->exec(ui->sets->mapToGlobal(point));
}

/**
 * @brief Parses list of movie and constructs sets map
 */
void SetsWidget::loadSets()
{
    qDebug() << "Entered";
    emit setActionSaveEnabled(false, WidgetMovieSets);
    clear();
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewPoster->setEnabled(false);
    int currentRow = (ui->sets->currentRow() >= 0 && ui->sets->currentRow() < ui->sets->rowCount()) ? ui->sets->currentRow() : 0;
    ui->sets->clear();
    ui->sets->setRowCount(0);
    m_sets.clear();
    m_moviesToSave.clear();
    m_setPosters.clear();
    m_setBackdrops.clear();
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (!movie->set().isEmpty()) {
            if (m_sets.contains(movie->set())) {
                m_sets[movie->set()].append(movie);
            } else {
                QList<Movie*> l;
                QList<Movie*> el;
                l << movie;
                m_sets.insert(movie->set(), l);
                m_moviesToSave.insert(movie->set(), el);
                m_setPosters.insert(movie->set(), QImage());
                m_setBackdrops.insert(movie->set(), QImage());
            }
        }
    }
    foreach (const QString &set, m_addedSets) {
        if (!set.isEmpty() && !m_sets.contains(set)) {
            m_sets.insert(set, QList<Movie*>());
            m_moviesToSave.insert(set, QList<Movie*>());
            m_setPosters.insert(set, QImage());
            m_setBackdrops.insert(set, QImage());
        }
    }
    QMapIterator<QString, QList<Movie*> > it(m_sets);
    while (it.hasNext()) {
        it.next();
        int row = ui->sets->rowCount();
        ui->sets->insertRow(row);
        ui->sets->setItem(row, 0, new QTableWidgetItem(it.key()));
        ui->sets->item(row, 0)->setData(Qt::UserRole, it.key());
    }
    if (ui->sets->rowCount() > 0 && currentRow < ui->sets->rowCount())
        ui->sets->setCurrentItem(ui->sets->item(currentRow, 0));
    emit setActionSaveEnabled(true, WidgetMovieSets);
}

/**
 * @brief Called when set table selection changes
 * @see SetsWidget::loadSets
 */
void SetsWidget::onSetSelected()
{
    qDebug() << "Entered";
    int row = ui->sets->currentRow();
    qDebug() << "row=" << row << "rowCount=" << ui->sets->rowCount();
    if (row < 0 || row >= ui->sets->rowCount()) {
        clear();
        return;
    }

    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
    loadSet(setName);
}

/**
 * @brief Clears contents
 */
void SetsWidget::clear()
{
    qDebug() << "Entered";
    ui->setName->clear();
    ui->movies->clearContents();
    ui->movies->setRowCount(0);
    ui->backdropResolution->clear();
    ui->posterResolution->clear();
    m_currentBackdrop = QImage();
    m_currentPoster = QImage();
    m_addedSets.clear();
}

/**
 * @brief Fills the widget with set data
 * @param set Name of the set
 */
void SetsWidget::loadSet(QString set)
{
    qDebug() << "Entered, set=" << set;
    clear();
    ui->setName->setText(set);
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewPoster->setEnabled(false);
    ui->movies->blockSignals(true);

    foreach (Movie *movie, m_sets[set]) {
        int row = ui->movies->rowCount();
        ui->movies->insertRow(row);
        ui->movies->setItem(row, 0, new QTableWidgetItem(movie->name()));
        ui->movies->item(row, 0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->movies->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(movie));
        ui->movies->setItem(row, 1, new QTableWidgetItem(movie->sortTitle()));
    }
    ui->movies->sortByColumn(1, Qt::AscendingOrder);

    if (!m_setPosters[set].isNull()) {
        QImage poster = m_setPosters[set];
        ui->poster->setPixmap(QPixmap::fromImage(poster).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(poster.width()).arg(poster.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = poster;
    } else if (Manager::instance()->mediaCenterInterface()->hasFeature(MediaCenterFeatures::HandleMovieSetImages) &&
        !Manager::instance()->mediaCenterInterface()->movieSetPoster(set).isNull()) {
        QImage poster = Manager::instance()->mediaCenterInterface()->movieSetPoster(set);
        ui->poster->setPixmap(QPixmap::fromImage(poster).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(poster.width()).arg(poster.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = poster;
    } else {
        ui->poster->setPixmap(QPixmap(":/img/poster.png"));
        ui->buttonPreviewPoster->setEnabled(false);
    }

    if (!m_setBackdrops[set].isNull()) {
        QImage backdrop = m_setBackdrops[set];
        ui->backdrop->setPixmap(QPixmap::fromImage(backdrop).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(backdrop.width()).arg(backdrop.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = backdrop;
    } else if (Manager::instance()->mediaCenterInterface()->hasFeature(MediaCenterFeatures::HandleMovieSetImages) &&
        !Manager::instance()->mediaCenterInterface()->movieSetBackdrop(set).isNull()) {
        QImage backdrop = Manager::instance()->mediaCenterInterface()->movieSetBackdrop(set);
        ui->backdrop->setPixmap(QPixmap::fromImage(backdrop).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(backdrop.width()).arg(backdrop.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = backdrop;
    } else {
        ui->backdrop->setPixmap(QPixmap(":/img/missing_art.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->buttonPreviewBackdrop->setEnabled(false);
    }
    ui->movies->blockSignals(false);
}

/**
 * @brief Called when an item in the movies table was changed
 *        Updates movies sorttitle and reorders the table
 * @param item changed item
 */
void SetsWidget::onSortTitleChanged(QTableWidgetItem *item)
{
    qDebug() << "Entered, item->row=" << item->row() << "rowCount=" << ui->movies->rowCount();
    if (item->row() < 0 || item->row() >= ui->movies->rowCount() || item->column() != 1) {
        qDebug() << "Invalid row";
        return;
    }
    Movie *movie = ui->movies->item(item->row(), 0)->data(Qt::UserRole).value<Movie*>();
    movie->setSortTitle(item->text());
    ui->movies->sortByColumn(1, Qt::AscendingOrder);
    if (!m_moviesToSave[movie->set()].contains(movie))
        m_moviesToSave[movie->set()].append(movie);
}

/**
 * @brief Execs the MovieListDialog and (if accepted) adds a movie to the movies table,
 *        sets the setname in the movie and adds the movie to m_sets
 */
void SetsWidget::onAddMovie()
{
    qDebug() << "Entered";
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount()) {
        qDebug() << "Invalid current row";
        return;
    }
    if (MovieListDialog::instance()->exec() == QDialog::Accepted) {
        Movie *movie = MovieListDialog::instance()->selectedMovie();
        int row = ui->sets->currentRow();
        if (row < 0 || row >= ui->sets->rowCount())
            return;

        QString setName = ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
        if (movie->set() == setName)
            return;
        movie->setSet(setName);
        m_sets[setName].append(movie);
        if (!m_moviesToSave[setName].contains(movie))
            m_moviesToSave[setName].append(movie);
        loadSet(setName);
    }
}

/**
 * @brief Removes a movie from the movies table, sets and empty sorttitle and set for the movie
 *        and removes it from m_sets
 */
void SetsWidget::onRemoveMovie()
{
    qDebug() << "Entered";
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount()) {
        qDebug() << "Invalid current row in sets";
        return;
    }
    if (ui->movies->currentRow() < 0 || ui->movies->currentRow() >= ui->movies->rowCount()) {
        qDebug() << "Invalid current row in movies";
        return;
    }
    Movie *movie = ui->movies->item(ui->movies->currentRow(), 0)->data(Qt::UserRole).value<Movie*>();
    m_sets[movie->set()].removeOne(movie);
    if (!m_moviesToSave[movie->set()].contains(movie))
        m_moviesToSave[movie->set()].append(movie);
    movie->setSortTitle("");
    movie->setSet("");
    ui->movies->removeRow(ui->movies->currentRow());
}

/**
 * @brief Shows QFileDialog to choose an image, if successful sets the poster
 */
void SetsWidget::chooseSetPoster()
{
    qDebug() << "Entered";
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount()) {
        qDebug() << "Invalid current row in sets";
        return;
    }

    if (!Manager::instance()->mediaCenterInterface()->hasFeature(MediaCenterFeatures::HandleMovieSetImages))
        return;

    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
    Movie *movie = new Movie(QStringList());
    movie->setName(setName);
    ImageDialog::instance()->setImageType(TypePoster);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMovie(movie);
    ImageDialog::instance()->exec(ImageDialogType::MoviePoster);
    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        DownloadManagerElement d;
        d.movie = movie;
        d.imageType = static_cast<ImageType>(TypePoster);
        d.url = ImageDialog::instance()->imageUrl();
        m_downloadManager->addDownload(d);
        ui->poster->setPixmap(QPixmap());
        ui->poster->setMovie(m_loadingMovie);
        ui->buttonPreviewPoster->setEnabled(false);
    }
}

/**
 * @brief Shows QFileDialog to choose an image, if successful sets the backdrop
 */
void SetsWidget::chooseSetBackdrop()
{
    qDebug() << "Entered";
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount()) {
        qDebug() << "Invalid current row in sets";
        return;
    }

    if (!Manager::instance()->mediaCenterInterface()->hasFeature(MediaCenterFeatures::HandleMovieSetImages))
        return;

    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
    Movie *movie = new Movie(QStringList());
    movie->setName(setName);
    ImageDialog::instance()->setImageType(TypeBackdrop);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMovie(movie);
    ImageDialog::instance()->exec(ImageDialogType::MovieBackdrop);
    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        DownloadManagerElement d;
        d.movie = movie;
        d.imageType = static_cast<ImageType>(TypeBackdrop);
        d.url = ImageDialog::instance()->imageUrl();
        m_downloadManager->addDownload(d);
        ui->backdrop->setPixmap(QPixmap());
        ui->backdrop->setMovie(m_loadingMovie);
        ui->buttonPreviewBackdrop->setEnabled(false);
    }
}

/**
 * @brief Saves changed movies in this set
 */
void SetsWidget::saveSet()
{
    qDebug() << "Entered";
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount()) {
        qDebug() << "Invalid current row in sets";
        return;
    }
    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
    foreach (Movie *movie, m_moviesToSave[setName])
        movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
    m_moviesToSave[setName].clear();

    if (!m_setPosters[setName].isNull() && Manager::instance()->mediaCenterInterface()->hasFeature(MediaCenterFeatures::HandleMovieSetImages)) {
        Manager::instance()->mediaCenterInterface()->saveMovieSetPoster(setName, m_setPosters[setName]);
        m_setPosters[setName] = QImage();
    }
    if (!m_setBackdrops[setName].isNull() && Manager::instance()->mediaCenterInterface()->hasFeature(MediaCenterFeatures::HandleMovieSetImages)) {
        Manager::instance()->mediaCenterInterface()->saveMovieSetBackdrop(setName, m_setBackdrops[setName]);
        m_setBackdrops[setName] = QImage();
    }

    MessageBox::instance()->showMessage(tr("<b>\"%1\"</b> Saved").arg(setName));
}

/**
 * @brief Shows a full preview of the current backdrop
 */
void SetsWidget::onPreviewBackdrop()
{
    qDebug() << "Entered";
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentBackdrop));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows a full preview of the current poster
 */
void SetsWidget::onPreviewPoster()
{
    qDebug() << "Entered";
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentPoster));
    ImagePreviewDialog::instance()->exec();
}

void SetsWidget::onAddMovieSet()
{
    QString setName = tr("New Movie Set");
    int adder = -1;
    bool setExists;
    do {
        adder++;
        setExists = false;
        for (int i=0, n=ui->sets->rowCount() ; i<n ; ++i) {
            if ((adder == 0 && ui->sets->item(i, 0)->text() == setName) ||
                (adder > 0 && ui->sets->item(i, 0)->text() == QString("%1 %2").arg(setName).arg(adder))) {
                setExists = true;
                break;
            }
        }
    } while(setExists);

    if (adder > 0)
        setName.append(QString(" %1").arg(adder));

    m_addedSets << setName;

    QList<Movie*> l;
    QList<Movie*> el;
    m_sets.insert(setName, l);
    m_moviesToSave.insert(setName, el);
    m_setPosters.insert(setName, QImage());
    m_setBackdrops.insert(setName, QImage());

    ui->sets->blockSignals(true);
    int row = ui->sets->rowCount();
    ui->sets->insertRow(row);
    ui->sets->setItem(row, 0, new QTableWidgetItem(setName));
    ui->sets->item(row, 0)->setData(Qt::UserRole, setName);
    ui->sets->blockSignals(false);
}

void SetsWidget::onRemoveMovieSet()
{
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount()) {
        qWarning() << "Invalid row" << ui->sets->currentRow();
        return;
    }

    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->text();
    QString origSetName = ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
    ui->sets->removeRow(ui->sets->currentRow());

    foreach (Movie *movie, m_sets[origSetName]) {
        movie->setSet("");
        movie->setSortTitle("");
    }
    m_sets.remove(setName);
    m_setPosters.remove(setName);
    m_setBackdrops.remove(setName);
    m_addedSets.removeOne(setName);
}

void SetsWidget::onSetNameChanged(QTableWidgetItem *item)
{
    QString newName = item->text();
    QString origSetName = item->data(Qt::UserRole).toString();
    if (newName == origSetName)
        return;

    for (int i=0, n=ui->sets->rowCount() ; i<n ; ++i) {
        if (i != item->row() && ui->sets->item(i, 0)->text() == newName) {
            ui->sets->removeRow(i);
            break;
        }
    }

    if (!m_moviesToSave.contains(newName))
        m_moviesToSave.insert(newName, QList<Movie*>());

    foreach (Movie *movie, m_sets[origSetName]) {
        m_moviesToSave[newName].append(movie);
        movie->setSet(newName);
    }

    m_moviesToSave[origSetName].clear();

    if (!m_sets.contains(newName))
        m_sets[newName].append(QList<Movie*>());

    m_sets[newName].append(m_sets[origSetName]);
    m_sets.remove(origSetName);

    if (!m_setPosters.contains(newName))
        m_setPosters.insert(newName, QImage());
    if (!m_setBackdrops.contains(newName))
        m_setBackdrops.insert(newName, QImage());

    if (m_addedSets.contains(newName)) {
        m_addedSets.removeOne(origSetName);
        if (!m_addedSets.contains(newName))
            m_addedSets.append(newName);
    }

    loadSet(newName);
}

void SetsWidget::onDownloadFinished(DownloadManagerElement elem)
{
    QString setName = elem.movie->name();
    if (elem.imageType == TypePoster) {
        if (m_setPosters.contains(setName))
            m_setPosters[setName] = QImage::fromData(elem.data);
        if (ui->sets->currentRow() >= 0 && ui->sets->currentRow() < ui->sets->rowCount() && ui->sets->item(ui->sets->currentRow(), 0)->text() == setName)
            loadSet(setName);
    } else if (elem.imageType == TypeBackdrop) {
        if (m_setBackdrops.contains(setName))
            m_setBackdrops[setName] = QImage::fromData(elem.data);
        if (ui->sets->currentRow() >= 0 && ui->sets->currentRow() < ui->sets->rowCount() && ui->sets->item(ui->sets->currentRow(), 0)->text() == setName)
            loadSet(setName);
    }
    if (elem.movie)
        delete elem.movie;
}
