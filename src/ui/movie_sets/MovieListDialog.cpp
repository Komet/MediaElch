#include "MovieListDialog.h"
#include "ui_MovieListDialog.h"

#include "globals/Globals.h"
#include "globals/Manager.h"
#include "ui/main/MainWindow.h"

MovieListDialog::MovieListDialog(QWidget* parent) : QDialog(parent), ui(new Ui::MovieListDialog)
{
    ui->setupUi(this);
    ui->movies->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #MovieListDialog { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif
    connect(ui->buttonClose, &QAbstractButton::clicked, this, &QDialog::reject);
    connect(ui->buttonAddMovies, &QAbstractButton::clicked, this, &MovieListDialog::onAddMovies);
    connect(ui->filter, &QLineEdit::textEdited, this, &MovieListDialog::onFilterEdited);
}

MovieListDialog::~MovieListDialog()
{
    delete ui;
}

/**
 * \brief Executes the dialog
 * \return Result of QDialog::exec
 */
int MovieListDialog::exec()
{
    reposition();
    ui->filter->clear();
    ui->movies->clearContents();
    ui->movies->setRowCount(0);
    ui->movies->setSortingEnabled(false);
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        int row = ui->movies->rowCount();
        ui->movies->insertRow(row);
        QString title = (movie->released().isValid())
                            ? QString("%1 (%2)").arg(movie->title()).arg(movie->released().toString("yyyy"))
                            : movie->title();
        ui->movies->setItem(row, 0, new QTableWidgetItem(title));
        ui->movies->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(movie));
    }
    ui->movies->setSortingEnabled(true);
    ui->movies->sortByColumn(0, Qt::AscendingOrder);
    return QDialog::exec();
}

/**
 * \brief Executes the dialog and displays all movies except the ones who have the genre
 * \param genre Genre to exclude
 * \return Result of QDialog::exec
 */
int MovieListDialog::execWithoutGenre(QString genre)
{
    reposition();
    ui->filter->clear();
    ui->movies->clearContents();
    ui->movies->setRowCount(0);
    ui->movies->setSortingEnabled(false);
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (movie->genres().contains(genre)) {
            continue;
        }
        const int row = ui->movies->rowCount();
        ui->movies->insertRow(row);
        const QString title = (movie->released().isValid())
                                  ? QString("%1 (%2)").arg(movie->title()).arg(movie->released().toString("yyyy"))
                                  : movie->title();
        ui->movies->setItem(row, 0, new QTableWidgetItem(title));
        ui->movies->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(movie));
    }
    ui->movies->setSortingEnabled(true);
    ui->movies->sortByColumn(0, Qt::AscendingOrder);
    return QDialog::exec();
}

/**
 * \brief Executes the dialog and displays all movies except the ones who have the certification
 * \param certification Certification to exclude
 * \return Result of QDialog::exec
 */
int MovieListDialog::execWithoutCertification(Certification certification)
{
    reposition();
    ui->filter->clear();
    ui->movies->clearContents();
    ui->movies->setRowCount(0);
    ui->movies->setSortingEnabled(false);
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (movie->certification() == certification) {
            continue;
        }
        const int row = ui->movies->rowCount();
        ui->movies->insertRow(row);
        QString title = (movie->released().isValid())
                            ? QString("%1 (%2)").arg(movie->title()).arg(movie->released().toString("yyyy"))
                            : movie->title();
        ui->movies->setItem(row, 0, new QTableWidgetItem(title));
        ui->movies->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(movie));
    }
    ui->movies->setSortingEnabled(true);
    ui->movies->sortByColumn(0, Qt::AscendingOrder);
    return QDialog::exec();
}

/**
 * \brief Resizes and repositions the dialog
 */
void MovieListDialog::reposition()
{
    QWidget* window = MainWindow::instance();

    QSize newSize;
    newSize.setHeight(window->size().height() - 200);
    newSize.setWidth(qMin(1000, window->size().width() - 400));
    resize(newSize);

    const int xMove = (window->size().width() - size().width()) / 2;
    move(window->x() + xMove, window->y());
}

void MovieListDialog::onAddMovies()
{
    m_selectedMovies.clear();
    for (QTableWidgetItem* item : ui->movies->selectedItems()) {
        m_selectedMovies << item->data(Qt::UserRole).value<Movie*>();
    }
    accept();
}

QVector<Movie*> MovieListDialog::selectedMovies()
{
    return m_selectedMovies;
}

/**
 * \brief Hides all rows which doesn't contain text
 * \param text Filter text
 */
void MovieListDialog::onFilterEdited(QString text)
{
    for (int row = 0, n = ui->movies->rowCount(); row < n; ++row) {
        bool contains = ui->movies->item(row, 0)->text().contains(text, Qt::CaseInsensitive);
        ui->movies->setRowHidden(row, !contains);
    }
}
