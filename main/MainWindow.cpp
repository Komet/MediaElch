#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QCheckBox>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QPainter>
#include <QPluginLoader>
#include <QTimer>
#include <QToolBar>

#include "concerts/ConcertSearch.h"
#include "data/MediaCenterInterface.h"
#include "data/Storage.h"
#include "globals/NameFormatter.h"
#include "data/ScraperInterface.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "globals/TrailerDialog.h"
#include "notifications/NotificationBox.h"
#include "main/Update.h"
#include "movies/MovieMultiScrapeDialog.h"
#include "movies/MovieSearch.h"
#include "notifications/Notificator.h"
#include "plugins/PluginInterface.h"
#include "plugins/PluginManager.h"
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

    m_aboutDialog = new AboutDialog(ui->centralWidget);
    m_supportDialog = new SupportDialog(ui->centralWidget);
    m_settingsWindow = new SettingsWindow(ui->centralWidget);
    m_fileScannerDialog = new FileScannerDialog(ui->centralWidget);
    m_xbmcSync = new XbmcSync(ui->centralWidget);
    m_renamer = new Renamer(ui->centralWidget);
    m_settings = Settings::instance(this);
    m_exportDialog = new ExportDialog(this);
    setupToolbar();

    foreach (QToolButton *btn, ui->menuWidget->findChildren<QToolButton*>())
        setIcons(btn);

    Helper::instance(this);
    NotificationBox::instance(this)->reposition(this->size());
    Manager::instance();
    PluginManager::instance(this);

    if (!m_settings->mainSplitterState().isNull()) {
        ui->movieSplitter->restoreState(m_settings->mainSplitterState());
        ui->tvShowSplitter->restoreState(m_settings->mainSplitterState());
        ui->setsWidget->splitter()->restoreState(m_settings->mainSplitterState());
        ui->concertSplitter->restoreState(m_settings->mainSplitterState());
        ui->genreWidget->splitter()->restoreState(m_settings->mainSplitterState());
        ui->certificationWidget->splitter()->restoreState(m_settings->mainSplitterState());
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

    foreach (QToolButton *btn, ui->menuWidget->findChildren<QToolButton*>())
        connect(btn, SIGNAL(clicked()), this, SLOT(onMenu()));

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
    connect(ui->downloadsWidget, SIGNAL(sigScanFinished(bool)), this, SLOT(setNewMarks()));

    connect(m_xbmcSync, SIGNAL(sigTriggerReload()), this, SLOT(onTriggerReloadAll()));
    connect(m_xbmcSync, SIGNAL(sigFinished()), this, SLOT(onXbmcSyncFinished()));

    connect(m_renamer, SIGNAL(sigFilesRenamed(Renamer::RenameType)), this, SLOT(onFilesRenamed(Renamer::RenameType)));

    connect(m_settingsWindow, SIGNAL(sigSaved()), this, SLOT(onRenewModels()), Qt::QueuedConnection);

    connect(ui->setsWidget, SIGNAL(sigJumpToMovie(Movie*)), this, SLOT(onJumpToMovie(Movie*)));
    connect(ui->certificationWidget, SIGNAL(sigJumpToMovie(Movie*)), this, SLOT(onJumpToMovie(Movie*)));
    connect(ui->genreWidget, SIGNAL(sigJumpToMovie(Movie*)), this, SLOT(onJumpToMovie(Movie*)));

    MovieSearch::instance(ui->centralWidget);
    TvShowSearch::instance(ui->centralWidget);
    ImageDialog::instance(ui->centralWidget);
    MovieListDialog::instance(ui->centralWidget);
    ImagePreviewDialog::instance(ui->centralWidget);
    ConcertSearch::instance(ui->centralWidget);
    TrailerDialog::instance(ui->centralWidget);
    TvTunesDialog::instance(ui->centralWidget);
    NameFormatter::instance(this);
    MovieMultiScrapeDialog::instance(ui->centralWidget);
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

    ui->labelPlugins->setVisible(false);
    connect(PluginManager::instance(), SIGNAL(sigAddPlugin(PluginInterface*)), this, SLOT(onAddPlugin(PluginInterface*)));
    connect(PluginManager::instance(), SIGNAL(sigRemovePlugin(PluginInterface*)), this, SLOT(onRemovePlugin(PluginInterface*)));
    PluginManager::instance()->loadPlugins();

    if (Settings::instance()->startupSection() == "tvshows")
        onMenu(ui->buttonTvshows);
    else if (Settings::instance()->startupSection() == "concerts")
        onMenu(ui->buttonConcerts);
    else if (Settings::instance()->startupSection() == "import")
        onMenu(ui->buttonDownloads);
    else
        onMenu(ui->buttonMovies);

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
    } else if (m_plugins.contains(ui->stackedWidget->currentIndex())) {
        m_plugins.value(ui->stackedWidget->currentIndex())->doAction(PluginInterface::ActionSearch);
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
    else if (m_plugins.contains(ui->stackedWidget->currentIndex()))
        m_plugins.value(ui->stackedWidget->currentIndex())->doAction(PluginInterface::ActionSave);
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
    else if (m_plugins.contains(ui->stackedWidget->currentIndex()))
        m_plugins.value(ui->stackedWidget->currentIndex())->doAction(PluginInterface::ActionSaveAll);
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

    if (m_plugins.contains(ui->stackedWidget->currentIndex())) {
        m_plugins.value(ui->stackedWidget->currentIndex())->doAction(PluginInterface::ActionReload);
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
    } else if (m_plugins.contains(ui->stackedWidget->currentIndex())) {
        m_plugins.value(ui->stackedWidget->currentIndex())->doAction(PluginInterface::ActionRename);
        return;
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

    foreach (QSplitter *splitter, splitters) {
        splitter->setSizes(sizes);
    }

    Manager::instance()->movieModel()->update();
    Manager::instance()->concertModel()->update();
}

/**
 * @brief Sets or removes the new mark in the main menu on the left
 */
void MainWindow::setNewMarks()
{
    if (Manager::instance()->movieModel()->hasNewMovies()) {
        if (ui->buttonMovies->property("isActive").toBool())
            ui->buttonMovies->setIcon(ui->buttonMovies->property("iconActiveNew").value<QIcon>());
        else
            ui->buttonMovies->setIcon(ui->buttonMovies->property("iconInactiveNew").value<QIcon>());
    }

    if (Manager::instance()->tvShowModel()->hasNewShowOrEpisode()) {
        if (ui->buttonTvshows->property("isActive").toBool())
            ui->buttonTvshows->setIcon(ui->buttonTvshows->property("iconActiveNew").value<QIcon>());
        else
            ui->buttonTvshows->setIcon(ui->buttonTvshows->property("iconInactiveNew").value<QIcon>());
    }

    if (Manager::instance()->concertModel()->hasNewConcerts()) {
        if (ui->buttonConcerts->property("isActive").toBool())
            ui->buttonConcerts->setIcon(ui->buttonConcerts->property("iconActiveNew").value<QIcon>());
        else
            ui->buttonConcerts->setIcon(ui->buttonConcerts->property("iconInactiveNew").value<QIcon>());
    }

    if (ui->downloadsWidget->hasNewItems()) {
        if (ui->buttonDownloads->property("isActive").toBool())
            ui->buttonDownloads->setIcon(ui->buttonDownloads->property("iconActiveNew").value<QIcon>());
        else
            ui->buttonDownloads->setIcon(ui->buttonDownloads->property("iconInactiveNew").value<QIcon>());
    }

    ui->filesWidget->setAlphaListData();
    ui->concertFilesWidget->setAlphaListData();
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
    onMenu(ui->buttonMovies);
    ui->filesWidget->selectMovie(movie);
}

void MainWindow::updateTvShows()
{
    foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows()) {
        if (show->showMissingEpisodes())
            TvShowUpdater::instance()->updateShow(show);
    }
}

