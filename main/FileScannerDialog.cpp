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
#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QFont font = ui->currentDir->font();
#ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize()-1);
#else
    font.setPointSize(font.pointSize()-2);
#endif
    ui->currentDir->setFont(font);

    m_forceReload = false;
    m_reloadType = TypeAll;

    Manager::instance()->setFileScannerDialog(this);

    connect(Manager::instance()->movieFileSearcher(), SIGNAL(progress(int,int,int)), this, SLOT(onProgress(int,int)));
    connect(Manager::instance()->concertFileSearcher(), SIGNAL(progress(int,int,int)), this, SLOT(onProgress(int,int)));
    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(progress(int,int,int)), this, SLOT(onProgress(int,int)));

    connect(Manager::instance()->movieFileSearcher(), SIGNAL(currentDir(QString)), this, SLOT(onCurrentDir(QString)));
    connect(Manager::instance()->concertFileSearcher(), SIGNAL(currentDir(QString)), this, SLOT(onCurrentDir(QString)));
    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(currentDir(QString)), this, SLOT(onCurrentDir(QString)));
    connect(Manager::instance()->movieFileSearcher(), SIGNAL(searchStarted(QString,int)), ui->status, SLOT(setText(QString)));
    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(searchStarted(QString,int)), ui->status, SLOT(setText(QString)));
    connect(Manager::instance()->concertFileSearcher(), SIGNAL(searchStarted(QString,int)), ui->status, SLOT(setText(QString)));

    connect(Manager::instance()->movieFileSearcher(), SIGNAL(moviesLoaded(int)), this, SLOT(onLoadDone(int)));
    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(tvShowsLoaded(int)), this, SLOT(onLoadDone(int)));
    connect(Manager::instance()->concertFileSearcher(), SIGNAL(concertsLoaded(int)), this, SLOT(onLoadDone(int)));
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
int FileScannerDialog::exec()
{
    Manager::instance()->movieFileSearcher()->setMovieDirectories(Settings::instance()->movieDirectories());
    Manager::instance()->tvShowFileSearcher()->setMovieDirectories(Settings::instance()->tvShowDirectories());
    Manager::instance()->concertFileSearcher()->setConcertDirectories(Settings::instance()->concertDirectories());

    ui->status->setText("");
    ui->progressBar->setValue(0);
    ui->currentDir->setText("");
    adjustSize();
    QDialog::show();

    if (m_reloadType == TypeMovies || m_reloadType == TypeAll)
        onStartMovieScanner();
    else if (m_reloadType == TypeTvShows)
        onStartTvShowScanner();
    else if (m_reloadType == TypeConcerts)
        onStartConcertScanner();
    else if (m_reloadType == TypeEpisodes)
        onStartEpisodeScanner();

    return 0;
}

void FileScannerDialog::setForceReload(bool force)
{
    m_forceReload = force;
}

void FileScannerDialog::setReloadType(ReloadType type)
{
    m_reloadType = type;
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
    ui->progressBar->setValue(0);
    Manager::instance()->movieModel()->clear();
    if (m_forceReload)
        QTimer::singleShot(0, this, SLOT(onStartMovieScannerForce()));
    else
        QTimer::singleShot(0, this, SLOT(onStartMovieScannerCache()));
}

void FileScannerDialog::onStartMovieScannerCache()
{
    Manager::instance()->movieFileSearcher()->reload(false);
}

void FileScannerDialog::onStartMovieScannerForce()
{
    Manager::instance()->movieFileSearcher()->reload(true);
}

/**
 * @brief Starts the tv show file searcher
 */
void FileScannerDialog::onStartTvShowScanner()
{
    ui->progressBar->setValue(0);
    Manager::instance()->tvShowModel()->clear();
    qApp->processEvents();
    if (m_forceReload)
        QTimer::singleShot(0, this, SLOT(onStartTvShowScannerForce()));
    else
        QTimer::singleShot(0, this, SLOT(onStartTvShowScannerCache()));
}

void FileScannerDialog::onStartTvShowScannerCache()
{
    Manager::instance()->tvShowFileSearcher()->reload(false);
}

void FileScannerDialog::onStartTvShowScannerForce()
{
    Manager::instance()->tvShowFileSearcher()->reload(true);
}

void FileScannerDialog::onStartEpisodeScanner()
{
    Manager::instance()->tvShowFileSearcher()->reloadEpisodes(m_scanDir);
}

/**
 * @brief Starts the concert file scanner
 */
void FileScannerDialog::onStartConcertScanner()
{
    ui->progressBar->setValue(0);
    Manager::instance()->concertModel()->clear();
    qApp->processEvents();
    if (m_forceReload)
        QTimer::singleShot(0, this, SLOT(onStartConcertScannerForce()));
    else
        QTimer::singleShot(0, this, SLOT(onStartConcertScannerCache()));
}

void FileScannerDialog::onStartConcertScannerCache()
{
    Manager::instance()->concertFileSearcher()->reload(false);
}

void FileScannerDialog::onStartConcertScannerForce()
{
    Manager::instance()->concertFileSearcher()->reload(true);
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

/**
 * @brief Displays the current directory
 * @param dir Current directory
 */
void FileScannerDialog::onCurrentDir(QString dir)
{
    ui->currentDir->setText(dir);
    qApp->processEvents();
}

void FileScannerDialog::onLoadDone(int msgId)
{
    if (m_reloadType != TypeAll) {
        accept();
        return;
    }

    if (msgId == Constants::MovieFileSearcherProgressMessageId) {
        onStartTvShowScanner();
    } else if (msgId == Constants::TvShowSearcherProgressMessageId) {
        onStartConcertScanner();
    } else if (msgId == Constants::ConcertFileSearcherProgressMessageId) {
        accept();
    }
}

void FileScannerDialog::setScanDir(QString dir)
{
    m_scanDir = dir;
}
