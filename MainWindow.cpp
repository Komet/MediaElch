#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QTimer>

#include "data/MediaCenterInterface.h"
#include "data/ScraperInterface.h"
#include "ExportDialog.h"
#include "ImagePreviewDialog.h"
#include "Manager.h"
#include "MovieImageDialog.h"
#include "MovieSearch.h"
#include "QuestionDialog.h"
#include "SettingsWidget.h"
#include "TvShowSearch.h"
#include "MessageBox.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_movieActions.insert(ActionSave, false);
    m_movieActions.insert(ActionSearch, false);
    m_movieActions.insert(ActionRefresh, false);
    m_movieActions.insert(ActionExport, false);
    m_tvShowActions.insert(ActionSave, false);
    m_tvShowActions.insert(ActionSearch, false);
    m_tvShowActions.insert(ActionRefresh, false);
    m_tvShowActions.insert(ActionExport, false);

    m_aboutDialog = new AboutDialog(ui->centralWidget);
    m_exportDialog = new ExportDialog(ui->centralWidget);
    m_filterWidget = new FilterWidget(ui->mainToolBar);
    m_settingsWidget = static_cast<SettingsWidget*>(ui->stackedWidget->widget(2));
    setupToolbar();

    MessageBox::instance(this)->reposition(this->size());
    Manager::instance();
    if (m_settingsWidget->mainWindowSize().isValid())
        resize(m_settingsWidget->mainWindowSize());
    // Size for Screenshots
    // resize(1121, 735);
    if (!m_settingsWidget->mainWindowPosition().isNull())
        move(m_settingsWidget->mainWindowPosition());
    if (!m_settingsWidget->movieSplitterState().isNull())
        ui->movieSplitter->restoreState(m_settingsWidget->movieSplitterState());
    if (!m_settingsWidget->tvShowSplitterState().isNull())
        ui->tvShowSplitter->restoreState(m_settingsWidget->tvShowSplitterState());

    Manager::instance()->movieFileSearcher()->setMovieDirectories(m_settingsWidget->movieDirectories());
    Manager::instance()->tvShowFileSearcher()->setMovieDirectories(m_settingsWidget->tvShowDirectories());

    connect(ui->filesWidget, SIGNAL(setRefreshButtonEnabled(bool,MainWidgets)), this, SLOT(onSetRefreshEnabled(bool,MainWidgets)));
    connect(ui->filesWidget, SIGNAL(movieSelected(Movie*)), ui->movieWidget, SLOT(setMovie(Movie*)));
    connect(ui->filesWidget, SIGNAL(movieSelected(Movie*)), ui->movieWidget, SLOT(setEnabledTrue(Movie*)));
    connect(ui->filesWidget, SIGNAL(noMovieSelected()), ui->movieWidget, SLOT(clear()));
    connect(ui->filesWidget, SIGNAL(noMovieSelected()), ui->movieWidget, SLOT(setDisabledTrue()));

    connect(ui->tvShowFilesWidget, SIGNAL(setRefreshButtonEnabled(bool,MainWidgets)), this, SLOT(onSetRefreshEnabled(bool,MainWidgets)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigTvShowSelected(TvShow*)), ui->tvShowWidget, SLOT(onTvShowSelected(TvShow*)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigEpisodeSelected(TvShowEpisode*)), ui->tvShowWidget, SLOT(onEpisodeSelected(TvShowEpisode*)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigTvShowSelected(TvShow*)), ui->tvShowWidget, SLOT(onSetEnabledTrue(TvShow*)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigEpisodeSelected(TvShowEpisode*)), ui->tvShowWidget, SLOT(onSetEnabledTrue(TvShowEpisode*)));
    connect(ui->tvShowFilesWidget, SIGNAL(sigNothingSelected()), ui->tvShowWidget, SLOT(onClear()));
    connect(ui->tvShowFilesWidget, SIGNAL(sigNothingSelected()), ui->tvShowWidget, SLOT(onSetDisabledTrue()));

    connect(Manager::instance()->movieFileSearcher(), SIGNAL(moviesLoaded(int)), this, SLOT(progressFinished(int)));
    connect(Manager::instance()->movieFileSearcher(), SIGNAL(progress(int,int,int)), this, SLOT(progressProgress(int,int,int)));
    connect(Manager::instance()->movieFileSearcher(), SIGNAL(searchStarted(QString,int)), this, SLOT(progressStarted(QString,int)));

    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(tvShowsLoaded(int)), this, SLOT(progressFinished(int)));
    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(progress(int,int,int)), this, SLOT(progressProgress(int,int,int)));
    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(searchStarted(QString,int)), this, SLOT(progressStarted(QString,int)));

    connect(ui->movieWidget, SIGNAL(actorDownloadProgress(int,int,int)), this, SLOT(progressProgress(int,int,int)));
    connect(ui->movieWidget, SIGNAL(actorDownloadStarted(QString,int)), this, SLOT(progressStarted(QString,int)));
    connect(ui->movieWidget, SIGNAL(actorDownloadFinished(int)), this, SLOT(progressFinished(int)));
    connect(ui->movieWidget, SIGNAL(setActionSaveEnabled(bool, MainWidgets)), this, SLOT(onSetSaveEnabled(bool, MainWidgets)));
    connect(ui->movieWidget, SIGNAL(setActionSearchEnabled(bool, MainWidgets)), this, SLOT(onSetSearchEnabled(bool, MainWidgets)));

    connect(ui->tvShowWidget, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)), this, SLOT(onSetSaveEnabled(bool,MainWidgets)));
    connect(ui->tvShowWidget, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)), this, SLOT(onSetSearchEnabled(bool,MainWidgets)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsStarted(QString,int)), this, SLOT(progressStarted(QString,int)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsProgress(int,int,int)), this, SLOT(progressProgress(int,int,int)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsFinished(int)), this, SLOT(progressFinished(int)));

    connect(m_filterWidget, SIGNAL(sigFilterTextChanged(QString)), this, SLOT(onFilterChanged(QString)));

    connect(ui->buttonMovies, SIGNAL(clicked()), this, SLOT(onMenuMovies()));
    connect(ui->buttonTvshows, SIGNAL(clicked()), this, SLOT(onMenuTvShows()));
    connect(ui->buttonSettings, SIGNAL(clicked()), this, SLOT(onMenuSettings()));

    connect(ui->movieSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(onMovieSplitterMoved()));
    connect(ui->tvShowSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(onTvShowSplitterMoved()));

    Manager::instance()->setupMediaCenterInterface();

    MovieSearch::instance(ui->centralWidget);
    TvShowSearch::instance(ui->centralWidget);
    MovieImageDialog::instance(ui->centralWidget);
    QuestionDialog::instance(ui->centralWidget);
    ImagePreviewDialog::instance(ui->centralWidget);

    // start TV Show File Searcher after Movie File Searcher has finished
    Manager::instance()->movieFileSearcher()->start();
    connect(Manager::instance()->movieFileSearcher(), SIGNAL(moviesLoaded(int)), Manager::instance()->tvShowFileSearcher(), SLOT(start()));