void MainWindow::onAddPlugin(PluginInterface *plugin)
{
    int index = ui->stackedWidget->addWidget(plugin->widget());
    QToolButton *button = new QToolButton(this);
    button->setIconSize(QSize(28, 28));
    button->setIcon(plugin->menuIcon());
    button->setStyleSheet("QToolButton { border: 0; margin-left: 10px; margin-right: 10px;}");
    button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    button->setToolTip(plugin->name());
    button->setProperty("storage", Storage::toVariant(button, plugin));
    button->setProperty("page", index);
    button->setProperty("isPlugin", true);
    setIcons(button);
    m_plugins.insert(index, plugin);

    connect(button, SIGNAL(clicked()), this, SLOT(onMenu()));
    switch (plugin->section()) {
    case PluginInterface::SectionMovies:
        ui->layoutMovies->addWidget(button);
        break;
    case PluginInterface::SectionTvShows:
        ui->layoutTvShows->addWidget(button);
        break;
    case PluginInterface::SectionConcerts:
        ui->layoutConcerts->addWidget(button);
        break;
    case PluginInterface::SectionImport:
        ui->layoutImport->addWidget(button);
        break;
    default:
        ui->labelPlugins->setVisible(true);
        ui->layoutPlugins->addWidget(button);
        break;
    }
}

void MainWindow::onRemovePlugin(PluginInterface *plugin)
{
    int index = ui->stackedWidget->indexOf(plugin->widget());
    if (ui->stackedWidget->currentIndex() >= index)
        ui->stackedWidget->setCurrentIndex(0);

    QMap<int, PluginInterface*> plugins;
    QMapIterator<int, PluginInterface*> it(m_plugins);
    while (it.hasNext()) {
        it.next();
        if (it.value() == plugin)
            continue;
        int pIndex = it.key();
        if (pIndex > index)
            pIndex--;
        plugins.insert(pIndex, plugin);
    }
    m_plugins = plugins;

    foreach (QToolButton *btn, ui->menuWidget->findChildren<QToolButton*>()) {
        if (btn->property("isPlugin").toBool() && btn->property("storage").value<Storage*>()->pluginInterface() == plugin)
            btn->deleteLater();
    }
    ui->stackedWidget->removeWidget(plugin->widget());
}

