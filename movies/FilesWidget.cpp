#include "FilesWidget.h"
#include "ui_FilesWidget.h"

#include <QLocale>
#include <QTableWidget>
#include <QTimer>
#include "globals/Globals.h"
#include "globals/Manager.h"
#include "smallWidgets/LoadingStreamDetails.h"

FilesWidget *FilesWidget::m_instance;

/**
 * @brief FilesWidget::FilesWidget
 * @param parent
 */
FilesWidget::FilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilesWidget)
{
    m_instance = this;
    ui->setupUi(this);
    ui->files->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#ifdef Q_WS_MAC
    QFont font = ui->files->font();
    font.setPointSize(font.pointSize()-2);
    ui->files->setFont(font);
#endif
#ifdef Q_WS_WIN
    ui->verticalLayout->setContentsMargins(0, 0, 0, 1);
#endif
    m_movieDelegate = new MovieDelegate(this);
    m_movieProxyModel = new MovieProxyModel(this);
    m_movieProxyModel->setSourceModel(Manager::instance()->movieModel());
    m_movieProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_movieProxyModel->setDynamicSortFilter(true);
    ui->files->setModel(m_movieProxyModel);
    ui->files->setItemDelegate(m_movieDelegate);

    m_baseLabelCss = ui->sortByYear->styleSheet();
    m_activeLabelCss = ui->sortByNew->styleSheet();

    QAction *actionMarkAsWatched = new QAction(tr("Mark as watched"), this);
    QAction *actionMarkAsUnwatched = new QAction(tr("Mark as unwatched"), this);
    QAction *actionLoadStreamDetails = new QAction(tr("Load Stream Details"), this);
    m_contextMenu = new QMenu(ui->files);
    m_contextMenu->addAction(actionMarkAsWatched);
    m_contextMenu->addAction(actionMarkAsUnwatched);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(actionLoadStreamDetails);
    connect(actionMarkAsWatched, SIGNAL(triggered()), this, SLOT(markAsWatched()));
    connect(actionMarkAsUnwatched, SIGNAL(triggered()), this, SLOT(markAsUnwatched()));
    connect(actionLoadStreamDetails, SIGNAL(triggered()), this, SLOT(loadStreamDetails()));

    connect(ui->files, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(ui->files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(itemActivated(QModelIndex, QModelIndex)));

    connect(ui->sortByNew, SIGNAL(clicked()), this, SLOT(onSortByNew()));
    connect(ui->sortByName, SIGNAL(clicked()), this, SLOT(onSortByName()));
    connect(ui->sortByLastAdded, SIGNAL(clicked()), this, SLOT(onSortByAdded()));
    connect(ui->sortBySeen, SIGNAL(clicked()), this, SLOT(onSortBySeen()));
    connect(ui->sortByYear, SIGNAL(clicked()), this, SLOT(onSortByYear()));
}

/**
 * @brief FilesWidget::~FilesWidget
 */
FilesWidget::~FilesWidget()
{
    delete ui;
}

/**
 * @brief Returns the current instance
 * @return Instance of FilesWidget
 */
FilesWidget *FilesWidget::instance()
{
    return m_instance;
}

void FilesWidget::showContextMenu(QPoint point)
{
    m_contextMenu->exec(ui->files->mapToGlobal(point));
}

void FilesWidget::markAsWatched()
{
    foreach (const QModelIndex &index, ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Movie *movie = Manager::instance()->movieModel()->movie(row);
        movie->setWatched(true);
        if (movie->playcount() < 1)
            movie->setPlayCount(1);
        if (!movie->lastPlayed().isValid())
            movie->setLastPlayed(QDateTime::currentDateTime());
    }
    if (ui->files->selectionModel()->selectedRows(0).count() > 0)
        movieSelectedEmitter();
}

void FilesWidget::markAsUnwatched()
{
    foreach (const QModelIndex &index, ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Movie *movie = Manager::instance()->movieModel()->movie(row);
        if (movie->watched())
            movie->setWatched(false);
        if (movie->playcount() != 0)
            movie->setPlayCount(0);
    }
    if (ui->files->selectionModel()->selectedRows(0).count() > 0)
        movieSelectedEmitter();
}

void FilesWidget::loadStreamDetails()
{
    QList<Movie*> movies;
    foreach (const QModelIndex &index, ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Movie *movie = Manager::instance()->movieModel()->movie(row);
        movies.append(movie);
    }
    if (movies.count() == 1) {
        movies.at(0)->loadStreamDetailsFromFile();
        movies.at(0)->setChanged(true);
    } else {
        LoadingStreamDetails *loader = new LoadingStreamDetails(this);
        loader->loadMovies(movies);
        delete loader;
    }
    movieSelectedEmitter();
    m_movieProxyModel->setSourceModel(0);
    m_movieProxyModel->setSourceModel(Manager::instance()->movieModel());

}

/**
 * @brief Called when an item has selected
 * @param index
 * @param previous
 */
void FilesWidget::itemActivated(QModelIndex index, QModelIndex previous)
{
    qDebug() << "Entered";
    if (!index.isValid()) {
        qDebug() << "Index is invalid";
        emit noMovieSelected();
        return;
    }
    m_lastModelIndex = previous;
    int row = index.model()->data(index, Qt::UserRole).toInt();
    m_lastMovie = Manager::instance()->movieModel()->movie(row);
    QTimer::singleShot(0, this, SLOT(movieSelectedEmitter()));
}

/**
 * @brief Just emits movieSelected
 */
void FilesWidget::movieSelectedEmitter()
{
    qDebug() << "Entered";
    emit movieSelected(m_lastMovie);
}

/**
 * @brief Sets the filters
 * @param filters List of filters
 * @param text Filter text
 */
void FilesWidget::setFilter(QList<Filter*> filters, QString text)
{
    m_movieProxyModel->setFilter(filters, text);
    m_movieProxyModel->setFilterWildcard("*" + text + "*");
}

/**
 * @brief Restores the last selected item
 */
void FilesWidget::restoreLastSelection()
{
    qDebug() << "Entered";
    ui->files->setCurrentIndex(m_lastModelIndex);
}

/**
 * @brief Adjusts labels and sets sort by to added
 */
void FilesWidget::onSortByAdded()
{
    ui->sortByNew->setStyleSheet(m_baseLabelCss);
    ui->sortByLastAdded->setStyleSheet(m_activeLabelCss);
    ui->sortByName->setStyleSheet(m_baseLabelCss);
    ui->sortByYear->setStyleSheet(m_baseLabelCss);
    ui->sortBySeen->setStyleSheet(m_baseLabelCss);
    m_movieProxyModel->setSortBy(SortByAdded);
}

/**
 * @brief Adjusts labels and sets sort by to name
 */
void FilesWidget::onSortByName()
{
    ui->sortByNew->setStyleSheet(m_baseLabelCss);
    ui->sortByLastAdded->setStyleSheet(m_baseLabelCss);
    ui->sortByName->setStyleSheet(m_activeLabelCss);
    ui->sortByYear->setStyleSheet(m_baseLabelCss);
    ui->sortBySeen->setStyleSheet(m_baseLabelCss);
    m_movieProxyModel->setSortBy(SortByName);
}

/**
 * @brief Adjusts labels and sets sort by to name
 */
void FilesWidget::onSortByNew()
{
    ui->sortByLastAdded->setStyleSheet(m_baseLabelCss);
    ui->sortByName->setStyleSheet(m_baseLabelCss);
    ui->sortByYear->setStyleSheet(m_baseLabelCss);
    ui->sortBySeen->setStyleSheet(m_baseLabelCss);
    ui->sortByNew->setStyleSheet(m_activeLabelCss);
    m_movieProxyModel->setSortBy(SortByNew);
}

/**
 * @brief Adjusts labels and sets sort by to seen
 */
void FilesWidget::onSortBySeen()
{
    ui->sortByNew->setStyleSheet(m_baseLabelCss);
    ui->sortByLastAdded->setStyleSheet(m_baseLabelCss);
    ui->sortByName->setStyleSheet(m_baseLabelCss);
    ui->sortByYear->setStyleSheet(m_baseLabelCss);
    ui->sortBySeen->setStyleSheet(m_activeLabelCss);
    m_movieProxyModel->setSortBy(SortBySeen);
}

/**
 * @brief Adjusts labels and sets sort by to year
 */
void FilesWidget::onSortByYear()
{
    ui->sortByNew->setStyleSheet(m_baseLabelCss);
    ui->sortByLastAdded->setStyleSheet(m_baseLabelCss);
    ui->sortByName->setStyleSheet(m_baseLabelCss);
    ui->sortByYear->setStyleSheet(m_activeLabelCss);
    ui->sortBySeen->setStyleSheet(m_baseLabelCss);
    m_movieProxyModel->setSortBy(SortByYear);
}
