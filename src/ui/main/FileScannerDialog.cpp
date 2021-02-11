#include "FileScannerDialog.h"
#include "globals/Manager.h"
#include "ui_FileScannerDialog.h"

#include <QTimer>

#include "data/ImageCache.h"

FileScannerDialog::FileScannerDialog(QWidget* parent) : QDialog(parent), ui(new Ui::FileScannerDialog)
{
    using namespace mediaelch;

    ui->setupUi(this);
#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QFont font = ui->currentDir->font();
#ifdef Q_OS_WIN
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->currentDir->setFont(font);

    auto* manager = Manager::instance();
    manager->setFileScannerDialog(this);

    // clang-format off
    connect(manager->movieFileSearcher(),   &MovieFileSearcher::progress,   this, &FileScannerDialog::onProgress);
    connect(manager->concertFileSearcher(), &ConcertFileSearcher::progress, this, &FileScannerDialog::onProgress);
    connect(manager->tvShowFileSearcher(),  &TvShowFileSearcher::progress,  this, &FileScannerDialog::onProgress);
    connect(manager->musicFileSearcher(),   &MusicFileSearcher::progress,   this, &FileScannerDialog::onProgress);

    connect(manager->movieFileSearcher(),   &MovieFileSearcher::currentDir,   this, &FileScannerDialog::onCurrentDir);
    connect(manager->concertFileSearcher(), &ConcertFileSearcher::currentDir, this, &FileScannerDialog::onCurrentDir);
    connect(manager->tvShowFileSearcher(),  &TvShowFileSearcher::currentDir,  this, &FileScannerDialog::onCurrentDir);
    connect(manager->musicFileSearcher(),   &MusicFileSearcher::currentDir,   this, &FileScannerDialog::onCurrentDir);

    connect(manager->movieFileSearcher(),   &MovieFileSearcher::searchStarted,   ui->status, &QLabel::setText);
    connect(manager->tvShowFileSearcher(),  &TvShowFileSearcher::searchStarted,  ui->status, &QLabel::setText);
    connect(manager->concertFileSearcher(), &ConcertFileSearcher::searchStarted, ui->status, &QLabel::setText);
    connect(manager->musicFileSearcher(),   &MusicFileSearcher::searchStarted,   ui->status, &QLabel::setText);
    // clang-format on

    connect(manager->movieFileSearcher(), &MovieFileSearcher::moviesLoaded, [this]() {
        if (m_reloadType != ReloadType::All) {
            accept();
        } else {
            onStartTvShowScanner();
        }
    });
    connect(manager->tvShowFileSearcher(), &TvShowFileSearcher::tvShowsLoaded, [this]() {
        if (m_reloadType != ReloadType::All) {
            accept();
        } else {
            onStartConcertScanner();
        }
    });
    connect(manager->concertFileSearcher(), &ConcertFileSearcher::concertsLoaded, [this]() {
        if (m_reloadType != ReloadType::All) {
            accept();
        } else {
            onStartMusicScanner();
        }
    });
    connect(manager->musicFileSearcher(), &MusicFileSearcher::musicLoaded, this, &FileScannerDialog::accept);
}

/**
 * \brief FileScannerDialog::~FileScannerDialog
 */
FileScannerDialog::~FileScannerDialog()
{
    delete ui;
}