#ifdef Q_WS_WIN
    setStyleSheet(styleSheet() + " #centralWidget { border-bottom: 1px solid rgba(0, 0, 0, 100); } ");
#endif
}

MainWindow::~MainWindow()
{
    Manager::instance()->shutdownMediaCenterInterfaces();
    m_settingsWidget->setMainWindowSize(size());
    m_settingsWidget->setMainWindowPosition(pos());
    m_settingsWidget->setMovieSplitterState(ui->movieSplitter->saveState());
    m_settingsWidget->setTvShowSplitterState(ui->tvShowSplitter->saveState());
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    MessageBox::instance()->reposition(event->size());
    QWidget::resizeEvent(event);
}

void MainWindow::setupToolbar()
{
    setUnifiedTitleAndToolBarOnMac(true);

    QPixmap spanner(":/img/spanner.png");
    QPixmap info(":/img/info.png");
    QPixmap refresh(":/img/arrow_circle_right.png");
    QPixmap exportDb(":/img/folder_in.png");
    QPixmap quit(":/img/stop.png");
    QPixmap search(":/img/magnifier.png");
    QPixmap save(":/img/save.png");
    QPixmap saveAll(":/img/storage.png");
    QPainter p;
    p.begin(&search);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(search.rect(), QColor(0, 0, 0, 100));
    p.end();
    p.begin(&save);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(save.rect(), QColor(0, 0, 0, 100));
    p.end();
    p.begin(&refresh);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(refresh.rect(), QColor(0, 0, 0, 100));
    p.end();
    p.begin(&spanner);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(spanner.rect(), QColor(0, 0, 0, 100));
    p.end();
    p.begin(&exportDb);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(exportDb.rect(), QColor(0, 0, 0, 100));
    p.end();
    p.begin(&info);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(info.rect(), QColor(0, 0, 0, 100));
    p.end();
    p.begin(&quit);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(quit.rect(), QColor(0, 0, 0, 100));
    p.end();
    p.begin(&saveAll);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(saveAll.rect(), QColor(0, 0, 0, 100));
    p.end();

    m_actionSearch = new QAction(QIcon(search), tr("Search"), this);
    m_actionSave = new QAction(QIcon(save), tr("Save"), this);
    m_actionSaveAll = new QAction(QIcon(saveAll), tr("Save All"), this);
    m_actionRefreshFiles = new QAction(QIcon(refresh), tr("Reload"), this);
    m_actionRefreshFiles->setToolTip(tr("Reload Movie List"));
    m_actionExport = new QAction(QIcon(exportDb), tr("Export"), this);
    m_actionExport->setToolTip(tr("Export Movie Database"));
    m_actionAbout = new QAction(QIcon(info), tr("About"), this);
    m_actionQuit = new QAction(QIcon(quit), tr("Quit"), this);
    ui->mainToolBar->addAction(m_actionSearch);
    ui->mainToolBar->addAction(m_actionSave);
    ui->mainToolBar->addAction(m_actionSaveAll);
    ui->mainToolBar->addAction(m_actionRefreshFiles);
    // @TODO: currently disabled as the whole exports need rethinking... ;)
    // ui->mainToolBar->addAction(m_actionExport);
    ui->mainToolBar->addAction(m_actionAbout);
    ui->mainToolBar->addAction(m_actionQuit);
    ui->mainToolBar->addWidget(m_filterWidget);

    connect(m_actionSearch, SIGNAL(triggered()), this, SLOT(onActionSearch()));
    connect(m_actionSave, SIGNAL(triggered()), this, SLOT(onActionSave()));
    connect(m_actionSaveAll, SIGNAL(triggered()), this, SLOT(onActionSaveAll()));
    connect(m_actionRefreshFiles, SIGNAL(triggered()), this, SLOT(onActionRefresh()));
    connect(m_actionExport, SIGNAL(triggered()), m_exportDialog, SLOT(exec()));
    connect(m_actionAbout, SIGNAL(triggered()), m_aboutDialog, SLOT(exec()));
    connect(m_actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

    m_actionSearch->setEnabled(false);
    m_actionSave->setEnabled(false);
    m_actionSaveAll->setEnabled(false);
    m_actionExport->setEnabled(false);

#ifdef Q_WS_WIN
    ui->mainToolBar->setStyleSheet("QToolButton {border: 0; padding: 5px;} QToolBar { border-bottom: 1px solid rgba(0, 0, 0, 100); }");
#endif
}

void MainWindow::progressStarted(QString msg, int id)
{
    if (id == Constants::MovieFileSearcherProgressMessageId) {
        ui->filesWidget->disableRefresh();
        onSetExportEnabled(false, WidgetMovies);
    } else if (id == Constants::TvShowSearcherProgressMessageId) {
        ui->tvShowFilesWidget->disableRefresh();
        onSetExportEnabled(false, WidgetTvShows);
    } else if (id == Constants::MovieWidgetProgressMessageId) {
        onSetExportEnabled(false, WidgetMovies);
        onSetRefreshEnabled(false, WidgetMovies);
    }
    MessageBox::instance()->showProgressBar(msg, id);
}

void MainWindow::progressProgress(int current, int max, int id)
{
    MessageBox::instance()->progressBarProgress(current, max, id);
}

void MainWindow::progressFinished(int id)
{
    if (id == Constants::MovieFileSearcherProgressMessageId) {
        ui->filesWidget->enableRefresh();
        onSetExportEnabled(true, WidgetMovies);
    } else if (id == Constants::TvShowSearcherProgressMessageId) {
        ui->tvShowFilesWidget->renewModel();
        ui->tvShowFilesWidget->enableRefresh();
        onSetExportEnabled(true, WidgetTvShows);
    } else if (id == Constants::MovieWidgetProgressMessageId) {
        onSetRefreshEnabled(true, WidgetMovies);
        onSetExportEnabled(true, WidgetMovies);
    }
    MessageBox::instance()->hideProgressBar(id);
}

void MainWindow::onMenuMovies()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->buttonMovies->setIcon(QIcon(":/img/video_menuActive.png"));
    ui->buttonTvshows->setIcon(QIcon(":/img/display_on_menu.png"));
    ui->buttonSettings->setIcon(QIcon(":/img/spanner_menu.png"));
    m_actionSearch->setEnabled(m_movieActions[ActionSearch]);
    m_actionSave->setEnabled(m_movieActions[ActionSave]);
    m_actionSaveAll->setEnabled(m_movieActions[ActionSave]);
    m_actionRefreshFiles->setEnabled(m_movieActions[ActionRefresh]);
    m_actionExport->setEnabled(m_movieActions[ActionExport]);
    m_filterWidget->setEnabled(true);
}

