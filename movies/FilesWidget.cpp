#include "FilesWidget.h"
#include "ui_FilesWidget.h"

#include <QDesktopServices>
#include <QLocale>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QTableWidget>
#include <QTimer>
#include "globals/Globals.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "movies/MovieMultiScrapeDialog.h"
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
#ifdef Q_OS_MAC
    QFont font = ui->files->font();
    font.setPointSize(font.pointSize()-2);
    ui->files->setFont(font);
#endif

#ifdef Q_OS_WIN32
    ui->verticalLayout->setContentsMargins(0, 0, 0, 1);
#endif

    m_lastMovie = 0;
    m_mouseIsIn = false;
    m_movieProxyModel = new MovieProxyModel(this);
    m_movieProxyModel->setSourceModel(Manager::instance()->movieModel());
    m_movieProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_movieProxyModel->setDynamicSortFilter(true);
    ui->files->setModel(m_movieProxyModel);
    for (int i=1, n=ui->files->model()->columnCount() ; i<n ; ++i) {
        ui->files->setColumnWidth(i, 24);
        ui->files->setColumnHidden(i, true);
    }
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    ui->files->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
    ui->files->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
#endif

    foreach (const MediaStatusColumns &column, Settings::instance()->mediaStatusColumns())
        ui->files->setColumnHidden(MovieModel::mediaStatusToColumn(column), false);

    m_alphaList = new AlphabeticalList(this, ui->files);
    m_baseLabelCss = ui->sortByYear->styleSheet();
    m_activeLabelCss = ui->sortByNew->styleSheet();

    QMenu *mediaStatusColumnsMenu = new QMenu(tr("Media Status Columns"), ui->files);
    for (int i=MediaStatusFirst, n=MediaStatusLast ; i<=n ; ++i) {
        QAction *action = new QAction(MovieModel::mediaStatusToText(static_cast<MediaStatusColumns>(i)), this);
        action->setProperty("mediaStatusColumn", i);
        action->setCheckable(true);
        action->setChecked(Settings::instance()->mediaStatusColumns().contains(static_cast<MediaStatusColumns>(i)));
        connect(action, SIGNAL(triggered()), this, SLOT(onActionMediaStatusColumn()));
        mediaStatusColumnsMenu->addAction(action);
    }

    QAction *actionMultiScrape = new QAction(tr("Load Information"), this);
    QAction *actionMarkAsWatched = new QAction(tr("Mark as watched"), this);
    QAction *actionMarkAsUnwatched = new QAction(tr("Mark as unwatched"), this);
    QAction *actionLoadStreamDetails = new QAction(tr("Load Stream Details"), this);
    QAction *actionMarkForSync = new QAction(tr("Add to Synchronization Queue"), this);
    QAction *actionUnmarkForSync = new QAction(tr("Remove from Synchronization Queue"), this);
    QAction *actionOpenFolder = new QAction(tr("Open Movie Folder"), this);
    m_contextMenu = new QMenu(ui->files);
    m_contextMenu->addAction(actionMultiScrape);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(actionMarkAsWatched);
    m_contextMenu->addAction(actionMarkAsUnwatched);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(actionLoadStreamDetails);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(actionMarkForSync);
    m_contextMenu->addAction(actionUnmarkForSync);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(actionOpenFolder);
    m_contextMenu->addSeparator();
    m_contextMenu->addMenu(mediaStatusColumnsMenu);

    connect(actionMultiScrape, SIGNAL(triggered()), this, SLOT(multiScrape()));
    connect(actionMarkAsWatched, SIGNAL(triggered()), this, SLOT(markAsWatched()));
    connect(actionMarkAsUnwatched, SIGNAL(triggered()), this, SLOT(markAsUnwatched()));
    connect(actionLoadStreamDetails, SIGNAL(triggered()), this, SLOT(loadStreamDetails()));
    connect(actionMarkForSync, SIGNAL(triggered()), this, SLOT(markForSync()));
    connect(actionUnmarkForSync, SIGNAL(triggered()), this, SLOT(unmarkForSync()));
    connect(actionOpenFolder, SIGNAL(triggered()), this, SLOT(openFolder()));

    connect(ui->files, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(ui->files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(itemActivated(QModelIndex, QModelIndex)));
    connect(ui->files->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(setAlphaListData()));
    connect(ui->files, SIGNAL(sigLeftEdge(bool)), this, SLOT(onLeftEdge(bool)));

    connect(m_alphaList, SIGNAL(sigAlphaClicked(QString)), this, SLOT(scrollToAlpha(QString)));

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

void FilesWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    int scrollBarWidth = 0;
    if (ui->files->verticalScrollBar()->isVisible())
        scrollBarWidth = ui->files->verticalScrollBar()->width();
    m_alphaList->setRightSpace(scrollBarWidth+5);
    m_alphaList->setBottomSpace(ui->widget->height()+10);
    m_alphaList->adjustSize();
}

void FilesWidget::showContextMenu(QPoint point)
{
    m_contextMenu->exec(ui->files->mapToGlobal(point));
}