/// Sets directories and starts scanning
int FileScannerDialog::exec()
{
    auto* manager = Manager::instance();
    const auto& dirSettings = Settings::instance()->directorySettings();

    if (m_reloadType == ReloadType::All || m_reloadType == ReloadType::Movies) {
        manager->movieFileSearcher()->setMovieDirectories(dirSettings.movieDirectories());
    }
    if (m_reloadType == ReloadType::All || m_reloadType == ReloadType::TvShows) {
        manager->tvShowFileSearcher()->setTvShowDirectories(dirSettings.tvShowDirectories());
    }
    if (m_reloadType == ReloadType::All || m_reloadType == ReloadType::Concerts) {
        manager->concertFileSearcher()->setConcertDirectories(dirSettings.concertDirectories());
    }
    if (m_reloadType == ReloadType::All || m_reloadType == ReloadType::Music) {
        manager->musicFileSearcher()->setMusicDirectories(dirSettings.musicDirectories());
    }

    ui->status->setText("");
    ui->progressBar->setValue(0);
    ui->currentDir->setText("");
    QDialog::show();
    adjustSize();

    if (m_forceReload) {
        ImageCache::instance()->clearCache();
    }

    switch (m_reloadType) {
    case ReloadType::All: // start with movies
    case ReloadType::Movies: onStartMovieScanner(); break;
    case ReloadType::TvShows: onStartTvShowScanner(); break;
    case ReloadType::Concerts: onStartConcertScanner(); break;
    case ReloadType::Episodes: onStartEpisodeScanner(); break;
    case ReloadType::Music: onStartMusicScanner(); break;
    }

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
 * \brief Rejecting should not be possible
 */
void FileScannerDialog::reject()
{
    if (m_reloadType == ReloadType::Movies || m_reloadType == ReloadType::All) {
        Manager::instance()->movieFileSearcher()->abort();
        Manager::instance()->movieModel()->clear();
    }
    if (m_reloadType == ReloadType::TvShows || m_reloadType == ReloadType::All) {
        Manager::instance()->tvShowFileSearcher()->abort();
        Manager::instance()->tvShowModel()->clear();
        Manager::instance()->tvShowFilesWidget()->renewModel();
    }
    if (m_reloadType == ReloadType::Concerts || m_reloadType == ReloadType::All) {
        Manager::instance()->concertFileSearcher()->abort();
        Manager::instance()->concertModel()->clear();
    }
    if (m_reloadType == ReloadType::Music || m_reloadType == ReloadType::All) {
        Manager::instance()->musicFileSearcher()->abort();
        Manager::instance()->musicModel()->clear();
    }

    QDialog::reject();
}

/**
 * \brief Starts the movie file searcher
 */
void FileScannerDialog::onStartMovieScanner()
{
    ui->progressBar->setValue(0);
    Manager::instance()->movieModel()->clear();
    QApplication::processEvents();
    if (m_forceReload) {
        QTimer::singleShot(0, this, &FileScannerDialog::onStartMovieScannerForce);
    } else {
        QTimer::singleShot(0, this, &FileScannerDialog::onStartMovieScannerCache);
    }
}

void FileScannerDialog::onStartMovieScannerCache()
{
    Manager::instance()->movieFileSearcher()->reload(false);
}

void FileScannerDialog::onStartMovieScannerForce()
{
    Manager::instance()->movieFileSearcher()->reload(true);
}

/// Starts the TV show file searcher
void FileScannerDialog::onStartTvShowScanner()
{
    ui->progressBar->setValue(0);
    Manager::instance()->tvShowModel()->clear();
    QApplication::processEvents();
    if (m_forceReload) {
        QTimer::singleShot(0, this, &FileScannerDialog::onStartTvShowScannerForce);
    } else {
        QTimer::singleShot(0, this, &FileScannerDialog::onStartTvShowScannerCache);
    }
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
 * \brief Starts the concert file scanner
 */
void FileScannerDialog::onStartConcertScanner()
{
    ui->progressBar->setValue(0);
    Manager::instance()->concertModel()->clear();
    QApplication::processEvents();
    if (m_forceReload) {
        QTimer::singleShot(0, this, &FileScannerDialog::onStartConcertScannerForce);
    } else {
        QTimer::singleShot(0, this, &FileScannerDialog::onStartConcertScannerCache);
    }
}

void FileScannerDialog::onStartConcertScannerCache()
{
    Manager::instance()->concertFileSearcher()->reload(false);
}

void FileScannerDialog::onStartConcertScannerForce()
{
    Manager::instance()->concertFileSearcher()->reload(true);
}

void FileScannerDialog::onStartMusicScanner()
{
    ui->progressBar->setValue(0);
    Manager::instance()->musicModel()->clear();
    QApplication::processEvents();
    if (m_forceReload) {
        QTimer::singleShot(0, this, &FileScannerDialog::onStartMusicScannerForce);
    } else {
        QTimer::singleShot(0, this, &FileScannerDialog::onStartMusicScannerCache);
    }
}

void FileScannerDialog::onStartMusicScannerCache()
{
    Manager::instance()->musicFileSearcher()->reload(false);
}

void FileScannerDialog::onStartMusicScannerForce()
{
    Manager::instance()->musicFileSearcher()->reload(true);
}

/**
 * \brief Updates the progress bar
 * \param current Current value
 * \param max Maximum value
 */
void FileScannerDialog::onProgress(int current, int max)
{
    ui->progressBar->setRange(0, max);
    ui->progressBar->setValue(current);
}

/**
 * \brief Displays the current directory
 * \param dir Current directory
 */
void FileScannerDialog::onCurrentDir(QString dir)
{
    ui->currentDir->setText(dir);
    QApplication::processEvents();
}

void FileScannerDialog::setScanDir(const mediaelch::DirectoryPath& dir)
{
    m_scanDir = dir;
}
