#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QTimer>

#include "data/MediaCenterInterface.h"
#include "data/ScraperInterface.h"
#include "ExportDialog.h"
#include "Manager.h"
#include "MovieImageDialog.h"
#include "MovieSearch.h"
#include "QuestionDialog.h"
#include "SettingsWidget.h"
#include "MessageBox.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_aboutDialog = new AboutDialog(ui->centralWidget);
    m_exportDialog = new ExportDialog(ui->centralWidget);
    m_filterWidget = new FilterWidget(ui->mainToolBar);
    m_settingsWidget = static_cast<SettingsWidget*>(ui->stackedWidget->widget(2));
    setupToolbar();

    MessageBox::instance(this)->reposition(this->size());
    Manager::instance();
    if (m_settingsWidget->mainWindowSize().isValid()) {
        resize(m_settingsWidget->mainWindowSize());
    }
    if (!m_settingsWidget->mainWindowPosition().isNull()) {
        move(m_settingsWidget->mainWindowPosition());
    }

    Manager::instance()->movieFileSearcher()->setMovieDirectories(m_settingsWidget->movieDirectories());

    connect(Manager::instance()->movieFileSearcher(), SIGNAL(moviesLoaded(int)), this, SLOT(progressFinished(int)));
    connect(Manager::instance()->movieFileSearcher(), SIGNAL(progress(int,int,int)), this, SLOT(progressProgress(int,int,int)));
    connect(Manager::instance()->movieFileSearcher(), SIGNAL(searchStarted(QString,int)), this, SLOT(progressStarted(QString,int)));
    connect(Manager::instance()->movieFileSearcher(), SIGNAL(moviesLoaded(int)), this, SLOT(setActionExportEnabled()));
    connect(Manager::instance()->movieFileSearcher(), SIGNAL(searchStarted(QString,int)), this, SLOT(setActionExportDisabled()));
    connect(ui->filesWidget, SIGNAL(movieSelected(Movie*)), ui->movieWidget, SLOT(setMovie(Movie*)));
    connect(ui->filesWidget, SIGNAL(movieSelected(Movie*)), ui->movieWidget, SLOT(setEnabledTrue()));
    connect(ui->filesWidget, SIGNAL(noMovieSelected()), ui->movieWidget, SLOT(clear()));
    connect(ui->filesWidget, SIGNAL(noMovieSelected()), ui->movieWidget, SLOT(setDisabledTrue()));
    connect(ui->filesWidget, SIGNAL(setRefreshButtonEnabled(bool)), m_actionRefreshFiles, SLOT(setEnabled(bool)));
    connect(ui->movieWidget, SIGNAL(actorDownloadProgress(int,int,int)), this, SLOT(progressProgress(int,int,int)));
    connect(ui->movieWidget, SIGNAL(actorDownloadStarted(QString,int)), this, SLOT(progressStarted(QString,int)));
    connect(ui->movieWidget, SIGNAL(actorDownloadFinished(int)), this, SLOT(progressFinished(int)));
    connect(ui->movieWidget, SIGNAL(movieChangeCanceled()), ui->filesWidget, SLOT(restoreLastSelection()));
    connect(ui->movieWidget, SIGNAL(setActionSaveEnabled(bool)), this, SLOT(setActionSaveEnabled(bool)));
    connect(ui->movieWidget, SIGNAL(setActionSearchEnabled(bool)), this, SLOT(setActionSearchEnabled(bool)));
    connect(m_filterWidget, SIGNAL(sigFilterTextChanged(QString)), this, SLOT(onFilterChanged(QString)));
    connect(ui->buttonMovies, SIGNAL(clicked()), this, SLOT(onMenuMovies()));
    connect(ui->buttonTvshows, SIGNAL(clicked()), this, SLOT(onMenuTvShows()));
    connect(ui->buttonSettings, SIGNAL(clicked()), this, SLOT(onMenuSettings()));

    MovieSearch::instance(ui->centralWidget);
    MovieImageDialog::instance(ui->centralWidget);
    QuestionDialog::instance(ui->centralWidget);
    Manager::instance()->movieFileSearcher()->start();

#ifdef Q_WS_WIN
    setStyleSheet(styleSheet() + " #centralWidget { border-bottom: 1px solid rgba(0, 0, 0, 100); } ");
#endif
}