void MainWindow::onMenuTvShows()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->buttonMovies->setIcon(QIcon(":/img/video_menu.png"));
    ui->buttonTvshows->setIcon(QIcon(":/img/display_on_menuActive.png"));
    ui->buttonSettings->setIcon(QIcon(":/img/spanner_menu.png"));
    m_actionSearch->setEnabled(m_tvShowActions[ActionSearch]);
    m_actionSave->setEnabled(m_tvShowActions[ActionSave]);
    m_actionSaveAll->setEnabled(m_tvShowActions[ActionSave]);
    m_actionRefreshFiles->setEnabled(m_tvShowActions[ActionRefresh]);
    m_actionExport->setEnabled(m_tvShowActions[ActionExport]);
    m_filterWidget->setEnabled(true);
}

void MainWindow::onMenuSettings()
{
    m_settingsWidget->loadSettings();
    ui->stackedWidget->setCurrentIndex(2);
    ui->buttonMovies->setIcon(QIcon(":/img/video_menu.png"));
    ui->buttonTvshows->setIcon(QIcon(":/img/display_on_menu.png"));
    ui->buttonSettings->setIcon(QIcon(":/img/spanner_menuActive.png"));
    m_actionSearch->setEnabled(false);
    m_actionSave->setEnabled(true);
    m_actionSaveAll->setEnabled(false);
    m_actionRefreshFiles->setEnabled(false);
    m_actionExport->setEnabled(false);
    m_filterWidget->setEnabled(false);
}

