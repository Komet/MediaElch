#include "SetsWidget.h"
#include "ui_SetsWidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include "data/Movie.h"
#include "ImagePreviewDialog.h"
#include "Manager.h"
#include "MessageBox.h"
#include "MovieListDialog.h"

SetsWidget::SetsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SetsWidget)
{
    ui->setupUi(this);

    ui->sets->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->movies->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->movies->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewPoster->setEnabled(false);

    QFont font = ui->setName->font();
    font.setPointSize(font.pointSize()+4);
    ui->setName->setFont(font);
#ifdef Q_WS_MAC
    QFont setsFont = ui->sets->font();
    setsFont.setPointSize(setsFont.pointSize()-2);
    ui->sets->setFont(setsFont);
#endif

    connect(ui->sets, SIGNAL(itemSelectionChanged()), this, SLOT(onSetSelected()));
    connect(ui->movies, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onSortTitleChanged(QTableWidgetItem*)));
    connect(ui->buttonAddMovie, SIGNAL(clicked()), this, SLOT(onAddMovie()));
    connect(ui->buttonRemoveMovie, SIGNAL(clicked()), this, SLOT(onRemoveMovie()));
    connect(ui->poster, SIGNAL(clicked()), this, SLOT(chooseSetPoster()));
    connect(ui->backdrop, SIGNAL(clicked()), this, SLOT(chooseSetBackdrop()));
    connect(ui->buttonPreviewPoster, SIGNAL(clicked()), this, SLOT(onPreviewPoster()));
    connect(ui->buttonPreviewBackdrop, SIGNAL(clicked()), this, SLOT(onPreviewBackdrop()));

    clear();
}

SetsWidget::~SetsWidget()
{
    delete ui;
}

QSplitter *SetsWidget::splitter()
{
    return ui->splitter;
}

/**
 * @brief Parses list of movie and constructs sets map
 */
void SetsWidget::loadSets()
{
    emit setActionSaveEnabled(false, WidgetMovieSets);
    clear();
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewPoster->setEnabled(false);
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
    QMapIterator<QString, QList<Movie*> > it(m_sets);
    while (it.hasNext()) {
        it.next();
        int row = ui->sets->rowCount();
        ui->sets->insertRow(row);
        ui->sets->setItem(row, 0, new QTableWidgetItem(it.key()));
        ui->sets->item(row, 0)->setData(Qt::UserRole, it.key());
    }
    emit setActionSaveEnabled(true, WidgetMovieSets);
}

/**
 * @brief Called when set table selection changes
 * @see SetsWidget::loadSets
 */
void SetsWidget::onSetSelected()
{
    int row = ui->sets->currentRow();
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
    ui->setName->clear();
    ui->movies->clearContents();
    ui->movies->setRowCount(0);
    ui->backdropResolution->clear();
    ui->posterResolution->clear();
    m_currentBackdrop = QImage();
    m_currentPoster = QImage();
}

/**
 * @brief Fills the widget with set data
 * @param set Name of the set
 */
