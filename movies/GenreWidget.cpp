#include "GenreWidget.h"
#include "ui_GenreWidget.h"

#include "globals/Manager.h"
#include "main/MessageBox.h"
#include "sets/MovieListDialog.h"

/**
 * @brief GenreWidget::GenreWidget
 * @param parent
 */
GenreWidget::GenreWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GenreWidget)
{
    ui->setupUi(this);
    ui->genres->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->movies->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    QFont font = ui->genreName->font();
    font.setPointSize(font.pointSize()+4);
    ui->genreName->setFont(font);

#ifdef Q_WS_MAC
    QFont genresFont = ui->genres->font();
    genresFont.setPointSize(genresFont.pointSize()-2);
    ui->genres->setFont(genresFont);
#endif

    m_tableContextMenu = new QMenu(this);
    QAction *actionDeleteGenre = new QAction(tr("Delete Genre"), this);
    m_tableContextMenu->addAction(actionDeleteGenre);
    connect(actionDeleteGenre, SIGNAL(triggered()), this, SLOT(deleteGenre()));

    connect(ui->genres, SIGNAL(itemSelectionChanged()), this, SLOT(onGenreSelected()));
    connect(ui->genres, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onGenreNameChanged(QTableWidgetItem*)));
}

/**
 * @brief GenreWidget::~GenreWidget
 */
GenreWidget::~GenreWidget()
{
    delete ui;
}

/**
 * @brief GenreWidget::contextMenuEvent
 * @param event
 */
void GenreWidget::contextMenuEvent(QContextMenuEvent *event)
{
    m_tableContextMenu->exec(event->globalPos());
}

/**
 * @brief Returns the splitter
 * @return The splitter
 */
QSplitter *GenreWidget::splitter()
{
    return ui->splitter;
}

/**
 * @brief Clears the genres table
 */
void GenreWidget::clear()
{
    ui->genres->clearContents();
    ui->genres->setRowCount(0);
    ui->movies->clearContents();
    ui->movies->setRowCount(0);
    ui->genreName->clear();
}

/**
 * @brief Loads all genres from movies and fills the genres table
 */
void GenreWidget::loadGenres()
{
    emit setActionSaveEnabled(false, WidgetGenres);
    ui->genres->blockSignals(true);
    clear();
    QStringList genres;
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        foreach (const QString &genre, movie->genres()) {
            if (!genre.isEmpty() && !genres.contains(genre))
                genres.append(genre);;
        }
    }
    genres.sort();

    foreach (const QString &genre, genres) {
        QTableWidgetItem *item = new QTableWidgetItem(genre);
        item->setData(Qt::UserRole, genre);
        int row = ui->genres->rowCount();
        ui->genres->insertRow(row);
        ui->genres->setItem(row, 0, item);
    }

    ui->genres->blockSignals(false);
    emit setActionSaveEnabled(true, WidgetGenres);
}

/**
 * @brief Fills the movies table with movies
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

    QString genreName = ui->genres->item(ui->genres->currentRow(), 0)->data(Qt::UserRole).toString();
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->genres().contains(genreName)) {
            int row = ui->movies->rowCount();
            QTableWidgetItem *item = new QTableWidgetItem(movie->name());
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
 * @brief Renames a genre
 * @param item Changed item in genres table
 */
void GenreWidget::onGenreNameChanged(QTableWidgetItem *item)
{
    QString newName = item->text();
    QString origName = item->data(Qt::UserRole).toString();
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->genres().contains(origName)) {
            movie->removeGenre(origName);
            if (!movie->genres().contains(newName))
                movie->addGenre(newName);
        }
    }
    ui->genreName->setText(newName);
}

/**
 * @brief Removes the current genre from all corresponding movies
 */
void GenreWidget::deleteGenre()
{
    if (ui->genres->currentRow() < 0 || ui->genres->currentRow() >= ui->genres->rowCount()) {
        qWarning() << "Invalid row" << ui->genres->currentRow();
        return;
    }

    QString genreName = ui->genres->item(ui->genres->currentRow(), 0)->data(Qt::UserRole).toString();
    ui->genres->removeRow(ui->genres->currentRow());

    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->genres().contains(genreName))
            movie->removeGenre(genreName);
    }
}

/**
 * @brief Remove a movie from the current genre
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
    Movie *movie = ui->movies->item(ui->movies->currentRow(), 0)->data(Qt::UserRole).value<Movie*>();
    movie->removeGenre(genreName);
    ui->movies->removeRow(ui->movies->currentRow());
}

/**
 * @brief Add a movie to the current genre
 */
void GenreWidget::addMovie()
{
    if (ui->genres->currentRow() < 0 || ui->genres->currentRow() >= ui->genres->rowCount()) {
        qWarning() << "Invalid genre row" << ui->genres->currentRow();
        return;
    }


    if (MovieListDialog::instance()->execWithoutGenre(ui->genres->item(ui->genres->currentRow(), 0)->text()) == QDialog::Accepted) {
        Movie *movie = MovieListDialog::instance()->selectedMovie();
        QString genreName = ui->genres->item(ui->genres->currentRow(), 0)->data(Qt::UserRole).toString();
        if (!movie->genres().contains(genreName)) {
            movie->addGenre(genreName);
            onGenreSelected();
        }
    }
}

/**
 * @brief Saves all changed movies with the current genre
 */
void GenreWidget::onSaveInformation()
{
    if (ui->genres->currentRow() < 0 || ui->genres->currentRow() >= ui->genres->rowCount()) {
        qWarning() << "Invalid genre row" << ui->genres->currentRow();
        return;
    }

    QString genreName = ui->genres->item(ui->genres->currentRow(), 0)->data(Qt::UserRole).toString();
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->hasChanged() && movie->genres().contains(genreName))
            movie->saveData(Manager::instance()->mediaCenterInterface());
    }

    MessageBox::instance()->showMessage(tr("Genre <b>\"%1\"</b> Saved").arg(genreName));
}

/**
 * @brief Saves all changed movies
 */
void GenreWidget::onSaveAll()
{
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->hasChanged())
            movie->saveData(Manager::instance()->mediaCenterInterface());
    }

    MessageBox::instance()->showMessage(tr("All Movies Saved"));
}
