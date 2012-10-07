#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QTimer>

#include "concerts/ConcertSearch.h"
#include "data/MediaCenterInterface.h"
#include "data/ScraperInterface.h"
#include "globals/Globals.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "main/MessageBox.h"
#include "movies/MovieSearch.h"
#include "tvShows/TvShowSearch.h"
#include "sets/MovieListDialog.h"
#include "settings/Settings.h"

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qDebug() << "MediaElch version" << QApplication::applicationVersion() << "starting up";

    for (int i=WidgetMovies ; i!=WidgetCertifications ; i++) {
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
    m_settingsWidget = new SettingsWidget(ui->centralWidget);
    m_filterWidget = new FilterWidget(ui->mainToolBar);
    m_fileScannerDialog = new FileScannerDialog(ui->centralWidget);
    m_settings = Settings::instance(this);
    setupToolbar();

    MessageBox::instance(this)->reposition(this->size());
    Manager::instance();

    if (!m_settings->mainSplitterState().isNull()) {
        ui->movieSplitter->restoreState(m_settings->mainSplitterState());
        ui->tvShowSplitter->restoreState(m_settings->mainSplitterState());
        ui->setsWidget->splitter()->restoreState(m_settings->mainSplitterState());
        ui->concertSplitter->restoreState(m_settings->mainSplitterState());
        ui->genreWidget->splitter()->restoreState(m_settings->mainSplitterState());
        ui->certificationWidget->splitter()->restoreState(m_settings->mainSplitterState());
    }

    if (m_settings->mainWindowSize().isValid() && !m_settings->mainWindowPosition().isNull()) {
        #ifdef Q_WS_MAC
            // Ugly workaround from https://bugreports.qt-project.org/browse/QTBUG-3116
            // to fix invisible toolbar on mac
            bool workaround = !isVisible();
            if (workaround) {
                // make "invisible"
                setWindowOpacity(0); // let Qt update its frameStruts
                show();
            }
            resize(m_settings->mainWindowSize());
            if (workaround) {
                move(m_settings->mainWindowPosition());
                setWindowOpacity(1);
            }
        #else
        resize(m_settings->mainWindowSize());
        move(m_settings->mainWindowPosition());
        #endif
    }
    // Size for Screenshots
    // resize(1121, 735);

    connect(ui->filesWidget, SIGNAL(movieSelected(Movie*)), ui->movieWidget, SLOT(setMovie(Movie*)));
    connect(ui->filesWidget, SIGNAL(movieSelected(Movie*)), ui->movieWidget, SLOT(setEnabledTrue(Movie*)));
    connect(ui->filesWidget, SIGNAL(noMovieSelected()), ui->movieWidget, SLOT(clear()));
    connect(ui->filesWidget, SIGNAL(noMovieSelected()), ui->movieWidget, SLOT(setDisabledTrue()));

    connect(ui->concertFilesWidget, SIGNAL(concertSelected(Concert*)), ui->concertWidget, SLOT(setConcert(Concert*)));
    connect(ui->concertFilesWidget, SIGNAL(concertSelected(Concert*)), ui->concertWidget, SLOT(setEnabledTrue(Concert*)));
    connect(ui->concertFilesWidget, SIGNAL(noConcertSelected()), ui->concertWidget, SLOT(clear()));
    connect(ui->concertFilesWidget, SIGNAL(noConcertSelected()), ui->concertWidget, SLOT(setDisabledTrue()));

    connect(ui->tvShowFilesWidget, SIGNAL(sigTvShowSelected(TvShow*)), ui->tvShowWidget, SLOT(onTvShowSelected(TvShow*)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigEpisodeSelected(TvShowEpisode*)), ui->tvShowWidget, SLOT(onEpisodeSelected(TvShowEpisode*)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigTvShowSelected(TvShow*)), ui->tvShowWidget, SLOT(onSetEnabledTrue(TvShow*)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigEpisodeSelected(TvShowEpisode*)), ui->tvShowWidget, SLOT(onSetEnabledTrue(TvShowEpisode*)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigNothingSelected()), ui->tvShowWidget, SLOT(onClear()));
    connect(ui->tvShowFilesWidget, SIGNAL(sigNothingSelected()), ui->tvShowWidget, SLOT(onSetDisabledTrue()));

    connect(ui->movieWidget, SIGNAL(actorDownloadProgress(int,int,int)), this, SLOT(progressProgress(int,int,int)));
    connect(ui->movieWidget, SIGNAL(actorDownloadStarted(QString,int)), this, SLOT(progressStarted(QString,int)));
    connect(ui->movieWidget, SIGNAL(actorDownloadFinished(int)), this, SLOT(progressFinished(int)));

    connect(ui->tvShowWidget, SIGNAL(sigDownloadsStarted(QString,int)), this, SLOT(progressStarted(QString,int)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsProgress(int,int,int)), this, SLOT(progressProgress(int,int,int)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsFinished(int)), this, SLOT(progressFinished(int)));

    connect(m_filterWidget, SIGNAL(sigFilterTextChanged(QString)), this, SLOT(onFilterChanged(QString)));

    connect(ui->movieSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(moveSplitter(int,int)));
    connect(ui->tvShowSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(moveSplitter(int,int)));
    connect(ui->setsWidget->splitter(), SIGNAL(splitterMoved(int,int)), this, SLOT(moveSplitter(int,int)));
    connect(ui->genreWidget->splitter(), SIGNAL(splitterMoved(int,int)), this, SLOT(moveSplitter(int,int)));
    connect(ui->certificationWidget->splitter(), SIGNAL(splitterMoved(int,int)), this, SLOT(moveSplitter(int,int)));
    connect(ui->concertSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(moveSplitter(int,int)));

    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(tvShowsLoaded(int)), ui->tvShowFilesWidget, SLOT(renewModel()));

    Manager::instance()->setupMediaCenterInterface();

    MovieSearch::instance(ui->centralWidget);
    TvShowSearch::instance(ui->centralWidget);
    ImageDialog::instance(ui->centralWidget);
    MovieListDialog::instance(ui->centralWidget);
    ImagePreviewDialog::instance(ui->centralWidget);
    ConcertSearch::instance(ui->centralWidget);

    // Start scanning for files
    m_fileScannerDialog->exec();