void SetsWidget::loadSet(QString set)
{
    clear();
    ui->setName->setText(set);
    ui->buttonPreviewBackdrop->setEnabled(false);
    ui->buttonPreviewPoster->setEnabled(false);

    foreach (Movie *movie, m_sets[set]) {
        int row = ui->movies->rowCount();
        ui->movies->insertRow(row);
        ui->movies->setItem(row, 0, new QTableWidgetItem(movie->name()));
        ui->movies->item(row, 0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->movies->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(movie));
        ui->movies->setItem(row, 1, new QTableWidgetItem(movie->sortTitle()));
    }
    ui->movies->sortByColumn(1, Qt::AscendingOrder);

    if (m_setPosters[set].isNull() && Manager::instance()->mediaCenterInterface()->hasFeature(MediaCenterFeatures::HandleMovieSetImages) &&
        !Manager::instance()->mediaCenterInterface()->movieSetPoster(set).isNull()) {
        QImage poster = Manager::instance()->mediaCenterInterface()->movieSetPoster(set);
        ui->poster->setPixmap(QPixmap::fromImage(poster).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(poster.width()).arg(poster.height()));
        ui->buttonPreviewPoster->setEnabled(true);
        m_currentPoster = poster;
    } else {
        ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
        ui->buttonPreviewPoster->setEnabled(false);
    }

    if (m_setBackdrops[set].isNull() && Manager::instance()->mediaCenterInterface()->hasFeature(MediaCenterFeatures::HandleMovieSetImages) &&
        !Manager::instance()->mediaCenterInterface()->movieSetBackdrop(set).isNull()) {
        QImage backdrop = Manager::instance()->mediaCenterInterface()->movieSetBackdrop(set);
        ui->backdrop->setPixmap(QPixmap::fromImage(backdrop).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(backdrop.width()).arg(backdrop.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = backdrop;
    } else {
        ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->buttonPreviewBackdrop->setEnabled(false);
    }
}

/**
 * @brief Called when an item in the movies table was changed
 *        Updates movies sorttitle and reorders the table
 * @param item changed item
 */
void SetsWidget::onSortTitleChanged(QTableWidgetItem *item)
{
    if (item->row() < 0 || item->row() >= ui->movies->rowCount() || item->column() != 1)
        return;

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
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount())
        return;
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
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount())
        return;
    if (ui->movies->currentRow() < 0 || ui->movies->currentRow() >= ui->movies->rowCount())
        return;
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
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount())
        return;

    if (!Manager::instance()->mediaCenterInterface()->hasFeature(MediaCenterFeatures::HandleMovieSetImages)) {
        QMessageBox::information(this, tr("MediaElch"),
                                       tr("Setting Posters and Backdrops is only possible with XBMC MySQL and SQLite interfaces."),
                                 QMessageBox::Close, QMessageBox::Close);
        return;
    }

    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
    QString fileName = QFileDialog::getOpenFileName(parentWidget(), tr("Choose Image"), QDir::homePath(), tr("Images (*.jpg *.jpeg)"));
    if (!fileName.isNull()) {
        QImage img(fileName);
        if (!img.isNull()) {
            ui->poster->setPixmap(QPixmap::fromImage(img).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->posterResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
            ui->buttonPreviewPoster->setEnabled(true);
            m_setPosters[setName] = img;
            m_currentPoster = img;
        }
    }
}

/**
 * @brief Shows QFileDialog to choose an image, if successful sets the backdrop
 */
void SetsWidget::chooseSetBackdrop()
{
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount())
        return;

    if (!Manager::instance()->mediaCenterInterface()->hasFeature(MediaCenterFeatures::HandleMovieSetImages)) {
        QMessageBox::information(this, tr("MediaElch"),
                                       tr("Setting Posters and Backdrops is only possible with XBMC MySQL and SQLite interfaces."),
                                 QMessageBox::Close, QMessageBox::Close);
        return;
    }

    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
    QString fileName = QFileDialog::getOpenFileName(parentWidget(), tr("Choose Image"), QDir::homePath(), tr("Images (*.jpg *.jpeg)"));
    if (!fileName.isNull()) {
        QImage img(fileName);
        if (!img.isNull()) {
            ui->backdrop->setPixmap(QPixmap::fromImage(img).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->backdropResolution->setText(QString("%1x%2").arg(img.width()).arg(img.height()));
            ui->buttonPreviewBackdrop->setEnabled(true);
            m_setBackdrops[setName] = img;
            m_currentBackdrop = img;
        }
    }
}

/**
 * @brief Saves changed movies in this set
 */
void SetsWidget::saveSet()
{
    if (ui->sets->currentRow() < 0 || ui->sets->currentRow() >= ui->sets->rowCount())
        return;
    QString setName = ui->sets->item(ui->sets->currentRow(), 0)->data(Qt::UserRole).toString();
    foreach (Movie *movie, m_moviesToSave[setName])
        movie->saveData(Manager::instance()->mediaCenterInterface());
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
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentBackdrop));
    ImagePreviewDialog::instance()->exec();
}

/**
 * @brief Shows a full preview of the current poster
 */
void SetsWidget::onPreviewPoster()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentPoster));
    ImagePreviewDialog::instance()->exec();
}
