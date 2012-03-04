#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QDir>

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
    ui->mainToolBar->setVisible(false);
    m_progressBar = new QProgressBar(ui->statusBar);
    m_progressBar->hide();
    m_aboutDialog = new AboutDialog(this);

    Manager::instance();

    if (SettingsDialog::instance()->mainWindowSize().isValid()) {
        resize(SettingsDialog::instance()->mainWindowSize());
    }
    if (!SettingsDialog::instance()->mainWindowPosition().isNull()) {
        move(SettingsDialog::instance()->mainWindowPosition());
    }

    Manager::instance()->movieFileSearcher()->setMovieDirectories(SettingsDialog::instance()->movieDirectories());

    connect(ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(execSettingsDialog()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(execAboutDialog()));
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

    if (SettingsDialog::instance()->firstTime()) {
        ui->filesWidget->showFirstTime();
        ui->movieWidget->showFirstTime();
    }

    Manager::instance()->movieFileSearcher()->start();
    MovieSearch::instance(ui->centralWidget);
    MovieImageDialog::instance(ui->centralWidget);
    QuestionDialog::instance(ui->centralWidget);
}

MainWindow::~MainWindow()
{
    SettingsDialog::instance()->setMainWindowSize(size());
    SettingsDialog::instance()->setMainWindowPosition(pos());
    delete SettingsDialog::instance();
    delete ui;
}

void MainWindow::execAboutDialog()
{
    m_aboutDialog->exec();
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
