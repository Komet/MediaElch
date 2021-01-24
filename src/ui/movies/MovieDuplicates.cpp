#include "MovieDuplicates.h"
#include "ui_MovieDuplicates.h"

#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"
#include "movies/Movie.h"
#include "movies/MovieProxyModel.h"
#include "ui/movies/MovieDuplicateItem.h"
#include "ui/notifications/NotificationBox.h"

#include <QDebug>
#include <QDesktopServices>
#include <QMenu>

MovieDuplicates::MovieDuplicates(QWidget* parent) : QWidget(parent), ui(new Ui::MovieDuplicates)
{
    ui->setupUi(this);

    if (!Settings::instance()->movieDuplicatesSplitterState().isNull()) {
        ui->splitter->restoreState(Settings::instance()->movieDuplicatesSplitterState());
    } else {
        ui->splitter->setSizes(QList<int>{200, 600});
    }

    m_movieProxyModel = new MovieProxyModel(this);
    m_movieProxyModel->setSourceModel(Manager::instance()->movieModel());
    m_movieProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_movieProxyModel->setFilterDuplicates(true);

    // The movie model that is assigned to ui->movies contains all movies,
    // but only duplicates are shown.
    ui->movies->setModel(m_movieProxyModel);

    for (int i = 1, n = ui->movies->model()->columnCount(); i < n; ++i) {
        ui->movies->setColumnWidth(i, 24);
        ui->movies->setColumnHidden(i, true);
    }
    ui->movies->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
#ifdef Q_OS_WIN
    ui->movies->setIconSize(QSize(12, 12));
#else
    ui->movies->setIconSize(QSize(16, 16));
#endif

    ui->duplicates->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    helper::applyStyle(ui->movieDuplicatesWidget);

    createContextMenu();

    // clang-format off
    connect(ui->movies,                   &MyTableView::doubleClicked,          this, &MovieDuplicates::onJumpToMovie);
    connect(ui->btnDetect,                &QPushButton::clicked,                this, &MovieDuplicates::detectDuplicates);
    connect(ui->movies->selectionModel(), &QItemSelectionModel::currentChanged, this, &MovieDuplicates::onItemActivated);
    // clang-format on
}

MovieDuplicates::~MovieDuplicates()
{
    Settings::instance()->setMovieDuplicatesSplitterState(ui->splitter->saveState());
    delete ui;
}

void MovieDuplicates::detectDuplicates()
{
    qDebug() << "Detecting duplicates";

    ui->duplicates->clear();
    ui->duplicates->setRowCount(0);
    m_duplicateMovies.clear();

    int counter = 0;
    int movieCount = Manager::instance()->movieModel()->movies().count();
    NotificationBox::instance()->showProgressBar(
        tr("Detecting duplicate movies..."), Constants::MovieDuplicatesProgressMessageId);
    NotificationBox::instance()->progressBarProgress(0, movieCount, Constants::MovieDuplicatesProgressMessageId);

    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        ++counter;
        QApplication::processEvents();

        NotificationBox::instance()->progressBarProgress(
            counter, movieCount, Constants::MovieDuplicatesProgressMessageId);
        movie->setHasDuplicates(false);

        QVector<Movie*> dups{movie};
        for (Movie* subMovie : Manager::instance()->movieModel()->movies()) {
            if (movie != subMovie && subMovie->isDuplicate(movie)) {
                dups.append(subMovie);
            }
        }
        if (dups.count() > 1) {
            m_duplicateMovies.insert(movie, dups);
            movie->setHasDuplicates(true);
        }
    }

    NotificationBox::instance()->hideProgressBar(Constants::MovieDuplicatesProgressMessageId);
}

void MovieDuplicates::onItemActivated(QModelIndex /*index*/, QModelIndex /*previous*/)
{
    Movie* movie = activeMovie();
    if (movie == nullptr) {
        return;
    }

    if (!m_duplicateMovies.contains(movie)) {
        return;
    }

    ui->duplicates->clear();
    ui->duplicates->setRowCount(0);

    for (Movie* dup : asConst(m_duplicateMovies[movie])) {
        auto* item = new MovieDuplicateItem(ui->duplicates);
        item->setMovie(dup, dup == movie);
        item->setDuplicateProperties(movie->duplicateProperties(dup));

        const int row = ui->duplicates->rowCount();
        ui->duplicates->insertRow(row);
        ui->duplicates->setCellWidget(row, 0, item);
    }
}

void MovieDuplicates::createContextMenu()
{
    // clang-format off
    auto *actionOpenDetailPage = new QAction(tr("Open Detail Page"),  this);
    auto *actionOpenFolder     = new QAction(tr("Open Movie Folder"), this);
    auto *actionOpenNfo        = new QAction(tr("Open NFO File"),     this);
    // clang-format on

    m_contextMenu = new QMenu(ui->movies);
    m_contextMenu->addAction(actionOpenDetailPage);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(actionOpenFolder);
    m_contextMenu->addAction(actionOpenNfo);

    // clang-format off
    connect(actionOpenDetailPage, &QAction::triggered, this, &MovieDuplicates::onOpenDetailPage);
    connect(actionOpenFolder,     &QAction::triggered, this, &MovieDuplicates::onOpenFolder);
    connect(actionOpenNfo,        &QAction::triggered, this, &MovieDuplicates::onOpenNfo);
    // clang-format on

    connect(ui->movies, &QWidget::customContextMenuRequested, this, &MovieDuplicates::showContextMenu);
}


void MovieDuplicates::showContextMenu(QPoint point)
{
    // Only show context menu if an item is selected.
    if (ui->movies->currentIndex().isValid()) {
        m_contextMenu->exec(ui->movies->mapToGlobal(point));
    }
}


void MovieDuplicates::onOpenDetailPage()
{
    Movie* movie = activeMovie();
    if (movie != nullptr) {
        emit sigJumpToMovie(movie);
    }
}

void MovieDuplicates::onOpenFolder()
{
    Movie* movie = activeMovie();
    if (movie != nullptr) {
        QFileInfo fi(movie->files().first().toString());
        QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
    }
}

void MovieDuplicates::onOpenNfo()
{
    Movie* movie = activeMovie();
    if (movie != nullptr) {
        QFileInfo fi(Manager::instance()->mediaCenterInterface()->nfoFilePath(movie));
        QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
    }
}

void MovieDuplicates::onJumpToMovie(const QModelIndex& /*index*/)
{
    Movie* movie = activeMovie();
    if (movie != nullptr) {
        emit sigJumpToMovie(movie);
    }
}

Movie* MovieDuplicates::activeMovie()
{
    const QModelIndex currentIndex = ui->movies->currentIndex();
    if (!currentIndex.isValid()) {
        return nullptr;
    }

    const int row = currentIndex.data(Qt::UserRole).toInt();
    Movie* movie = Manager::instance()->movieModel()->movie(row);
    if (movie != nullptr && !movie->files().isEmpty()) {
        return movie;
    }

    return nullptr;
}