void MainWindow::onActionSearch()
{
    if (ui->stackedWidget->currentIndex() == 0)
        QTimer::singleShot(0, ui->movieWidget, SLOT(startScraperSearch()));
    else if (ui->stackedWidget->currentIndex() == 1)
        QTimer::singleShot(0, ui->tvShowWidget, SLOT(onStartScraperSearch()));
}

void MainWindow::onActionSave()
{
    if (ui->stackedWidget->currentIndex() == 0)
        QTimer::singleShot(0, ui->movieWidget, SLOT(saveInformation()));
    else if (ui->stackedWidget->currentIndex() == 1)
        QTimer::singleShot(0, ui->tvShowWidget, SLOT(onSaveInformation()));
    else if (ui->stackedWidget->currentIndex() == 2)
        QTimer::singleShot(0, ui->settingsWidget, SLOT(saveSettings()));
}

void MainWindow::onActionSaveAll()
{
    if (ui->stackedWidget->currentIndex() == 0)
        QTimer::singleShot(0, ui->movieWidget, SLOT(saveAll()));
    else if (ui->stackedWidget->currentIndex() == 1)
        QTimer::singleShot(0, ui->tvShowWidget, SLOT(onSaveAll()));
}

void MainWindow::onActionRefresh()
{
    if (ui->stackedWidget->currentIndex() == 0)
        QTimer::singleShot(0, ui->filesWidget, SLOT(startSearch()));
    else if (ui->stackedWidget->currentIndex() == 1)
        QTimer::singleShot(0, ui->tvShowFilesWidget, SLOT(startSearch()));

}

