#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QCheckBox>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QPainter>
#include <QTimer>
#include <QToolBar>

#include "concerts/ConcertSearch.h"
#include "data/MediaCenterInterface.h"
#include "globals/NameFormatter.h"
#include "data/ScraperInterface.h"
#include "globals/Globals.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "globals/TrailerDialog.h"
#include "notifications/NotificationBox.h"
#include "main/Update.h"
#include "movies/MovieMultiScrapeDialog.h"
#include "notifications/Notificator.h"
#include "sets/MovieListDialog.h"
#include "settings/Settings.h"
#include "tvShows/TvShowSearch.h"
#include "tvShows/TvShowUpdater.h"
#include "tvShows/TvTunesDialog.h"
#include "xbmc/XbmcSync.h"

MainWindow *MainWindow::m_instance = 0;

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setMinimumHeight(500);

    MainWindow::m_instance = this;

    qDebug() << "MediaElch version" << QApplication::applicationVersion() << "starting up";

    for (int i=WidgetMovies ; i!=WidgetDownloads; i++) {
        QMap<MainActions, bool> actions;
        for (int n=ActionSearch ; n!=ActionExport ; n++) {
            actions.insert(static_cast<MainActions>(n), false);
        }
        if (static_cast<MainWidgets>(i) == WidgetMovies || static_cast<MainWidgets>(i) == WidgetTvShows || static_cast<MainWidgets>(i) == WidgetConcerts)
            actions[ActionFilterWidget] = true;
        m_actions.insert(static_cast<MainWidgets>(i), actions);
    }

    m_aboutDialog = new AboutDialog(this);
    m_supportDialog = new SupportDialog(this);
    m_settingsWindow = new SettingsWindow(this);
    m_fileScannerDialog = new FileScannerDialog(this);
    m_xbmcSync = new XbmcSync(this);
    m_renamer = new Renamer(this);
    m_settings = Settings::instance(this);
    m_exportDialog = new ExportDialog(this);
    setupToolbar();

    NotificationBox::instance(this)->reposition(this->size());
    Manager::instance();

    if (!m_settings->mainSplitterState().isNull()) {
        ui->movieSplitter->restoreState(m_settings->mainSplitterState());
        ui->tvShowSplitter->restoreState(m_settings->mainSplitterState());
        ui->setsWidget->splitter()->restoreState(m_settings->mainSplitterState());
        ui->concertSplitter->restoreState(m_settings->mainSplitterState());
        ui->genreWidget->splitter()->restoreState(m_settings->mainSplitterState());
        ui->certificationWidget->splitter()->restoreState(m_settings->mainSplitterState());
    } else {
        ui->movieSplitter->setSizes(QList<int>() << 200 << 600);
        ui->tvShowSplitter->setSizes(QList<int>() << 200 << 600);
        ui->setsWidget->splitter()->setSizes(QList<int>() << 200 << 600);
        ui->concertSplitter->setSizes(QList<int>() << 200 << 600);
        ui->genreWidget->splitter()->setSizes(QList<int>() << 200 << 600);
        ui->certificationWidget->splitter()->setSizes(QList<int>() << 200 << 600);
    }

    if (m_settings->mainWindowSize().isValid() && !m_settings->mainWindowPosition().isNull()) {
        resize(m_settings->mainWindowSize());
        move(m_settings->mainWindowPosition());
        #ifdef Q_OS_WIN32
        if (m_settings->mainWindowMaximized())
            showMaximized();
        #endif
    }
    // Size for Screenshots
    // resize(1200, 676);

    connect(ui->filesWidget, SIGNAL(movieSelected(Movie*)), ui->movieWidget, SLOT(setMovie(Movie*)));
    connect(ui->filesWidget, SIGNAL(movieSelected(Movie*)), ui->movieWidget, SLOT(setEnabledTrue(Movie*)));
    connect(ui->filesWidget, SIGNAL(noMovieSelected()), ui->movieWidget, SLOT(clear()));
    connect(ui->filesWidget, SIGNAL(noMovieSelected()), ui->movieWidget, SLOT(setDisabledTrue()));
    connect(ui->filesWidget, SIGNAL(sigStartSearch()), this, SLOT(onActionSearch()));

    connect(ui->concertFilesWidget, SIGNAL(concertSelected(Concert*)), ui->concertWidget, SLOT(setConcert(Concert*)));
    connect(ui->concertFilesWidget, SIGNAL(concertSelected(Concert*)), ui->concertWidget, SLOT(setEnabledTrue(Concert*)));
    connect(ui->concertFilesWidget, SIGNAL(noConcertSelected()), ui->concertWidget, SLOT(clear()));
    connect(ui->concertFilesWidget, SIGNAL(noConcertSelected()), ui->concertWidget, SLOT(setDisabledTrue()));

    connect(ui->tvShowFilesWidget, SIGNAL(sigTvShowSelected(TvShow*)), ui->tvShowWidget, SLOT(onTvShowSelected(TvShow*)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigSeasonSelected(TvShow*,int)), ui->tvShowWidget, SLOT(onSeasonSelected(TvShow*,int)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigEpisodeSelected(TvShowEpisode*)), ui->tvShowWidget, SLOT(onEpisodeSelected(TvShowEpisode*)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigTvShowSelected(TvShow*)), ui->tvShowWidget, SLOT(onSetEnabledTrue(TvShow*)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigSeasonSelected(TvShow*,int)), ui->tvShowWidget, SLOT(onSetEnabledTrue(TvShow*, int)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigEpisodeSelected(TvShowEpisode*)), ui->tvShowWidget, SLOT(onSetEnabledTrue(TvShowEpisode*)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigNothingSelected()), ui->tvShowWidget, SLOT(onClear()));
    connect(ui->tvShowFilesWidget, SIGNAL(sigNothingSelected()), ui->tvShowWidget, SLOT(onSetDisabledTrue()));

    connect(ui->movieWidget, SIGNAL(actorDownloadProgress(int,int,int)), this, SLOT(progressProgress(int,int,int)));
    connect(ui->movieWidget, SIGNAL(actorDownloadStarted(QString,int)), this, SLOT(progressStarted(QString,int)));
    connect(ui->movieWidget, SIGNAL(actorDownloadFinished(int)), this, SLOT(progressFinished(int)));

    connect(ui->tvShowWidget, SIGNAL(sigDownloadsStarted(QString,int)), this, SLOT(progressStarted(QString,int)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsProgress(int,int,int)), this, SLOT(progressProgress(int,int,int)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsFinished(int)), this, SLOT(progressFinished(int)));

    connect(ui->navbar, SIGNAL(sigFilterChanged(QList<Filter*>,QString)), this, SLOT(onFilterChanged(QList<Filter*>,QString)));

    connect(ui->movieSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(moveSplitter(int,int)));
    connect(ui->tvShowSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(moveSplitter(int,int)));
    connect(ui->setsWidget->splitter(), SIGNAL(splitterMoved(int,int)), this, SLOT(moveSplitter(int,int)));
    connect(ui->genreWidget->splitter(), SIGNAL(splitterMoved(int,int)), this, SLOT(moveSplitter(int,int)));
    connect(ui->certificationWidget->splitter(), SIGNAL(splitterMoved(int,int)), this, SLOT(moveSplitter(int,int)));
    connect(ui->concertSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(moveSplitter(int,int)));

    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(tvShowsLoaded(int)), ui->tvShowFilesWidget, SLOT(renewModel()));
    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(tvShowsLoaded(int)), this, SLOT(updateTvShows()));
    connect(m_fileScannerDialog, SIGNAL(accepted()), this, SLOT(setNewMarks()));
    connect(ui->downloadsWidget, SIGNAL(sigScanFinished(bool)), this, SLOT(setNewMarkForImports(bool)));

    connect(m_xbmcSync, SIGNAL(sigTriggerReload()), this, SLOT(onTriggerReloadAll()));
    connect(m_xbmcSync, SIGNAL(sigFinished()), this, SLOT(onXbmcSyncFinished()));

    connect(m_renamer, SIGNAL(sigFilesRenamed(Renamer::RenameType)), this, SLOT(onFilesRenamed(Renamer::RenameType)));

    connect(m_settingsWindow, SIGNAL(sigSaved()), this, SLOT(onRenewModels()), Qt::QueuedConnection);

    connect(ui->setsWidget, SIGNAL(sigJumpToMovie(Movie*)), this, SLOT(onJumpToMovie(Movie*)));
    connect(ui->certificationWidget, SIGNAL(sigJumpToMovie(Movie*)), this, SLOT(onJumpToMovie(Movie*)));
    connect(ui->genreWidget, SIGNAL(sigJumpToMovie(Movie*)), this, SLOT(onJumpToMovie(Movie*)));

    connect(Update::instance(this), SIGNAL(sigNewVersion(QString)), this, SLOT(onNewVersion(QString)));

    TvShowSearch::instance(this);
    ImageDialog::instance(this);
    MovieListDialog::instance(this);
    ImagePreviewDialog::instance(this);
    ConcertSearch::instance(this);
    TrailerDialog::instance(this);
    TvTunesDialog::instance(this);
    NameFormatter::instance(this);
    MovieMultiScrapeDialog::instance(this);
    Notificator::instance(0, ui->centralWidget);

