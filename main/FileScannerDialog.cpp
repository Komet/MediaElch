#include "FileScannerDialog.h"
#include "ui_FileScannerDialog.h"
#include "globals/Manager.h"

#include <QTimer>

/**
 * @brief FileScannerDialog::FileScannerDialog
 * @param parent
 */
FileScannerDialog::FileScannerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileScannerDialog)
{
    ui->setupUi(this);
#ifdef Q_WS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    connect(Manager::instance()->movieFileSearcher(), SIGNAL(progress(int,int,int)), this, SLOT(onProgress(int,int)));
    connect(Manager::instance()->concertFileSearcher(), SIGNAL(progress(int,int,int)), this, SLOT(onProgress(int,int)));
    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(progress(int,int,int)), this, SLOT(onProgress(int,int)));

    connect(Manager::instance()->movieFileSearcher(), SIGNAL(moviesLoaded(int)), this, SLOT(onStartTvShowScanner()));
    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(tvShowsLoaded(int)), this, SLOT(onStartConcertScanner()));

    connect(Manager::instance()->concertFileSearcher(), SIGNAL(concertsLoaded(int)), this, SLOT(accept()));
}

/**
 * @brief FileScannerDialog::~FileScannerDialog
 */
FileScannerDialog::~FileScannerDialog()
{
    delete ui;
}

/**
 * @brief Sets directories and starts scanning
 */
void FileScannerDialog::exec()
{
    Manager::instance()->movieFileSearcher()->setMovieDirectories(Settings::instance()->movieDirectories());
    Manager::instance()->tvShowFileSearcher()->setMovieDirectories(Settings::instance()->tvShowDirectories());
    Manager::instance()->concertFileSearcher()->setConcertDirectories(Settings::instance()->concertDirectories());

    adjustSize();
    ui->status->setText(tr("Searching for Movies..."));
    ui->progressBar->setValue(0);
    QDialog::show();
    onStartMovieScanner();
}

/**
 * @brief Rejecting should not be possible
 */
void FileScannerDialog::reject()
{
}

/**
 * @brief Starts the movie file searcher
 */
void FileScannerDialog::onStartMovieScanner()
{
    ui->status->setText(tr("Searching for Movies..."));
    ui->progressBar->setValue(0);
    Manager::instance()->movieModel()->clear();
    QTimer::singleShot(0, Manager::instance()->movieFileSearcher(), SLOT(run()));
}

/**
 * @brief Starts the tv show file searcher
 */
void FileScannerDialog::onStartTvShowScanner()
{
    ui->status->setText(tr("Searching for TV Shows..."));
    ui->progressBar->setValue(0);
    Manager::instance()->tvShowModel()->clear();
    QTimer::singleShot(0, Manager::instance()->tvShowFileSearcher(), SLOT(run()));
}

/**
 * @brief Starts the concert file scanner
 */
void FileScannerDialog::onStartConcertScanner()
{
    ui->status->setText(tr("Searching for Concerts..."));
    ui->progressBar->setValue(0);
    Manager::instance()->concertModel()->clear();
    QTimer::singleShot(0, Manager::instance()->concertFileSearcher(), SLOT(run()));
}

/**
 * @brief Updates the progress bar
 * @param current Current value
 * @param max Maximum value
 */
void FileScannerDialog::onProgress(int current, int max)
{
    ui->progressBar->setRange(0, max);
    ui->progressBar->setValue(current);
}