#ifdef Q_WS_WIN
    setStyleSheet(styleSheet() + " #centralWidget { border-bottom: 1px solid rgba(0, 0, 0, 100); } ");

    QFont font = ui->labelMovies->font();
    font.setPointSize(font.pointSize()-3);
    font.setBold(true);
    ui->labelMovies->setFont(font);
    ui->labelConcerts->setFont(font);
    ui->labelShows->setFont(font);
#endif
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
    Manager::instance()->shutdownMediaCenterInterfaces();
    m_settings->setMainWindowSize(size());
    m_settings->setMainWindowPosition(pos());
    m_settings->setMainSplitterState(ui->movieSplitter->saveState());
    delete ui;
}

/**
 * @brief Repositions the MessageBox
 * @param event
 */
void MainWindow::resizeEvent(QResizeEvent *event)
{
    MessageBox::instance()->reposition(event->size());
    QWidget::resizeEvent(event);
}

/**
 * @brief Sets up the toolbar
 */
void MainWindow::setupToolbar()
{
    qDebug() << "Entered";
    setUnifiedTitleAndToolBarOnMac(true);

    QPainter p;
    QList<QPixmap> icons;
    icons << QPixmap(":/img/spanner.png") << QPixmap(":/img/info.png") << QPixmap(":/img/folder_in.png")
          << QPixmap(":/img/stop.png") << QPixmap(":/img/magnifier.png") <<QPixmap(":/img/save.png")
          << QPixmap(":/img/storage.png") << QPixmap(":/img/heart.png") << QPixmap(":/img/arrow_circle_right.png");
    for (int i=0, n=icons.count() ; i<n ; ++i) {
        p.begin(&icons[i]);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(icons[i].rect(), QColor(0, 0, 0, 100));
        p.end();
    }

    m_actionSearch = new QAction(QIcon(icons[4]), tr("Search"), this);
    m_actionSearch->setShortcut(QKeySequence::Find);
    m_actionSearch->setToolTip(tr("Search (%1)").arg(QKeySequence(QKeySequence::Find).toString(QKeySequence::NativeText)));

    m_actionSave = new QAction(QIcon(icons[5]), tr("Save"), this);
    m_actionSave->setShortcut(QKeySequence::Save);
    m_actionSave->setToolTip(tr("Save (%1)").arg(QKeySequence(QKeySequence::Save).toString(QKeySequence::NativeText)));

    m_actionSaveAll = new QAction(QIcon(icons[6]), tr("Save All"), this);
    QKeySequence seqSaveAll(Qt::CTRL+Qt::ShiftModifier+Qt::Key_S);
    m_actionSaveAll->setShortcut(seqSaveAll);
    m_actionSaveAll->setToolTip(tr("Save All (%1)").arg(seqSaveAll.toString(QKeySequence::NativeText)));

    m_actionReload = new QAction(QIcon(icons[8]), tr("Reload"), this);
    m_actionReload->setShortcut(QKeySequence::Refresh);
    m_actionReload->setToolTip(tr("Reload all files (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));

    m_actionSettings = new QAction(QIcon(icons[0]), tr("Settings"), this);

    m_actionAbout = new QAction(QIcon(icons[1]), tr("About"), this);
    m_actionQuit = new QAction(QIcon(icons[3]), tr("Quit"), this);

    m_actionLike = new QAction(QIcon(icons[7]), tr("Support"), this);

    ui->mainToolBar->addAction(m_actionSearch);
    ui->mainToolBar->addAction(m_actionSave);
    ui->mainToolBar->addAction(m_actionSaveAll);
    ui->mainToolBar->addAction(m_actionReload);
    ui->mainToolBar->addAction(m_actionSettings);
    ui->mainToolBar->addAction(m_actionAbout);
    ui->mainToolBar->addAction(m_actionQuit);
    ui->mainToolBar->addWidget(m_filterWidget);
#ifndef APPSTORE
    ui->mainToolBar->addAction(m_actionLike);
#endif

    connect(m_actionSearch, SIGNAL(triggered()), this, SLOT(onActionSearch()));
    connect(m_actionSave, SIGNAL(triggered()), this, SLOT(onActionSave()));
    connect(m_actionSaveAll, SIGNAL(triggered()), this, SLOT(onActionSaveAll()));
    connect(m_actionReload, SIGNAL(triggered()), this, SLOT(onActionReload()));
    connect(m_actionAbout, SIGNAL(triggered()), m_aboutDialog, SLOT(exec()));
    connect(m_actionSettings, SIGNAL(triggered()), m_settingsWidget, SLOT(exec()));
    connect(m_actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(m_actionLike, SIGNAL(triggered()), m_supportDialog, SLOT(exec()));

    m_actionSearch->setEnabled(false);
    m_actionSave->setEnabled(false);
    m_actionSaveAll->setEnabled(false);

#ifdef Q_WS_WIN
    ui->mainToolBar->setStyleSheet("QToolButton {border: 0; padding: 5px;} QToolBar { border-bottom: 1px solid rgba(0, 0, 0, 100); }");
#endif
}

/**
 * @brief Called when a subwidget starts a progress, displays a progress MessageBox
 * @param msg Message
 * @param id (Unique) Id of the progress
 */
void MainWindow::progressStarted(QString msg, int id)
{
    qDebug() << "Entered, msg=" << msg << "id=" << id;
    MessageBox::instance()->showProgressBar(msg, id);
}

/**
 * @brief Updates the progress MessageBox
 * @param current Current value
 * @param max Maximum value
 * @param id (Unique) Id of the progress
 */
void MainWindow::progressProgress(int current, int max, int id)
{
    MessageBox::instance()->progressBarProgress(current, max, id);
}

/**
 * @brief Called when a progress has finished
 * @param id (Unique) Id of the progress
 */
void MainWindow::progressFinished(int id)
{
    qDebug() << "Entered, id=" << id;
    MessageBox::instance()->hideProgressBar(id);
}

/**
 * @brief Restores all menu icons to defaults and enables actions
 * @param widget Current widget
 */
void MainWindow::onMenu(MainWidgets widget)
{
    ui->buttonMovies->setIcon(QIcon(":/img/video_menu.png"));
    ui->buttonMovieSets->setIcon(QIcon(":/img/movieSets_menu.png"));
    ui->buttonGenres->setIcon(QIcon(":/img/genre_menu.png"));
    ui->buttonCertifications->setIcon(QIcon(":/img/certification2_menu.png"));
    ui->buttonTvshows->setIcon(QIcon(":/img/display_on_menu.png"));
    ui->buttonConcerts->setIcon(QIcon(":/img/concerts_menu.png"));

    m_actionSearch->setEnabled(m_actions[widget][ActionSearch]);
    m_actionSave->setEnabled(m_actions[widget][ActionSave]);
    m_actionSaveAll->setEnabled(m_actions[widget][ActionSaveAll]);
    m_filterWidget->setEnabled(m_actions[widget][ActionFilterWidget]);
}

/**
 * @brief Called when the menu item "Movies" was clicked
 * Updates menu icons and sets status of actions
 */
void MainWindow::onMenuMovies()
{
    qDebug() << "Entered";
    onMenu(WidgetMovies);
    ui->buttonMovies->setIcon(QIcon(":/img/video_menuActive.png"));
    ui->stackedWidget->setCurrentIndex(0);
}

/**
 * @brief Called when the menu item "Movie Sets" was clicked
 * Updates menu icons and sets status of actions
 */
void MainWindow::onMenuMovieSets()
{
    qDebug() << "Entered";
    onMenu(WidgetMovies);
    ui->buttonMovieSets->setIcon(QIcon(":/img/movieSets_menuActive.png"));
    ui->stackedWidget->setCurrentIndex(2);
    ui->setsWidget->loadSets();
}

/**
 * @brief Called when the menu item "Movie Genres" was clicked
 * Updates menu icons and sets status of actions
 */
void MainWindow::onMenuGenres()
{
    qDebug() << "Entered";
    onMenu(WidgetGenres);
    ui->buttonGenres->setIcon(QIcon(":/img/genre_menuActive.png"));
    ui->stackedWidget->setCurrentIndex(4);
    ui->genreWidget->loadGenres();
}

/**
 * @brief Called when the menu item "Movie Certifications" was clicked
 * Updates menu icons and sets status of actions
 */
void MainWindow::onMenuCertifications()
{
    qDebug() << "Entered";
    onMenu(WidgetCertifications);
    ui->buttonCertifications->setIcon(QIcon(":/img/certification2_menuActive.png"));
    ui->stackedWidget->setCurrentIndex(5);
    ui->certificationWidget->loadCertifications();
}

/**
 * @brief Called when the menu item "Tv Shows" was clicked
 * Updates menu icons and sets status of actions
 */
void MainWindow::onMenuTvShows()
{
    qDebug() << "Entered";
    onMenu(WidgetTvShows);
    ui->buttonTvshows->setIcon(QIcon(":/img/display_on_menuActive.png"));
    ui->stackedWidget->setCurrentIndex(1);
}

/**
 * @brief Called when the menu item "Concerts" was clicked
 * Updates menu icons and sets status of actions
 */
void MainWindow::onMenuConcerts()
{
    qDebug() << "Entered";
    onMenu(WidgetConcerts);
    ui->buttonConcerts->setIcon(QIcon(":/img/concerts_menuActive.png"));
    ui->stackedWidget->setCurrentIndex(3);
}

/**
 * @brief Called when the action "Search" was clicked
 * Delegates the event down to the current subwidget
 */
void MainWindow::onActionSearch()
{
    qDebug() << "Entered, currentIndex=" << ui->stackedWidget->currentIndex();
    if (ui->stackedWidget->currentIndex() == 0) {
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
        QTimer::singleShot(0, ui->movieWidget, SLOT(saveInformation()));
    else if (ui->stackedWidget->currentIndex() == 1)
        QTimer::singleShot(0, ui->tvShowWidget, SLOT(onSaveInformation()));
    else if (ui->stackedWidget->currentIndex() == 2)
        QTimer::singleShot(0, ui->setsWidget, SLOT(saveSet()));
    else if (ui->stackedWidget->currentIndex() == 3)
        QTimer::singleShot(0, ui->concertWidget, SLOT(onSaveInformation()));
    else if (ui->stackedWidget->currentIndex() == 4)
        QTimer::singleShot(0, ui->genreWidget, SLOT(onSaveInformation()));
    else if (ui->stackedWidget->currentIndex() == 5)
        QTimer::singleShot(0, ui->certificationWidget, SLOT(onSaveInformation()));
}

/**
 * @brief Called when the action "Save all" was clicked
 * Delegates the event down to the current subwidget
 */
void MainWindow::onActionSaveAll()
{
    qDebug() << "Entered, currentIndex=" << ui->stackedWidget->currentIndex();
    if (ui->stackedWidget->currentIndex() == 0)
        QTimer::singleShot(0, ui->movieWidget, SLOT(saveAll()));
    else if (ui->stackedWidget->currentIndex() == 1)
        QTimer::singleShot(0, ui->tvShowWidget, SLOT(onSaveAll()));
    else if (ui->stackedWidget->currentIndex() == 3)
        QTimer::singleShot(0, ui->concertWidget, SLOT(onSaveAll()));
}

/**
 * @brief Executes the file scanner dialog
 */
void MainWindow::onActionReload()
{
    m_fileScannerDialog->exec();
}

/**
 * @brief Called when the filter text was changed
 * Delegates the event down to the current subwidget
 * @param text Filter text
 */
void MainWindow::onFilterChanged(QString text)
{
    qDebug() << "Entered, text=" << text << "currentIndex=" << ui->stackedWidget->currentIndex();
    if (ui->stackedWidget->currentIndex() == 0)
        ui->filesWidget->setFilter(text);
    else if (ui->stackedWidget->currentIndex() == 1)
        ui->tvShowFilesWidget->setFilter(text);
    else if (ui->stackedWidget->currentIndex() == 3)
        ui->concertFilesWidget->setFilter(text);
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
    if (widget != WidgetMovieSets && widget != WidgetCertifications)
        m_actions[widget][ActionSaveAll] = enabled;

    if ((widget == WidgetMovies && ui->stackedWidget->currentIndex() == 0) ||
        (widget == WidgetTvShows && ui->stackedWidget->currentIndex() == 1) ||
        (widget == WidgetConcerts && ui->stackedWidget->currentIndex() == 3)) {
        m_actionSave->setEnabled(enabled);
        m_actionSaveAll->setEnabled(enabled);
    }
    if ((widget == WidgetMovieSets && ui->stackedWidget->currentIndex() == 2) ||
        (widget == WidgetCertifications && ui->stackedWidget->currentIndex() == 5) ||
        (widget == WidgetGenres && ui->stackedWidget->currentIndex() == 4))
        m_actionSave->setEnabled(enabled);
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
        m_actionSearch->setEnabled(enabled);
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
}
