#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QDir>
#include <QPainter>

#include "data/MediaCenterInterface.h"
#include "data/ScraperInterface.h"
#include "Manager.h"
#include "MovieImageDialog.h"
#include "MovieSearch.h"
#include "QuestionDialog.h"
#include "SettingsDialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_progressBar = new QProgressBar(ui->statusBar);
    m_progressBar->hide();
    m_aboutDialog = new AboutDialog(ui->centralWidget);
    setupToolbar();

    Manager::instance();
    SettingsDialog::instance(ui->centralWidget);

    if (SettingsDialog::instance()->mainWindowSize().isValid()) {
        resize(SettingsDialog::instance()->mainWindowSize());
    }
    if (!SettingsDialog::instance()->mainWindowPosition().isNull()) {
        move(SettingsDialog::instance()->mainWindowPosition());
    }

    Manager::instance()->movieFileSearcher()->setMovieDirectories(SettingsDialog::instance()->movieDirectories());

    connect(Manager::instance()->movieFileSearcher(), SIGNAL(moviesLoaded()), this, SLOT(progressFinished()));
    connect(Manager::instance()->movieFileSearcher(), SIGNAL(progress(int,int)), this, SLOT(progressProgress(int,int)));
    connect(Manager::instance()->movieFileSearcher(), SIGNAL(started()), this, SLOT(progressStarted()));
    connect(ui->filesWidget, SIGNAL(movieSelected(Movie*)), ui->movieWidget, SLOT(setMovie(Movie*)));
    connect(ui->filesWidget, SIGNAL(movieSelected(Movie*)), ui->movieWidget, SLOT(setEnabledTrue()));
    connect(ui->filesWidget, SIGNAL(noMovieSelected()), ui->movieWidget, SLOT(clear()));
    connect(ui->filesWidget, SIGNAL(noMovieSelected()), ui->movieWidget, SLOT(setDisabledTrue()));
    connect(ui->movieWidget, SIGNAL(actorDownloadProgress(int,int)), this, SLOT(progressProgress(int,int)));
    connect(ui->movieWidget, SIGNAL(actorDownloadStarted()), this, SLOT(progressStarted()));
    connect(ui->movieWidget, SIGNAL(actorDownloadFinished()), this, SLOT(progressFinished()));
    connect(ui->movieWidget, SIGNAL(movieChangeCanceled()), ui->filesWidget, SLOT(restoreLastSelection()));
    connect(ui->movieWidget, SIGNAL(setActionSaveEnabled(bool)), this, SLOT(setActionSaveEnabled(bool)));
    connect(ui->movieWidget, SIGNAL(setActionSearchEnabled(bool)), this, SLOT(setActionSearchEnabled(bool)));

    if (SettingsDialog::instance()->firstTime()) {
        ui->filesWidget->showFirstTime();
        ui->movieWidget->showFirstTime();
    }

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
    SettingsDialog::instance()->setMainWindowSize(size());
    SettingsDialog::instance()->setMainWindowPosition(pos());
    delete SettingsDialog::instance();
    delete ui;
}

void MainWindow::setupToolbar()
{
    setUnifiedTitleAndToolBarOnMac(true);

    QPixmap spanner(":/img/spanner.png");
    QPixmap info(":/img/info.png");
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
    p.begin(&spanner);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(spanner.rect(), QColor(0, 0, 0, 100));
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
    m_actionSettings = new QAction(QIcon(spanner), tr("Preferences"), this);
    m_actionAbout = new QAction(QIcon(info), tr("About"), this);
    m_actionQuit = new QAction(QIcon(quit), tr("Quit"), this);
    ui->mainToolBar->addAction(m_actionSearch);
    ui->mainToolBar->addAction(m_actionSave);
    ui->mainToolBar->addAction(m_actionSettings);
    ui->mainToolBar->addAction(m_actionAbout);
    ui->mainToolBar->addAction(m_actionQuit);

    connect(m_actionSearch, SIGNAL(triggered()), ui->movieWidget, SLOT(startScraperSearch()));
    connect(m_actionSave, SIGNAL(triggered()), ui->movieWidget, SLOT(saveInformation()));
    connect(m_actionSettings, SIGNAL(triggered()), this, SLOT(execSettingsDialog()));
    connect(m_actionAbout, SIGNAL(triggered()), m_aboutDialog, SLOT(exec()));
    connect(m_actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

    setActionSearchEnabled(false);
    setActionSaveEnabled(false);

#ifdef Q_WS_WIN
    ui->mainToolBar->setStyleSheet("QToolButton {border: 0;} QToolBar { border-bottom: 1px solid rgba(0, 0, 0, 100); }");
#endif
}

void MainWindow::setActionSaveEnabled(bool enabled)
{
    m_actionSave->setEnabled(enabled);
}

void MainWindow::setActionSearchEnabled(bool enabled)
{
    m_actionSearch->setEnabled(enabled);
}

void MainWindow::execSettingsDialog()
{
    int result = SettingsDialog::instance()->exec();
    if (result == QDialog::Accepted && !Manager::instance()->movieFileSearcher()->isRunning()) {
        Manager::instance()->movieFileSearcher()->start();
    }
}

void MainWindow::progressStarted()
{
    ui->filesWidget->disableRefresh();
    ui->statusBar->addWidget(m_progressBar);
    m_progressBar->show();
    m_progressBar->setValue(0);
}

void MainWindow::progressProgress(int current, int max)
{
    m_progressBar->setRange(0, max);
    m_progressBar->setValue(current);
}

void MainWindow::progressFinished()
{
    if (Manager::instance()->movieModel()->movies().size() > 0)
        ui->filesWidget->hideFirstTime();
    ui->filesWidget->enableRefresh();
    if (m_progressBar->isVisible())
        ui->statusBar->removeWidget(m_progressBar);
}