#ifdef Q_OS_WIN32
    setStyleSheet(styleSheet() + " #centralWidget { border-bottom: 1px solid rgba(0, 0, 0, 100); } ");

    QFont font = ui->labelMovies->font();
    font.setPointSize(font.pointSize()-3);
    font.setBold(true);
    ui->labelMovies->setFont(font);
    ui->labelConcerts->setFont(font);
    ui->labelShows->setFont(font);
    ui->labelDownloads->setFont(font);
#endif

    if (Settings::instance()->startupSection() == "tvshows")
        onMenuTvShows();
    else if (Settings::instance()->startupSection() == "concerts")
        onMenuConcerts();
    else if (Settings::instance()->startupSection() == "import")
        onMenuDownloads();
    else
        onMenuMovies();

    // hack. without only the fileScannerDialog pops up and blocks until it has finished
    show();

    // Start scanning for files
    QTimer::singleShot(0, m_fileScannerDialog, SLOT(exec()));

    if (Settings::instance()->checkForUpdates())
        Update::instance()->checkForUpdate();
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
    m_settings->setMainWindowSize(size());
    m_settings->setMainWindowPosition(pos());
    m_settings->setMainSplitterState(ui->movieSplitter->saveState());
    m_settings->setMainWindowMaximized(isMaximized());
    delete ui;
}

