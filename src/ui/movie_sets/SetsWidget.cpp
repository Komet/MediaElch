#include "SetsWidget.h"
#include "ui_SetsWidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include "globals/DownloadManager.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "movies/Movie.h"
#include "ui/movie_sets/MovieListDialog.h"
#include "ui/notifications/NotificationBox.h"

SetsWidget::SetsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::SetsWidget)
{
    ui->setupUi(this);

    ui->sets->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->movies->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->movies->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewPoster->setEnabled(false);

#ifndef Q_OS_MAC
    QFont nameFont = ui->setName->font();
    nameFont.setPointSize(nameFont.pointSize() - 4);
    ui->setName->setFont(nameFont);
#endif

    helper::applyStyle(ui->movies);
    helper::applyStyle(ui->label_13);
    helper::applyStyle(ui->label_14);
    helper::applyStyle(ui->posterResolution);
    helper::applyStyle(ui->backdropResolution);
    helper::applyStyle(ui->groupBox_3);
    helper::applyEffect(ui->groupBox_3);

    m_loadingMovie = new QMovie(":/img/spinner.gif", QByteArray(), this);
    m_loadingMovie->start();
    m_downloadManager = new DownloadManager(this);

    // clang-format off
    connect(ui->sets,                  &QTableWidget::itemSelectionChanged,   this, &SetsWidget::onSetSelected);
    connect(ui->sets,                  &QTableWidget::itemChanged,            this, &SetsWidget::onSetNameChanged);
    connect(ui->movies,                &QTableWidget::itemChanged,            this, &SetsWidget::onSortTitleChanged);
    connect(ui->movies,                &QTableWidget::itemDoubleClicked,      this, &SetsWidget::onJumpToMovie);
    connect(ui->buttonAddMovie,        &QAbstractButton::clicked,             this, &SetsWidget::onAddMovie);
    connect(ui->buttonRemoveMovie,     &QAbstractButton::clicked,             this, &SetsWidget::onRemoveMovie);
    connect(ui->poster,                &MyLabel::clicked,                     this, &SetsWidget::chooseSetPoster);
    connect(ui->backdrop,              &MyLabel::clicked,                     this, &SetsWidget::chooseSetBackdrop);
    connect(ui->buttonPreviewPoster,   &QAbstractButton::clicked,             this, &SetsWidget::onPreviewPoster);
    connect(ui->buttonPreviewBackdrop, &QAbstractButton::clicked,             this, &SetsWidget::onPreviewBackdrop);
    connect(m_downloadManager,         &DownloadManager::sigDownloadFinished, this, &SetsWidget::onDownloadFinished, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    // clang-format on

    ui->sets->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableContextMenu = new QMenu(ui->sets);
    auto* actionAddSet = new QAction(tr("Add Movie Set"), this);
    auto* actionDeleteSet = new QAction(tr("Delete Movie Set"), this);
    m_tableContextMenu->addAction(actionAddSet);
    m_tableContextMenu->addAction(actionDeleteSet);
    connect(actionAddSet, &QAction::triggered, this, &SetsWidget::onAddMovieSet);
    connect(actionDeleteSet, &QAction::triggered, this, &SetsWidget::onRemoveMovieSet);
    connect(ui->sets, &QWidget::customContextMenuRequested, this, &SetsWidget::showSetsContextMenu);

    clear();

    QPixmap pixmap =
        QPixmap(":/img/placeholders/poster.png")
            .scaled(QSize(160, 260) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
    ui->poster->setPixmap(pixmap);

    QPixmap pixmap2 =
        QPixmap(":/img/placeholders/fanart.png")
            .scaled(QSize(160, 72) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    helper::setDevicePixelRatio(pixmap2, helper::devicePixelRatio(this));
    ui->backdrop->setPixmap(pixmap2);
}

/**
 * \brief SetsWidget::~SetsWidget
 */
SetsWidget::~SetsWidget()
{
    delete ui;
}

/**
 * \brief Returns the splitter
 * \return The splitter
 */
QSplitter* SetsWidget::splitter()
{
    return ui->splitter;
}

void SetsWidget::showSetsContextMenu(QPoint point)
{
    m_tableContextMenu->exec(ui->sets->mapToGlobal(point));
}

/**
 * \brief Parses list of movie and constructs sets map
 */
void SetsWidget::loadSets()
{
    emit setActionSaveEnabled(false, MainWidgets::MovieSets);
    clear();
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewPoster->setEnabled(false);
    int currentRow =
        (ui->sets->currentRow() >= 0 && ui->sets->currentRow() < ui->sets->rowCount()) ? ui->sets->currentRow() : 0;
    ui->sets->clear();
    ui->sets->setRowCount(0);
    m_sets.clear();
    m_moviesToSave.clear();
    m_setPosters.clear();
    m_setBackdrops.clear();
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (!movie->set().name.isEmpty()) {
            if (m_sets.contains(movie->set().name)) {
                m_sets[movie->set().name].append(movie);
            } else {
                QVector<Movie*> l;
                QVector<Movie*> el;
                l << movie;
                m_sets.insert(movie->set().name, l);
                m_moviesToSave.insert(movie->set().name, el);
                m_setPosters.insert(movie->set().name, QImage());
                m_setBackdrops.insert(movie->set().name, QImage());
            }
        }
    }
    for (const QString& set : m_addedSets) {
        if (!set.isEmpty() && !m_sets.contains(set)) {
            m_sets.insert(set, QVector<Movie*>());
            m_moviesToSave.insert(set, QVector<Movie*>());
            m_setPosters.insert(set, QImage());
            m_setBackdrops.insert(set, QImage());
        }
    }
    QMapIterator<QString, QVector<Movie*>> it(m_sets);
    while (it.hasNext()) {
        it.next();
        int row = ui->sets->rowCount();
        ui->sets->insertRow(row);
        ui->sets->setItem(row, 0, new QTableWidgetItem(it.key()));
        ui->sets->item(row, 0)->setData(Qt::UserRole, it.key());
    }
    if (ui->sets->rowCount() > 0 && currentRow < ui->sets->rowCount()) {
        ui->sets->setCurrentItem(ui->sets->item(currentRow, 0));
    }
    emit setActionSaveEnabled(true, MainWidgets::MovieSets);
}

/**
 * \brief Called when set table selection changes
 * \see SetsWidget::loadSets
 */
void SetsWidget::onSetSelected()
{
    int row = ui->sets->currentRow();
    qDebug() << "row=" << row << "rowCount=" << ui->sets->rowCount();
    if (row < 0 || row >= ui->sets->rowCount()) {
        clear();
        return;
    }

    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->text();
    loadSet(setName);
}

/**
 * \brief Clears contents
 */
void SetsWidget::clear()
{
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
 * \brief Fills the widget with set data
 * \param set Name of the set
 */
void SetsWidget::loadSet(QString set)
{
    qDebug() << "Entered, set=" << set;
    clear();
    ui->setName->setText(set);
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewPoster->setEnabled(false);
    ui->movies->blockSignals(true);

    for (Movie* movie : m_sets[set]) {
        int row = ui->movies->rowCount();
        ui->movies->insertRow(row);
        ui->movies->setItem(row, 0, new QTableWidgetItem(movie->name()));
        ui->movies->item(row, 0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        ui->movies->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(movie));
        ui->movies->setItem(row, 1, new QTableWidgetItem(movie->sortTitle()));
    }
    ui->movies->sortByColumn(1, Qt::AscendingOrder);

    if (!m_setPosters[set].isNull()) {
        QImage poster = m_setPosters[set];
        QPixmap pixmap = QPixmap::fromImage(poster).scaled(
            QSize(200, 300) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
        ui->poster->setPixmap(pixmap);
        ui->posterResolution->setText(QString("%1x%2").arg(poster.width()).arg(poster.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = poster;
    } else if (!Manager::instance()->mediaCenterInterface()->movieSetPoster(set).isNull()) {
        QImage poster = Manager::instance()->mediaCenterInterface()->movieSetPoster(set);
        QPixmap pixmap = QPixmap::fromImage(poster).scaled(
            QSize(200, 300) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
        ui->poster->setPixmap(pixmap);
        ui->posterResolution->setText(QString("%1x%2").arg(poster.width()).arg(poster.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = poster;
    } else {
        QPixmap pixmap =
            QPixmap(":/img/placeholders/poster.png")
                .scaled(
                    QSize(120, 120) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
        ui->poster->setPixmap(pixmap);
        ui->buttonPreviewPoster->setEnabled(false);
    }

    if (!m_setBackdrops[set].isNull()) {
        QImage backdrop = m_setBackdrops[set];
        QPixmap pixmap = QPixmap::fromImage(backdrop).scaled(
            QSize(200, 112) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
        ui->backdrop->setPixmap(pixmap);
        ui->backdropResolution->setText(QString("%1x%2").arg(backdrop.width()).arg(backdrop.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = backdrop;
    } else if (!Manager::instance()->mediaCenterInterface()->movieSetBackdrop(set).isNull()) {
        QImage backdrop = Manager::instance()->mediaCenterInterface()->movieSetBackdrop(set);
        QPixmap pixmap = QPixmap::fromImage(backdrop).scaled(
            QSize(200, 112) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
        ui->backdrop->setPixmap(pixmap);
        ui->backdropResolution->setText(QString("%1x%2").arg(backdrop.width()).arg(backdrop.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = backdrop;
    } else {
        QPixmap pixmap =
            QPixmap(":/img/placeholders/fanart.png")
                .scaled(QSize(96, 96) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        helper::setDevicePixelRatio(pixmap, helper::devicePixelRatio(this));
        ui->backdrop->setPixmap(pixmap);
        ui->buttonPreviewBackdrop->setEnabled(false);
    }
    ui->movies->blockSignals(false);
}

/**
 * \brief Called when an item in the movies table was changed
 *        Updates movies sorttitle and reorders the table
 * \param item changed item
 */
void SetsWidget::onSortTitleChanged(QTableWidgetItem* item)
{
    qDebug() << "Entered, item->row=" << item->row() << "rowCount=" << ui->movies->rowCount();
    if (item->row() < 0 || item->row() >= ui->movies->rowCount() || item->column() != 1) {
        qDebug() << "Invalid row";
        return;
    }
    auto* movie = ui->movies->item(item->row(), 0)->data(Qt::UserRole).value<Movie*>();
    movie->setSortTitle(item->text());
    ui->movies->sortByColumn(1, Qt::AscendingOrder);
    if (!m_moviesToSave[movie->set().name].contains(movie)) {
        m_moviesToSave[movie->set().name].append(movie);
    }
}

/**
 * \brief Execs the MovieListDialog and (if accepted) adds a movie to the movies table,
 *        sets the setname in the movie and adds the movie to m_sets
 */
void SetsWidget::onAddMovie()
{
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount()) {
        qDebug() << "[SetsWidget] Invalid current row";
        return;
    }

    auto* listDialog = new MovieListDialog(this);
    const int exitCode = listDialog->exec();
    QVector<Movie*> movies = listDialog->selectedMovies();
    listDialog->deleteLater();

    if (exitCode != QDialog::Accepted || movies.isEmpty()) {
        return;
    }

    const int row = ui->sets->currentRow();
    if (row < 0 || row >= ui->sets->rowCount()) {
        return;
    }

    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->text();
    for (Movie* movie : movies) {
        if (movie->set().name == setName) {
            continue;
        }
        MovieSet set = movie->set();
        set.name = setName;
        movie->setSet(set);
        m_sets[setName].append(movie);
        if (!m_moviesToSave[setName].contains(movie)) {
            m_moviesToSave[setName].append(movie);
        }
    }
    loadSet(setName);
}

/**
 * \brief Removes a movie from the movies table, sets and empty sorttitle and set for the movie
 *        and removes it from m_sets
 */
void SetsWidget::onRemoveMovie()
{
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount()) {
        qDebug() << "Invalid current row in sets";
        return;
    }
    if (ui->movies->currentRow() < 0 || ui->movies->currentRow() >= ui->movies->rowCount()) {
        qDebug() << "Invalid current row in movies";
        return;
    }
    auto* movie = ui->movies->item(ui->movies->currentRow(), 0)->data(Qt::UserRole).value<Movie*>();
    m_sets[movie->set().name].removeOne(movie);
    if (!m_moviesToSave[movie->set().name].contains(movie)) {
        m_moviesToSave[movie->set().name].append(movie);
    }
    movie->setSortTitle("");
    movie->setSet(MovieSet{});
    ui->movies->removeRow(ui->movies->currentRow());
}

/**
 * \brief Shows QFileDialog to choose an image, if successful sets the poster
 */
void SetsWidget::chooseSetPoster()
{
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount()) {
        qDebug() << "[SetsWidget] Invalid current row in sets";
        return;
    }

    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
    auto* movie = new Movie(QStringList());
    movie->setName(setName);

    auto* imageDialog = new ImageDialog(this);
    imageDialog->setImageType(ImageType::MovieSetPoster);
    imageDialog->setMovie(movie);
    imageDialog->execWithType(ImageType::MoviePoster);
    const int exitCode = imageDialog->result();
    const QUrl imageUrl = imageDialog->imageUrl();
    imageDialog->deleteLater();

    if (exitCode == QDialog::Accepted) {
        DownloadManagerElement d;
        d.movie = movie;
        d.imageType = ImageType::MovieSetPoster;
        d.url = imageUrl;
        m_downloadManager->addDownload(d);
        ui->poster->setPixmap(QPixmap());
        ui->poster->setMovie(m_loadingMovie);
        ui->buttonPreviewPoster->setEnabled(false);
    }
}

/**
 * \brief Shows QFileDialog to choose an image, if successful sets the backdrop
 */
void SetsWidget::chooseSetBackdrop()
{
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount()) {
        qDebug() << "Invalid current row in sets";
        return;
    }

    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
    auto* movie = new Movie(QStringList());
    movie->setName(setName);

    auto* imageDialog = new ImageDialog(this);
    imageDialog->setImageType(ImageType::MovieSetBackdrop);
    imageDialog->setMovie(movie);
    imageDialog->execWithType(ImageType::MovieBackdrop);
    const int exitCode = imageDialog->result();
    const QUrl imageUrl = imageDialog->imageUrl();
    imageDialog->deleteLater();

    if (exitCode == QDialog::Accepted) {
        DownloadManagerElement d;
        d.movie = movie;
        d.imageType = ImageType::MovieSetBackdrop;
        d.url = imageUrl;
        m_downloadManager->addDownload(d);
        ui->backdrop->setPixmap(QPixmap());
        ui->backdrop->setMovie(m_loadingMovie);
        ui->buttonPreviewBackdrop->setEnabled(false);
    }
}

/**
 * \brief Saves changed movies in this set
 */
void SetsWidget::saveSet()
{
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount()) {
        qDebug() << "Invalid current row in sets";
        return;
    }

    QStringList setNames;
    setNames << ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
    setNames << ui->sets->item(ui->sets->currentRow(), 0)->text();
    setNames.removeDuplicates();

    for (const QString& setName : setNames) {
        for (Movie* movie : m_moviesToSave[setName]) {
            movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
        }
        m_moviesToSave[setName].clear();

        if (!m_setPosters[setName].isNull()) {
            Manager::instance()->mediaCenterInterface()->saveMovieSetPoster(setName, m_setPosters[setName]);
            m_setPosters[setName] = QImage();
        }
        if (!m_setBackdrops[setName].isNull()) {
            Manager::instance()->mediaCenterInterface()->saveMovieSetBackdrop(setName, m_setBackdrops[setName]);
            m_setBackdrops[setName] = QImage();
        }
    }

    NotificationBox::instance()->showSuccess(
        tr("<b>\"%1\"</b> Saved").arg(ui->sets->item(ui->sets->currentRow(), 0)->text()));
}

/**
 * \brief Shows a full preview of the current backdrop
 */
void SetsWidget::onPreviewBackdrop()
{
    auto* dialog = new ImagePreviewDialog(this);
    dialog->setImage(QPixmap::fromImage(m_currentBackdrop));
    dialog->exec();
    dialog->deleteLater();
}

/**
 * \brief Shows a full preview of the current poster
 */
void SetsWidget::onPreviewPoster()
{
    auto* dialog = new ImagePreviewDialog(this);
    dialog->setImage(QPixmap::fromImage(m_currentPoster));
    dialog->exec();
    dialog->deleteLater();
}

void SetsWidget::onAddMovieSet()
{
    m_tableContextMenu->close();
    QString setName = tr("New Movie Set");
    int adder = -1;
    bool setExists = false;
    do {
        adder++;
        setExists = false;
        for (int i = 0, n = ui->sets->rowCount(); i < n; ++i) {
            if ((adder == 0 && ui->sets->item(i, 0)->text() == setName)
                || (adder > 0 && ui->sets->item(i, 0)->text() == QString("%1 %2").arg(setName).arg(adder))) {
                setExists = true;
                break;
            }
        }
    } while (setExists);

    if (adder > 0) {
        setName.append(QString(" %1").arg(adder));
    }

    m_addedSets << setName;

    QVector<Movie*> l;
    QVector<Movie*> el;
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
    m_tableContextMenu->close();
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount()) {
        qWarning() << "Invalid row" << ui->sets->currentRow();
        return;
    }

    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->text();
    QString origSetName = ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
    ui->sets->removeRow(ui->sets->currentRow());

    for (Movie* movie : m_sets[origSetName]) {
        movie->setSet(MovieSet{});
        movie->setSortTitle("");
    }
    m_sets.remove(setName);
    m_setPosters.remove(setName);
    m_setBackdrops.remove(setName);
    m_addedSets.removeOne(setName);
}

void SetsWidget::onSetNameChanged(QTableWidgetItem* item)
{
    QString newName = item->text();
    QString origSetName = item->data(Qt::UserRole).toString();
    if (newName == origSetName) {
        return;
    }

    for (int i = 0, n = ui->sets->rowCount(); i < n; ++i) {
        if (i != item->row() && ui->sets->item(i, 0)->text() == newName) {
            ui->sets->removeRow(i);
            break;
        }
    }

    if (!m_moviesToSave.contains(newName)) {
        m_moviesToSave.insert(newName, QVector<Movie*>());
    }

    for (Movie* movie : m_sets[origSetName]) {
        m_moviesToSave[newName].append(movie);
        MovieSet set;
        set.name = newName;
        movie->setSet(set);
    }

    m_moviesToSave[origSetName].clear();

    if (!m_sets.contains(newName)) {
        m_sets[newName].append(QVector<Movie*>());
    }

    m_sets[newName].append(m_sets[origSetName]);
    m_sets.remove(origSetName);

    if (!m_setPosters.contains(newName)) {
        m_setPosters.insert(newName, QImage());
    }
    if (!m_setBackdrops.contains(newName)) {
        m_setBackdrops.insert(newName, QImage());
    }

    if (m_addedSets.contains(newName)) {
        m_addedSets.removeOne(origSetName);
        if (!m_addedSets.contains(newName)) {
            m_addedSets.append(newName);
        }
    }

    loadSet(newName);
}

void SetsWidget::onDownloadFinished(DownloadManagerElement elem)
{
    QString setName = elem.movie->name();
    if (elem.imageType == ImageType::MovieSetPoster) {
        if (m_setPosters.contains(setName)) {
            m_setPosters[setName] = QImage::fromData(elem.data);
        }
        if (ui->sets->currentRow() >= 0 && ui->sets->currentRow() < ui->sets->rowCount()
            && ui->sets->item(ui->sets->currentRow(), 0)->text() == setName) {
            loadSet(setName);
        }
    } else if (elem.imageType == ImageType::MovieSetBackdrop) {
        if (m_setBackdrops.contains(setName)) {
            m_setBackdrops[setName] = QImage::fromData(elem.data);
        }
        if (ui->sets->currentRow() >= 0 && ui->sets->currentRow() < ui->sets->rowCount()
            && ui->sets->item(ui->sets->currentRow(), 0)->text() == setName) {
            loadSet(setName);
        }
    }
    delete elem.movie;
}

void SetsWidget::onJumpToMovie(QTableWidgetItem* item)
{
    if (item->column() != 0) {
        return;
    }

    auto* movie = item->data(Qt::UserRole).value<Movie*>();
    emit sigJumpToMovie(movie);
}
