#include "GenreWidget.h"
#include "ui_GenreWidget.h"

#include "globals/Helper.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "movies/Movie.h"
#include "ui/movie_sets/MovieListDialog.h"
#include "ui/notifications/NotificationBox.h"

GenreWidget::GenreWidget(QWidget* parent) : QWidget(parent), ui(new Ui::GenreWidget)
{
    ui->setupUi(this);
    ui->genres->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->movies->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

#ifndef Q_OS_MAC
    QFont nameFont = ui->genreName->font();
    nameFont.setPointSize(nameFont.pointSize() - 4);
    ui->genreName->setFont(nameFont);
#endif

    ui->genres->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableContextMenu = new QMenu(ui->genres);
    auto* actionAddGenre = new QAction(tr("Add Genre"), this);
    auto* actionDeleteGenre = new QAction(tr("Delete Genre"), this);
    m_tableContextMenu->addAction(actionAddGenre);
    m_tableContextMenu->addAction(actionDeleteGenre);
    connect(actionAddGenre, &QAction::triggered, this, &GenreWidget::addGenre);
    connect(actionDeleteGenre, &QAction::triggered, this, &GenreWidget::deleteGenre);
    connect(ui->genres, &QWidget::customContextMenuRequested, this, &GenreWidget::showGenresContextMenu);
    connect(ui->movies, &QTableWidget::itemDoubleClicked, this, &GenreWidget::onJumpToMovie);

    connect(ui->genres, &QTableWidget::itemSelectionChanged, this, &GenreWidget::onGenreSelected);
    connect(ui->genres, &QTableWidget::itemChanged, this, &GenreWidget::onGenreNameChanged);

    helper::applyStyle(ui->genres);
    helper::applyStyle(ui->infoGroupBox);
    helper::applyStyle(ui->label_2);
    helper::applyStyle(ui->label_3);
}

/**
 * \brief GenreWidget::~GenreWidget
 */
GenreWidget::~GenreWidget()
{
    delete ui;
}

/**
 * \brief Executes the genres table context menu
 * \param point Point where the menu will be shown
 */
void GenreWidget::showGenresContextMenu(QPoint point)
{
    m_tableContextMenu->exec(ui->genres->mapToGlobal(point));
}

/**
 * \brief Returns the splitter
 * \return The splitter
 */
QSplitter* GenreWidget::splitter()
{
    return ui->splitter;
}

/**
 * \brief Clears the genres table
 */
void GenreWidget::clear()
{
    ui->genres->clearContents();
    ui->genres->setRowCount(0);
    ui->movies->clearContents();
    ui->movies->setRowCount(0);
    ui->genreName->clear();
    m_addedGenres.clear();
}

/**
 * \brief Loads all genres from movies and fills the genres table
 */
void GenreWidget::loadGenres()
{
    emit setActionSaveEnabled(false, MainWidgets::Genres);
    ui->genres->blockSignals(true);
    clear();
    QStringList genres;
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        for (const QString& genre : movie->genres()) {
            if (!genre.isEmpty() && !genres.contains(genre)) {
                genres.append(genre);
            };
        }
    }
    for (const QString& genre : m_addedGenres) {
        if (!genre.isEmpty() && !genres.contains(genre)) {
            genres.append(genre);
        }
    }

    std::sort(genres.begin(), genres.end(), LocaleStringCompare());

    for (const QString& genre : genres) {
        auto* item = new QTableWidgetItem(genre);
        item->setData(Qt::UserRole, genre);
        int row = ui->genres->rowCount();
        ui->genres->insertRow(row);
        ui->genres->setItem(row, 0, item);
    }

    ui->genres->blockSignals(false);
    emit setActionSaveEnabled(true, MainWidgets::Genres);
}

/**
 * \brief Fills the movies table with movies
 */
void GenreWidget::onGenreSelected()
{
    if (ui->genres->currentRow() < 0 || ui->genres->currentRow() >= ui->genres->rowCount()) {
        qWarning() << "Invalid row" << ui->genres->currentRow();
        return;
    }

    ui->movies->clearContents();
    ui->movies->setRowCount(0);
    ui->movies->setSortingEnabled(false);

    QString genreName = ui->genres->item(ui->genres->currentRow(), 0)->text();
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (movie->genres().contains(genreName)) {
            int row = ui->movies->rowCount();
            auto* item = new QTableWidgetItem(movie->name());
            item->setData(Qt::UserRole, QVariant::fromValue(movie));
            ui->movies->insertRow(row);
            ui->movies->setItem(row, 0, item);
        }
    }
    ui->genreName->setText(genreName);
    ui->movies->setSortingEnabled(true);
    ui->movies->sortByColumn(0, Qt::AscendingOrder);
}

/**
 * \brief Renames a genre
 * \param item Changed item in genres table
 */