MainWindow *MainWindow::instance()
{
    return MainWindow::m_instance;
}

/**
 * @brief Repositions the MessageBox
 * @param event
 */
void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (event->size().width() >= 1500) {
        ui->movieWidget->setBigWindow(true);
        ui->tvShowWidget->setBigWindow(true);
        ui->concertWidget->setBigWindow(true);
    } else if (event->size().width() < 1500) {
        ui->movieWidget->setBigWindow(false);
        ui->tvShowWidget->setBigWindow(false);
        ui->concertWidget->setBigWindow(false);
    }

    NotificationBox::instance()->reposition(event->size());
    QWidget::resizeEvent(event);
}

/**
 * @brief Sets up the toolbar
 */
void MainWindow::setupToolbar()
{
    qDebug() << "Entered";

    connect(ui->navbar, SIGNAL(sigSearch()), this, SLOT(onActionSearch()));
    connect(ui->navbar, SIGNAL(sigSave()), this, SLOT(onActionSave()));
    connect(ui->navbar, SIGNAL(sigSaveAll()), this, SLOT(onActionSaveAll()));
    connect(ui->navbar, SIGNAL(sigReload()), this, SLOT(onActionReload()));
    connect(ui->navbar, SIGNAL(sigAbout()), m_aboutDialog, SLOT(exec()));
    connect(ui->navbar, SIGNAL(sigSettings()), m_settingsWindow, SLOT(show()));
    connect(ui->navbar, SIGNAL(sigLike()), m_supportDialog, SLOT(exec()));
    connect(ui->navbar, SIGNAL(sigSync()), this, SLOT(onActionXbmc()));
    connect(ui->navbar, SIGNAL(sigRename()), this, SLOT(onActionRename()));
    connect(ui->navbar, SIGNAL(sigExport()), m_exportDialog, SLOT(exec()));

    ui->navbar->setActionSearchEnabled(false);
    ui->navbar->setActionSaveEnabled(false);
    ui->navbar->setActionSaveAllEnabled(false);
    ui->navbar->setActionRenameEnabled(false);
}