void MainWindow::onMenu(QToolButton *button)
{
    if (button == 0)
        button = static_cast<QToolButton*>(QObject::sender());

    if (!button)
        return;


    foreach (QToolButton *btn, ui->menuWidget->findChildren<QToolButton*>()) {
        btn->setIcon(btn->property("iconInactive").value<QIcon>());
        btn->setProperty("isActive", false);
    }
    button->setIcon(button->property("iconActive").value<QIcon>());
    button->setProperty("isActive", true);
    setNewMarks();

    int page = button->property("page").toInt();

    if (button->property("isPlugin").toBool()) {
        PluginInterface *plugin = button->property("storage").value<Storage*>()->pluginInterface();
        ui->navbar->setActionSearchEnabled(plugin->enabledActions().contains(PluginInterface::ActionSearch));
        ui->navbar->setActionSaveEnabled(plugin->enabledActions().contains(PluginInterface::ActionSave));
        ui->navbar->setActionSaveAllEnabled(plugin->enabledActions().contains(PluginInterface::ActionSaveAll));
        ui->navbar->setActionReloadEnabled(plugin->enabledActions().contains(PluginInterface::ActionReload));
        ui->navbar->setActionRenameEnabled(plugin->enabledActions().contains(PluginInterface::ActionRename));
        ui->navbar->setFilterWidgetEnabled(false);
        ui->navbar->setReloadToolTip(tr("Reload (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
    } else {
        ui->navbar->setActionReloadEnabled(page == 0 || page == 1 || page == 3 || page == 6);
        MainWidgets widget;
        switch (page) {
        case 0:
            // Movies
            ui->navbar->setReloadToolTip(tr("Reload all Movies (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
            widget = WidgetMovies;
            break;
        case 1:
            // Tv Shows
            ui->navbar->setReloadToolTip(tr("Reload all TV Shows (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
            widget = WidgetTvShows;
            break;
        case 2:
            // Movie Sets
            widget = WidgetMovieSets;
            ui->setsWidget->loadSets();
            break;
        case 3:
            // Concerts
            ui->navbar->setReloadToolTip(tr("Reload all Concerts (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
            widget = WidgetConcerts;
            break;
        case 4:
            // Genres
            widget = WidgetGenres;
            ui->genreWidget->loadGenres();
            break;
        case 5:
            // Certification
            widget = WidgetCertifications;
            ui->certificationWidget->loadCertifications();
            break;
        case 6:
            // Import
            widget = WidgetDownloads;
            ui->navbar->setReloadToolTip(tr("Reload all Downloads (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));
            break;
        }
        ui->navbar->setActionSearchEnabled(m_actions[widget][ActionSearch]);
        ui->navbar->setActionSaveEnabled(m_actions[widget][ActionSave]);
        ui->navbar->setActionSaveAllEnabled(m_actions[widget][ActionSaveAll]);
        ui->navbar->setActionRenameEnabled(m_actions[widget][ActionRename]);
        ui->navbar->setFilterWidgetEnabled(m_actions[widget][ActionFilterWidget]);
        ui->navbar->setActiveWidget(widget);
    }

    ui->stackedWidget->setCurrentIndex(page);
}

void MainWindow::setIcons(QToolButton *button)
{
    // iconActive, iconInactive, iconActiveNew, iconInactiveNew
    QPainter p;
    QPixmap ribbon(":/menu/star.png");
    QPixmap pixmap = button->icon().pixmap(64, 64);

    p.begin(&pixmap);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(pixmap.rect(), QColor(180, 180, 180));
    p.end();
    button->setProperty("iconInactive", QIcon(pixmap));

    p.begin(&pixmap);
    p.drawPixmap(pixmap.width()-ribbon.width(), pixmap.height()-ribbon.height(), ribbon.width(), ribbon.height(), ribbon);
    p.end();
    button->setProperty("iconInactiveNew", QIcon(pixmap));

    pixmap = button->icon().pixmap(64, 64);
    p.begin(&pixmap);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(pixmap.rect(), QColor(70, 155, 198));
    p.end();
    button->setProperty("iconActive", QIcon(pixmap));

    p.begin(&pixmap);
    p.drawPixmap(pixmap.width()-ribbon.width(), pixmap.height()-ribbon.height(), ribbon.width(), ribbon.height(), ribbon);
    p.end();
    button->setProperty("iconActiveNew", QIcon(pixmap));
}