void GenreWidget::onGenreNameChanged(QTableWidgetItem* item)
{
    QString newName = item->text();
    QString origName = item->data(Qt::UserRole).toString();
    if (newName == origName) {
        return;
    }

    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (movie->genres().contains(origName)) {
            movie->removeGenre(origName);
            if (!movie->genres().contains(newName)) {
                movie->addGenre(newName);
            }
        }
    }
    ui->genreName->setText(newName);
    item->setData(Qt::UserRole, newName);
    if (m_addedGenres.contains(origName)) {
        m_addedGenres.removeOne(origName);
        if (!m_addedGenres.contains(newName)) {
            m_addedGenres.append(newName);
        }
    }
    loadGenres();
}

void GenreWidget::addGenre()
{
    m_tableContextMenu->close();
    QString genreName = tr("New Genre");
    int adder = -1;
    bool genreExists = false;
    do {
        adder++;
        genreExists = false;
        for (int i = 0, n = ui->genres->rowCount(); i < n; ++i) {
            if ((adder == 0 && ui->genres->item(i, 0)->text() == genreName)
                || (adder > 0 && ui->genres->item(i, 0)->text() == QString("%1 %2").arg(genreName).arg(adder))) {
                genreExists = true;
                break;
            }
        }
    } while (genreExists);

    if (adder > 0) {
        genreName.append(QString(" %1").arg(adder));
    }

    m_addedGenres << genreName;

    ui->genres->blockSignals(true);
    auto* item = new QTableWidgetItem(genreName);
    item->setData(Qt::UserRole, genreName);
    int row = ui->genres->rowCount();
    ui->genres->insertRow(row);
    ui->genres->setItem(row, 0, item);
    ui->genres->blockSignals(false);
}

/**
 * \brief Removes the current genre from all corresponding movies
 */
void GenreWidget::deleteGenre()
{
    m_tableContextMenu->close();
    if (ui->genres->currentRow() < 0 || ui->genres->currentRow() >= ui->genres->rowCount()) {
        qWarning() << "Invalid row" << ui->genres->currentRow();
        return;
    }

    QString genreName = ui->genres->item(ui->genres->currentRow(), 0)->text();
    QString origGenreName = ui->genres->item(ui->genres->currentRow(), 0)->data(Qt::UserRole).toString();
    ui->genres->removeRow(ui->genres->currentRow());

    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (movie->genres().contains(genreName)) {
            movie->removeGenre(genreName);
        }
    }

    m_addedGenres.removeOne(origGenreName);
}

/**
 * \brief Remove a movie from the current genre
 */
void GenreWidget::removeMovie()
{
    if (ui->movies->currentRow() < 0 || ui->movies->currentRow() >= ui->movies->rowCount()) {
        qWarning() << "Invalid row" << ui->movies->currentRow();
        return;
    }
    if (ui->genres->currentRow() < 0 || ui->genres->currentRow() >= ui->genres->rowCount()) {
        qWarning() << "Invalid genre row" << ui->genres->currentRow();
        return;
    }

    QString genreName = ui->genres->item(ui->genres->currentRow(), 0)->data(Qt::UserRole).toString();
    auto* movie = ui->movies->item(ui->movies->currentRow(), 0)->data(Qt::UserRole).value<Movie*>();
    movie->removeGenre(genreName);
    ui->movies->removeRow(ui->movies->currentRow());
}

void GenreWidget::addMovie()
{
    if (ui->genres->currentRow() < 0 || ui->genres->currentRow() >= ui->genres->rowCount()) {
        qWarning() << "[GenreWidget] Invalid genre row" << ui->genres->currentRow();
        return;
    }

    auto* listDialog = new MovieListDialog(this);
    const int exitCode = listDialog->execWithoutGenre(ui->genres->item(ui->genres->currentRow(), 0)->text());
    QVector<Movie*> movies = listDialog->selectedMovies();
    listDialog->deleteLater();

    if (exitCode != QDialog::Accepted || movies.isEmpty()) {
        return;
    }

    QString genreName = ui->genres->item(ui->genres->currentRow(), 0)->text();
    for (Movie* movie : movies) {
        if (movie->genres().contains(genreName)) {
            continue;
        }
        movie->addGenre(genreName);
    }
    onGenreSelected();
}

/**
 * \brief Saves all changed movies
 */
void GenreWidget::onSaveInformation()
{
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (movie->hasChanged()) {
            movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
        }
    }

    m_addedGenres.clear();
    loadGenres();
    NotificationBox::instance()->showSuccess(tr("All Movies Saved"));
}

void GenreWidget::onJumpToMovie(QTableWidgetItem* item)
{
    auto* movie = item->data(Qt::UserRole).value<Movie*>();
    emit sigJumpToMovie(movie);
}