void MainWindow::onFilterChanged(QString text)
{
    if (ui->stackedWidget->currentIndex() == 0)
        ui->filesWidget->setFilter(text);
    else if (ui->stackedWidget->currentIndex() == 1)
        ui->tvShowFilesWidget->setFilter(text);
}

void MainWindow::onSetRefreshEnabled(bool enabled, MainWidgets widget)
{
    if (widget == WidgetMovies)
        m_movieActions[ActionRefresh] = enabled;
    else if (widget == WidgetTvShows)
        m_tvShowActions[ActionRefresh] = enabled;

    if ((widget == WidgetMovies && ui->stackedWidget->currentIndex() == 0) ||
        (widget == WidgetTvShows && ui->stackedWidget->currentIndex() == 1))
        m_actionRefreshFiles->setEnabled(enabled);
}

void MainWindow::onSetExportEnabled(bool enabled, MainWidgets widget)
{
    if (widget == WidgetMovies)
        m_movieActions[ActionExport] = enabled;
    else if (widget == WidgetTvShows)
        m_tvShowActions[ActionExport] = enabled;

    if ((widget == WidgetMovies && ui->stackedWidget->currentIndex() == 0) ||
        (widget == WidgetTvShows && ui->stackedWidget->currentIndex() == 1))
        m_actionExport->setEnabled(enabled);
}

void MainWindow::onSetSaveEnabled(bool enabled, MainWidgets widget)
{
    if (widget == WidgetMovies)
        m_movieActions[ActionSave] = enabled;
    else if (widget == WidgetTvShows)
        m_tvShowActions[ActionSave] = enabled;

    if ((widget == WidgetMovies && ui->stackedWidget->currentIndex() == 0) ||
        (widget == WidgetTvShows && ui->stackedWidget->currentIndex() == 1)) {
        m_actionSave->setEnabled(enabled);
        m_actionSaveAll->setEnabled(enabled);
    }
}

void MainWindow::onSetSearchEnabled(bool enabled, MainWidgets widget)
{
    if (widget == WidgetMovies)
        m_movieActions[ActionSearch] = enabled;
    else if (widget == WidgetTvShows)
        m_tvShowActions[ActionSearch] = enabled;

    if ((widget == WidgetMovies && ui->stackedWidget->currentIndex() == 0) ||
        (widget == WidgetTvShows && ui->stackedWidget->currentIndex() == 1))
        m_actionSearch->setEnabled(enabled);
}

void MainWindow::onMovieSplitterMoved()
{
    ui->tvShowSplitter->restoreState(ui->movieSplitter->saveState());
}

void MainWindow::onTvShowSplitterMoved()
{
    ui->movieSplitter->restoreState(ui->tvShowSplitter->saveState());
}