MainWindow::~MainWindow()
{
    m_settingsWidget->setMainWindowSize(size());
    m_settingsWidget->setMainWindowPosition(pos());
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

    m_actionSearch = new QAction(QIcon(search), tr("Search"), this);
    m_actionSave = new QAction(QIcon(save), tr("Save"), this);
    m_actionRefreshFiles = new QAction(QIcon(refresh), tr("Reload"), this);
    m_actionRefreshFiles->setToolTip(tr("Reload Movie List"));
    m_actionExport = new QAction(QIcon(exportDb), tr("Export"), this);
    m_actionExport->setToolTip(tr("Export Movie Database"));
    m_actionAbout = new QAction(QIcon(info), tr("About"), this);
    m_actionQuit = new QAction(QIcon(quit), tr("Quit"), this);
    ui->mainToolBar->addAction(m_actionSearch);
    ui->mainToolBar->addAction(m_actionSave);
    ui->mainToolBar->addAction(m_actionRefreshFiles);
    ui->mainToolBar->addAction(m_actionExport);
    ui->mainToolBar->addAction(m_actionAbout);
    ui->mainToolBar->addAction(m_actionQuit);
    ui->mainToolBar->addWidget(m_filterWidget);

    connect(m_actionSearch, SIGNAL(triggered()), ui->movieWidget, SLOT(startScraperSearch()));
    connect(m_actionSave, SIGNAL(triggered()), this, SLOT(onActionSave()));
    connect(m_actionRefreshFiles, SIGNAL(triggered()), ui->filesWidget, SLOT(startSearch()));
    connect(m_actionExport, SIGNAL(triggered()), m_exportDialog, SLOT(exec()));
    connect(m_actionAbout, SIGNAL(triggered()), m_aboutDialog, SLOT(exec()));
    connect(m_actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

    setActionSearchEnabled(false);
    setActionSaveEnabled(false);
    setActionExportDisabled(true);

#ifdef Q_WS_WIN
    ui->mainToolBar->setStyleSheet("QToolButton {border: 0; padding: 5px;} QToolBar { border-bottom: 1px solid rgba(0, 0, 0, 100); }");
#endif
}

void MainWindow::setActionSaveEnabled(bool enabled)
{
    m_actionSave->setEnabled(enabled && ui->stackedWidget->currentIndex() == 0);
}

void MainWindow::setActionSearchEnabled(bool enabled)
{
    m_actionSearch->setEnabled(enabled && ui->stackedWidget->currentIndex() == 0);
}

void MainWindow::setActionExportEnabled(bool enabled)
{
    m_actionExport->setEnabled(enabled && ui->stackedWidget->currentIndex() == 0);
}

void MainWindow::setActionExportDisabled(bool disabled)
{
    m_actionExport->setDisabled(disabled);
}

void MainWindow::progressStarted(QString msg, int id)
{
    ui->filesWidget->disableRefresh();
    MessageBox::instance()->showProgressBar(msg, id);
}

void MainWindow::progressProgress(int current, int max, int id)
{
    MessageBox::instance()->progressBarProgress(current, max, id);
}

void MainWindow::progressFinished(int id)
{
    if (Manager::instance()->movieModel()->movies().size() > 0)
        ui->filesWidget->hideFirstTime();
    ui->filesWidget->enableRefresh();
    MessageBox::instance()->hideProgressBar(id);
}

void MainWindow::onMenuMovies()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->buttonMovies->setIcon(QIcon(":/img/video_menuActive.png"));
    ui->buttonTvshows->setIcon(QIcon(":/img/display_on_menu.png"));
    ui->buttonSettings->setIcon(QIcon(":/img/spanner_menu.png"));
    m_actionSearch->setEnabled(true);
    m_actionSave->setEnabled(true);
    m_actionRefreshFiles->setEnabled(true);
    m_actionExport->setEnabled(true);
    m_filterWidget->setEnabled(true);
}

void MainWindow::onMenuTvShows()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->buttonMovies->setIcon(QIcon(":/img/video_menu.png"));
    ui->buttonTvshows->setIcon(QIcon(":/img/display_on_menuActive.png"));
    ui->buttonSettings->setIcon(QIcon(":/img/spanner_menu.png"));
    m_actionSearch->setEnabled(false);
    m_actionSave->setEnabled(false);
    m_actionRefreshFiles->setEnabled(false);
    m_actionExport->setEnabled(false);
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
    m_actionRefreshFiles->setEnabled(false);
    m_actionExport->setEnabled(false);
    m_filterWidget->setEnabled(false);
}

void MainWindow::onActionSave()
{
    if (ui->stackedWidget->currentIndex() == 0)
        QTimer::singleShot(0, ui->movieWidget, SLOT(saveInformation()));
    else if (ui->stackedWidget->currentIndex() == 2)
        QTimer::singleShot(0, ui->settingsWidget, SLOT(saveSettings()));
}

void MainWindow::onFilterChanged(QString text)
{
    if (ui->stackedWidget->currentIndex() == 0)
        ui->filesWidget->setFilter(text);
}