void FilesWidget::multiScrape()
{
    QList<Movie*> movies = selectedMovies();
    if (movies.isEmpty())
        return;

    MovieMultiScrapeDialog::instance()->setMovies(movies);
    int result = MovieMultiScrapeDialog::instance()->exec();
    if (result == QDialog::Accepted)
        movieSelectedEmitter();
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
        movies.at(0)->controller()->loadStreamDetailsFromFile();
        movies.at(0)->setChanged(true);
    } else {
        LoadingStreamDetails *loader = new LoadingStreamDetails(this);
        loader->loadMovies(movies);
        delete loader;
    }
    movieSelectedEmitter();
    m_movieProxyModel->setSourceModel(Manager::instance()->movieModel());
}

void FilesWidget::markForSync()
{
    foreach (const QModelIndex &index, ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Movie *movie = Manager::instance()->movieModel()->movie(row);
        movie->setSyncNeeded(true);
        ui->files->update(index);
    }
}

void FilesWidget::unmarkForSync()
{
    foreach (const QModelIndex &index, ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        Movie *movie = Manager::instance()->movieModel()->movie(row);
        movie->setSyncNeeded(false);
        ui->files->update(index);
    }
}

void FilesWidget::openFolder()
{
    int row = ui->files->currentIndex().data(Qt::UserRole).toInt();
    Movie *movie = Manager::instance()->movieModel()->movie(row);
    if (movie->files().isEmpty())
        return;
    QFileInfo fi(movie->files().at(0));
    QUrl url;
    if (fi.absolutePath().startsWith("\\\\") || fi.absolutePath().startsWith("//"))
        url.setUrl(QDir::toNativeSeparators(fi.absolutePath()));
    else
        url = QUrl::fromLocalFile(fi.absolutePath());
    QDesktopServices::openUrl(url);
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
        m_lastMovie = 0;
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
    if (m_lastMovie)
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
    setAlphaListData();
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

QList<Movie*> FilesWidget::selectedMovies()
{
    QList<Movie*> movies;
    foreach (const QModelIndex &index, ui->files->selectionModel()->selectedRows(0)) {
        int row = index.model()->data(index, Qt::UserRole).toInt();
        movies.append(Manager::instance()->movieModel()->movie(row));
    }
    if (movies.isEmpty())
        movies << m_lastMovie;
    return movies;
}

void FilesWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_mouseIsIn = true;
}

void FilesWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_mouseIsIn = false;
    m_alphaList->hide();
}

void FilesWidget::setAlphaListData()
{
    QStringList alphas;
    for (int i=0, n=ui->files->model()->rowCount() ; i<n ; ++i) {
        QString title = ui->files->model()->data(ui->files->model()->index(i, 0)).toString();
        QString first = title.left(1).toUpper();
        if (!alphas.contains(first))
            alphas.append(first);
    }
    qSort(alphas.begin(), alphas.end(), LocaleStringCompare());
    int scrollBarWidth = 0;
    if (ui->files->verticalScrollBar()->isVisible())
        scrollBarWidth = ui->files->verticalScrollBar()->width();
    m_alphaList->setRightSpace(scrollBarWidth+5);
    m_alphaList->setAlphas(alphas);
}

void FilesWidget::scrollToAlpha(QString alpha)
{
    for (int i=0, n=ui->files->model()->rowCount() ; i<n ; ++i) {
        QModelIndex index = ui->files->model()->index(i, 0);
        QString title = ui->files->model()->data(index).toString();
        QString first = title.left(1).toUpper();
        if (first == alpha) {
            ui->files->scrollTo(index, QAbstractItemView::PositionAtTop);
            return;
        }
    }
}

void FilesWidget::renewModel()
{
    m_movieProxyModel->setSourceModel(Manager::instance()->movieModel());
    for (int i=1, n=ui->files->model()->columnCount() ; i<n ; ++i)
        ui->files->setColumnHidden(i, true);
    foreach (const MediaStatusColumns &column, Settings::instance()->mediaStatusColumns())
        ui->files->setColumnHidden(MovieModel::mediaStatusToColumn(column), false);
}

void FilesWidget::onLeftEdge(bool isEdge)
{
    if (isEdge && m_mouseIsIn)
        m_alphaList->show();
    else
        m_alphaList->hide();
}

void FilesWidget::selectMovie(Movie *movie)
{
    int row = Manager::instance()->movieModel()->movies().indexOf(movie);
    QModelIndex index = Manager::instance()->movieModel()->index(row, 0, QModelIndex());
    ui->files->selectRow(m_movieProxyModel->mapFromSource(index).row());
}

void FilesWidget::onActionMediaStatusColumn()
{
    QAction *action = static_cast<QAction*>(QObject::sender());
    if (!action)
        return;
    action->setChecked(action->isChecked());

    MediaStatusColumns col = static_cast<MediaStatusColumns>(action->property("mediaStatusColumn").toInt());
    QList<MediaStatusColumns> columns = Settings::instance()->mediaStatusColumns();
    if (action->isChecked() && !columns.contains(col))
        columns.append(col);
    else
        columns.removeAll(col);
    Settings::instance()->setMediaStatusColumns(columns);
    Settings::instance()->saveSettings();
    renewModel();
}