/**
 * @brief Called when a subwidget starts a progress, displays a progress MessageBox
 * @param msg Message
 * @param id (Unique) Id of the progress
 */
void MainWindow::progressStarted(QString msg, int id)
{
    qDebug() << "Entered, msg=" << msg << "id=" << id;
    NotificationBox::instance()->showProgressBar(msg, id);
}

/**
 * @brief Updates the progress MessageBox
 * @param current Current value
 * @param max Maximum value
 * @param id (Unique) Id of the progress
 */
void MainWindow::progressProgress(int current, int max, int id)
{
    NotificationBox::instance()->progressBarProgress(current, max, id);
}

/**
 * @brief Called when a progress has finished
 * @param id (Unique) Id of the progress
 */
void MainWindow::progressFinished(int id)
{
    qDebug() << "Entered, id=" << id;
    NotificationBox::instance()->hideProgressBar(id);
}

/**
 * @brief Restores all menu icons to defaults and enables actions
 * @param widget Current widget
 */
void MainWindow::onMenu(MainWidgets widget)
{
    m_icons.insert(WidgetMovies, QIcon(":/img/video_menu.png"));
    m_icons.insert(WidgetTvShows, QIcon(":/img/display_on_menu.png"));
    m_icons.insert(WidgetMovieSets, QIcon(":/img/movieSets_menu.png"));
    m_icons.insert(WidgetGenres, QIcon(":/img/genre_menu.png"));
    m_icons.insert(WidgetCertifications, QIcon(":/img/certification2_menu.png"));
    m_icons.insert(WidgetConcerts, QIcon(":/img/concerts_menu.png"));
    m_icons.insert(WidgetDownloads, QIcon(":/img/downloads_menu.png"));

    if (widget == WidgetMovies)
        m_icons.insert(widget, QIcon(":/img/video_menuActive.png"));
    else if (widget == WidgetTvShows)
        m_icons.insert(widget, QIcon(":/img/display_on_menuActive.png"));
    else if (widget == WidgetConcerts)
        m_icons.insert(widget, QIcon(":/img/concerts_menuActive.png"));
    else if (widget == WidgetGenres)
        m_icons.insert(widget, QIcon(":/img/genre_menuActive.png"));
    else if (widget == WidgetMovieSets)
        m_icons.insert(widget, QIcon(":/img/movieSets_menuActive.png"));
    else if (widget == WidgetCertifications)
        m_icons.insert(widget, QIcon(":/img/certification2_menuActive.png"));
    else if (widget == WidgetDownloads)
        m_icons.insert(widget, QIcon(":/img/downloads_menuActive.png"));

    ui->buttonMovies->setIcon(m_icons.value(WidgetMovies));
    ui->buttonMovieSets->setIcon(m_icons.value(WidgetMovieSets));
    ui->buttonGenres->setIcon(m_icons.value(WidgetGenres));
    ui->buttonCertifications->setIcon(m_icons.value(WidgetCertifications));
    ui->buttonTvshows->setIcon(m_icons.value(WidgetTvShows));
    ui->buttonConcerts->setIcon(m_icons.value(WidgetConcerts));
    ui->buttonDownloads->setIcon(m_icons.value(WidgetDownloads));

    setNewMarks();
    setNewMarkForImports(ui->downloadsWidget->hasNewItems());

    ui->navbar->setActionSearchEnabled(m_actions[widget][ActionSearch]);
    ui->navbar->setActionSaveEnabled(m_actions[widget][ActionSave]);
    ui->navbar->setActionSaveAllEnabled(m_actions[widget][ActionSaveAll]);
    ui->navbar->setActionRenameEnabled(m_actions[widget][ActionRename]);
    ui->navbar->setFilterWidgetEnabled(m_actions[widget][ActionFilterWidget]);
    ui->navbar->setActiveWidget(widget);

    ui->navbar->setActionReloadEnabled(widget == WidgetMovies || widget == WidgetTvShows || widget == WidgetConcerts || widget == WidgetDownloads);
    if (widget == WidgetMovies)
        ui->navbar->setReloadToolTip(tr("Reload all Movies (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
    else if (widget == WidgetTvShows)
        ui->navbar->setReloadToolTip(tr("Reload all TV Shows (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
    else if (widget == WidgetConcerts)
        ui->navbar->setReloadToolTip(tr("Reload all Concerts (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
    else if (widget == WidgetDownloads)
        ui->navbar->setReloadToolTip(tr("Reload all Downloads (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
}

/**
 * @brief Called when the menu item "Movies" was clicked
 * Updates menu icons and sets status of actions
 */
void MainWindow::onMenuMovies()
{
    qDebug() << "Entered";
    ui->stackedWidget->setCurrentIndex(0);
    onMenu(WidgetMovies);
}

/**
 * @brief Called when the menu item "Movie Sets" was clicked
 * Updates menu icons and sets status of actions
 */
void MainWindow::onMenuMovieSets()
{
    qDebug() << "Entered";
    ui->stackedWidget->setCurrentIndex(2);
    ui->setsWidget->loadSets();
    onMenu(WidgetMovieSets);
}

/**
 * @brief Called when the menu item "Movie Genres" was clicked
 * Updates menu icons and sets status of actions
 */
void MainWindow::onMenuGenres()
{
    qDebug() << "Entered";
    ui->stackedWidget->setCurrentIndex(4);
    ui->genreWidget->loadGenres();
    onMenu(WidgetGenres);
}

/**
 * @brief Called when the menu item "Movie Certifications" was clicked
 * Updates menu icons and sets status of actions
 */
void MainWindow::onMenuCertifications()
{
    qDebug() << "Entered";
    ui->stackedWidget->setCurrentIndex(5);
    ui->certificationWidget->loadCertifications();
    onMenu(WidgetCertifications);
}

/**
 * @brief Called when the menu item "Tv Shows" was clicked
 * Updates menu icons and sets status of actions
 */
void MainWindow::onMenuTvShows()
{
    qDebug() << "Entered";
    ui->stackedWidget->setCurrentIndex(1);
    onMenu(WidgetTvShows);
}

/**
 * @brief Called when the menu item "Concerts" was clicked
 * Updates menu icons and sets status of actions
 */
void MainWindow::onMenuConcerts()
{
    qDebug() << "Entered";
    ui->stackedWidget->setCurrentIndex(3);
    onMenu(WidgetConcerts);
}

void MainWindow::onMenuDownloads()
{
    ui->stackedWidget->setCurrentIndex(6);
    onMenu(WidgetDownloads);
}

/**
 * @brief Called when the action "Search" was clicked
 * Delegates the event down to the current subwidget
 */
void MainWindow::onActionSearch()
{
    qDebug() << "Entered, currentIndex=" << ui->stackedWidget->currentIndex();
    if (ui->stackedWidget->currentIndex() == 0) {
        if (ui->filesWidget->selectedMovies().count() > 1)
            ui->filesWidget->multiScrape();
        else
            QTimer::singleShot(0, ui->movieWidget, SLOT(startScraperSearch()));
    } else if (ui->stackedWidget->currentIndex() == 1) {
        QTimer::singleShot(0, ui->tvShowWidget, SLOT(onStartScraperSearch()));
    } else if (ui->stackedWidget->currentIndex() == 3) {
        QTimer::singleShot(0, ui->concertWidget, SLOT(onStartScraperSearch()));
    }
}

/**
 * @brief Called when the action "Save" was clicked
 * Delegates the event down to the current subwidget
 */
void MainWindow::onActionSave()
{
    qDebug() << "Entered, currentIndex=" << ui->stackedWidget->currentIndex();
    if (ui->stackedWidget->currentIndex() == 0)
        ui->movieWidget->saveInformation();
    else if (ui->stackedWidget->currentIndex() == 1)
        ui->tvShowWidget->onSaveInformation();
    else if (ui->stackedWidget->currentIndex() == 2)
        ui->setsWidget->saveSet();
    else if (ui->stackedWidget->currentIndex() == 3)
        ui->concertWidget->onSaveInformation();
    else if (ui->stackedWidget->currentIndex() == 4)
        ui->genreWidget->onSaveInformation();
    else if (ui->stackedWidget->currentIndex() == 5)
        ui->certificationWidget->onSaveInformation();
    setNewMarks();
}

/**
 * @brief Called when the action "Save all" was clicked
 * Delegates the event down to the current subwidget
 */
void MainWindow::onActionSaveAll()
{
    qDebug() << "Entered, currentIndex=" << ui->stackedWidget->currentIndex();
    if (ui->stackedWidget->currentIndex() == 0)
        ui->movieWidget->saveAll();
    else if (ui->stackedWidget->currentIndex() == 1)
        ui->tvShowWidget->onSaveAll();
    else if (ui->stackedWidget->currentIndex() == 3)
        ui->concertWidget->onSaveAll();
    setNewMarks();
}

/**
 * @brief Executes the file scanner dialog
 */
void MainWindow::onActionReload()
{
    if (ui->stackedWidget->currentIndex() == 6) {
        ui->downloadsWidget->scanDownloadFolders();
        return;
    }

    m_fileScannerDialog->setForceReload(true);

    if (ui->stackedWidget->currentIndex() == 0)
        m_fileScannerDialog->setReloadType(FileScannerDialog::TypeMovies);
    else if (ui->stackedWidget->currentIndex() == 1)
        m_fileScannerDialog->setReloadType(FileScannerDialog::TypeTvShows);
    else if (ui->stackedWidget->currentIndex() == 3)
        m_fileScannerDialog->setReloadType(FileScannerDialog::TypeConcerts);

    m_fileScannerDialog->exec();
}

void MainWindow::onActionRename()
{
    if (ui->stackedWidget->currentIndex() == 0) {
        m_renamer->setRenameType(Renamer::TypeMovies);
        m_renamer->setMovies(ui->filesWidget->selectedMovies());
    } else if (ui->stackedWidget->currentIndex() == 1) {
        m_renamer->setRenameType(Renamer::TypeTvShows);
        m_renamer->setShows(ui->tvShowFilesWidget->selectedShows());
        m_renamer->setEpisodes(ui->tvShowFilesWidget->selectedEpisodes());
    } else if (ui->stackedWidget->currentIndex() == 3) {
        m_renamer->setRenameType(Renamer::TypeConcerts);
        m_renamer->setConcerts(ui->concertFilesWidget->selectedConcerts());
    } else {
        return;
    }
    m_renamer->exec();
}

/**
 * @brief Called when the filter text was changed or a filter was added/removed
 * Delegates the event down to the current subwidget
 * @param filters List of filters
 * @param text Filter text
 */
void MainWindow::onFilterChanged(QList<Filter *> filters, QString text)
{
    qDebug() << "Filter has changed";
    if (ui->stackedWidget->currentIndex() == 0)
        ui->filesWidget->setFilter(filters, text);
    else if (ui->stackedWidget->currentIndex() == 1)
        ui->tvShowFilesWidget->setFilter(filters, text);
    else if (ui->stackedWidget->currentIndex() == 3)
        ui->concertFilesWidget->setFilter(filters, text);
}

/**
 * @brief Sets the status of the save and save all action
 * @param enabled Status
 * @param widget Widget to set the status for
 */
void MainWindow::onSetSaveEnabled(bool enabled, MainWidgets widget)
{
    qDebug() << "Entered, enabled=" << enabled;

    m_actions[widget][ActionSave] = enabled;
    if (widget != WidgetMovieSets && widget != WidgetCertifications) {
        m_actions[widget][ActionSaveAll] = enabled;
        m_actions[widget][ActionRename] = enabled;
    }

    if ((widget == WidgetMovies && ui->stackedWidget->currentIndex() == 0) ||
        (widget == WidgetTvShows && ui->stackedWidget->currentIndex() == 1) ||
        (widget == WidgetConcerts && ui->stackedWidget->currentIndex() == 3)) {
        ui->navbar->setActionSaveEnabled(enabled);
        ui->navbar->setActionSaveAllEnabled(enabled);
        ui->navbar->setActionRenameEnabled(enabled);
    }
    if ((widget == WidgetMovieSets && ui->stackedWidget->currentIndex() == 2) ||
        (widget == WidgetCertifications && ui->stackedWidget->currentIndex() == 5) ||
        (widget == WidgetGenres && ui->stackedWidget->currentIndex() == 4))
        ui->navbar->setActionSaveEnabled(enabled);
}

/**
 * @brief Sets the status of the search action
 * @param enabled Status
 * @param widget Widget to set the status for
 */
void MainWindow::onSetSearchEnabled(bool enabled, MainWidgets widget)
{
    qDebug() << "Entered, enabled=" << enabled;
    m_actions[widget][ActionSearch] = enabled;

    if ((widget == WidgetMovies && ui->stackedWidget->currentIndex() == 0) ||
        (widget == WidgetTvShows && ui->stackedWidget->currentIndex() == 1) ||
        (widget == WidgetConcerts && ui->stackedWidget->currentIndex() == 3))
        ui->navbar->setActionSearchEnabled(enabled);
}

/**
 * @brief Moves all splitters
 * @param pos
 * @param index
 */
void MainWindow::moveSplitter(int pos, int index)
{
    Q_UNUSED(index)
    QList<int> sizes;
    QList<QSplitter*> splitters;
    splitters << ui->movieSplitter << ui->tvShowSplitter << ui->setsWidget->splitter() << ui->genreWidget->splitter()
              << ui->certificationWidget->splitter() << ui->concertSplitter;
    foreach (QSplitter *splitter, splitters) {
        if (splitter->sizes().at(0) == pos) {
            sizes = splitter->sizes();
            break;
        }
    }

    foreach (QSplitter *splitter, splitters)
        splitter->setSizes(sizes);

    Manager::instance()->movieModel()->update();
    Manager::instance()->concertModel()->update();
}

/**
 * @brief Sets or removes the new mark in the main menu on the left
 */
void MainWindow::setNewMarks()
{
    bool newMovies = Manager::instance()->movieModel()->hasNewMovies();
    bool newConcerts = Manager::instance()->concertModel()->hasNewConcerts();
    bool newTvShows = Manager::instance()->tvShowModel()->hasNewShowOrEpisode();

    QPainter painter;
    QPixmap star(":/img/star.png");
    QIcon movies = m_icons.value(WidgetMovies);
    QIcon concerts = m_icons.value(WidgetConcerts);
    QIcon shows = m_icons.value(WidgetTvShows);
    if (newMovies) {
        QPixmap pixmap = movies.pixmap(64, 64);
        painter.begin(&pixmap);
        painter.drawPixmap(pixmap.width()-star.width(), pixmap.height()-star.height(), star.width(), star.height(), star);
        painter.end();
        movies = QIcon(pixmap);
    }

    if (newConcerts) {
        QPixmap pixmap = concerts.pixmap(64, 64);
        painter.begin(&pixmap);
        painter.drawPixmap(pixmap.width()-star.width(), pixmap.height()-star.height(), star.width(), star.height(), star);
        painter.end();
        concerts = QIcon(pixmap);
    }

    if (newTvShows) {
        QPixmap pixmap = shows.pixmap(64, 64);
        painter.begin(&pixmap);
        painter.drawPixmap(pixmap.width()-star.width(), pixmap.height()-star.height(), star.width(), star.height(), star);
        painter.end();
        shows = QPixmap(pixmap);
    }

    ui->buttonMovies->setIcon(movies);
    ui->buttonConcerts->setIcon(concerts);
    ui->buttonTvshows->setIcon(shows);

    ui->filesWidget->setAlphaListData();
    ui->concertFilesWidget->setAlphaListData();
}

void MainWindow::setNewMarkForImports(bool hasItems)
{
    QPainter painter;
    QPixmap star(":/img/star.png");
    QIcon downloads = m_icons.value(WidgetDownloads);
    if (hasItems) {
        QPixmap pixmap = downloads.pixmap(64, 64);
        painter.begin(&pixmap);
        painter.drawPixmap(pixmap.width()-star.width(), pixmap.height()-star.height(), star.width(), star.height(), star);
        painter.end();
        downloads = QIcon(pixmap);
    }
    ui->buttonDownloads->setIcon(downloads);
}

void MainWindow::onActionXbmc()
{
    m_xbmcSync->exec();
}

void MainWindow::onTriggerReloadAll()
{
    m_fileScannerDialog->setForceReload(true);
    m_fileScannerDialog->setReloadType(FileScannerDialog::TypeAll);
    m_fileScannerDialog->exec();
}

void MainWindow::onXbmcSyncFinished()
{
    ui->filesWidget->movieSelectedEmitter();
    ui->tvShowFilesWidget->emitLastSelection();
    ui->concertFilesWidget->concertSelectedEmitter();
}

void MainWindow::onFilesRenamed(Renamer::RenameType type)
{
    m_fileScannerDialog->setForceReload(true);
    if (type == Renamer::TypeMovies)
        m_fileScannerDialog->setReloadType(FileScannerDialog::TypeMovies);
    else if (type == Renamer::TypeConcerts)
        m_fileScannerDialog->setReloadType(FileScannerDialog::TypeConcerts);
    else if (type == Renamer::TypeTvShows)
        m_fileScannerDialog->setReloadType(FileScannerDialog::TypeTvShows);
    else if (type == Renamer::TypeAll)
        m_fileScannerDialog->setReloadType(FileScannerDialog::TypeAll);
    m_fileScannerDialog->exec();
}

void MainWindow::onRenewModels()
{
    ui->filesWidget->renewModel();
    ui->tvShowFilesWidget->renewModel();
    ui->concertFilesWidget->renewModel();
    ui->downloadsWidget->scanDownloadFolders();
}

void MainWindow::onJumpToMovie(Movie *movie)
{
    onMenuMovies();
    ui->filesWidget->selectMovie(movie);
}

void MainWindow::onNewVersion(QString version)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle(tr("Updates available"));
    msgBox.setText(tr("%1 is now available.<br>Get it now on %2").arg(version).arg("<a href=\"http://www.mediaelch.de\">http://www.mediaelch.de</a>"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIconPixmap(QPixmap(":/img/MediaElch.png").scaledToWidth(64, Qt::SmoothTransformation));
    QCheckBox dontCheck(QObject::tr("Don't check for updates"), &msgBox);
    dontCheck.blockSignals(true);
    msgBox.addButton(&dontCheck, QMessageBox::ActionRole);
    msgBox.exec();
    if (dontCheck.checkState() == Qt::Checked) {
        Settings::instance()->setCheckForUpdates(false);
        Settings::instance()->saveSettings();
    }
}

void MainWindow::updateTvShows()
{
    foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows()) {
        if (show->showMissingEpisodes())
            TvShowUpdater::instance()->updateShow(show);
    }
}
