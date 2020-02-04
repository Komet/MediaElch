#include "ImportDialog.h"
#include "ui_ImportDialog.h"

#include <QMessageBox>
#include <QMovie>

#include "data/ImageCache.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/NameFormatter.h"
#include "renamer/RenamerDialog.h"
#include "scrapers/movie/CustomMovieScraper.h"
#include "settings/Settings.h"
#include "tv_shows/TvShowFileSearcher.h"
#include "tv_shows/model/SeasonModelItem.h"
#include "tv_shows/model/TvShowModelItem.h"
#include "ui/notifications/Notificator.h"

ImportDialog::ImportDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ImportDialog)
{
    ui->setupUi(this);

    ui->stackedWidget->setAnimation(QEasingCurve::Linear);
    ui->stackedWidget->setSpeed(200);
    ui->badgeSuccess->setActive(true);
    ui->badgeSuccess->setBadgeType(Badge::Type::LabelSuccess);
    ui->badgeSuccess->setShowActiveMark(true);

    QMovie* loadingMovie = new QMovie(":/img/spinner.gif", QByteArray(), this);
    loadingMovie->start();
    ui->loading->setMovie(loadingMovie);

    m_timer.setInterval(500);

    m_posterDownloadManager = new DownloadManager(this);
    connect(
        m_posterDownloadManager, &DownloadManager::sigDownloadFinished, this, &ImportDialog::onEpisodeDownloadFinished);

    connect(ui->movieSearchWidget, &MovieSearchWidget::sigResultClicked, this, &ImportDialog::onMovieChosen);
    connect(ui->concertSearchWidget, &ConcertSearchWidget::sigResultClicked, this, &ImportDialog::onConcertChosen);
    connect(ui->tvShowSearchEpisode, &TvShowSearchEpisode::sigResultClicked, this, &ImportDialog::onTvShowChosen);
    connect(ui->btnImport, &QAbstractButton::clicked, this, &ImportDialog::onImport);
    connect(&m_timer, &QTimer::timeout, this, &ImportDialog::onFileWatcherTimeout);
}

ImportDialog::~ImportDialog()
{
    delete ui;
}

void ImportDialog::reject()
{
    if (!m_filesToMove.isEmpty()) {
        return;
    }

    if (m_movie != nullptr) {
        m_movie->controller()->abortDownloads();
        m_movie->deleteLater();
    }

    if (m_concert != nullptr) {
        m_concert->controller()->abortDownloads();
        m_concert->deleteLater();
    }

    if (m_episode != nullptr) {
        m_episode->deleteLater();
    }

    Settings::instance()->setImportDialogSize(size());
    Settings::instance()->setImportDialogPosition(pos());
    Settings::instance()->setKeepDownloadSource(ui->chkKeepSourceFiles->isChecked());
    Settings::instance()->saveSettings();
    storeDefaults();
    QDialog::reject();
}

void ImportDialog::accept()
{
    Settings::instance()->setImportDialogSize(size());
    Settings::instance()->setImportDialogPosition(pos());
    Settings::instance()->setKeepDownloadSource(ui->chkKeepSourceFiles->isChecked());
    Settings::instance()->saveSettings();
    storeDefaults();
    QDialog::accept();
}

int ImportDialog::exec()
{
    if (Settings::instance()->importDialogSize().isValid() && !Settings::instance()->importDialogPosition().isNull()) {
        move(Settings::instance()->importDialogPosition());
        resize(Settings::instance()->importDialogSize());
    }

    ui->btnImport->setEnabled(false);
    ui->btnReject->setEnabled(true);
    ui->btnReject->setVisible(true);
    ui->btnAccept->setVisible(false);
    ui->progressBar->setValue(0);
    ui->labelLoading->setVisible(true);

    ui->chkKeepSourceFiles->setChecked(Settings::instance()->keepDownloadSource());

    return QDialog::exec();
}

int ImportDialog::execMovie(QString searchString)
{
    ImdbId id;
    QRegExp rx("tt(\\d+)");
    if (rx.indexIn(searchString) != -1) {
        id = ImdbId(rx.cap(0));
        searchString = searchString.replace(rx.cap(0), "").trimmed();
    }

    m_type = "movie";
    m_filesToMove.clear();
    ui->stackedWidget->setCurrentIndex(0);
    ui->movieSearchWidget->search(NameFormatter::instance()->formatName(searchString), id, TmdbId::NoId);

    ui->placeholders->setType(Renamer::RenameType::Movies);
    ui->chkSeasonDirectories->setVisible(false);
    ui->labelUseSeasonDirectories->setVisible(false);
    ui->seasonNaming->setVisible(false);
    ui->labelSeasonNaming->setVisible(false);
    ui->labelDirectoryNaming->setVisible(m_separateFolders);
    ui->directoryNaming->setVisible(m_separateFolders);

    setDefaults(Renamer::RenameType::Movies);

    return exec();
}

int ImportDialog::execTvShow(QString searchString, TvShow* tvShow)
{
    Q_UNUSED(searchString);

    m_type = "tvshow";
    m_show = tvShow;

    // get path
    QString path;
    int index = -1;
    for (int i = 0, n = Settings::instance()->directorySettings().tvShowDirectories().count(); i < n; ++i) {
        if (tvShow->dir().startsWith(Settings::instance()->directorySettings().tvShowDirectories().at(i).path.path())) {
            if (index == -1) {
                index = i;
            } else if (Settings::instance()->directorySettings().tvShowDirectories().at(index).path.path().length()
                       < Settings::instance()->directorySettings().tvShowDirectories().at(i).path.path().length()) {
                index = i;
            }
        }
    }
    if (index != -1) {
        path = Settings::instance()->directorySettings().tvShowDirectories().at(index).path.path();
    }
    m_importDir = path;

    m_filesToMove.clear();

    ui->placeholders->setType(Renamer::RenameType::TvShows);
    ui->chkSeasonDirectories->setVisible(true);
    ui->labelUseSeasonDirectories->setVisible(true);
    ui->seasonNaming->setVisible(true);
    ui->labelSeasonNaming->setVisible(true);
    ui->labelDirectoryNaming->setVisible(false);
    ui->directoryNaming->setVisible(false);
    ui->stackedWidget->setCurrentIndex(3);
    ui->tvShowSearchEpisode->search(tvShow->name(), tvShow->tvdbId());

    setDefaults(Renamer::RenameType::TvShows);

    return exec();
}

int ImportDialog::execConcert(QString searchString)
{
    m_type = "concert";
    m_filesToMove.clear();
    ui->stackedWidget->setCurrentIndex(2);
    ui->concertSearchWidget->search(NameFormatter::instance()->formatName(searchString));

    ui->placeholders->setType(Renamer::RenameType::Concerts);
    ui->chkSeasonDirectories->setVisible(false);
    ui->labelUseSeasonDirectories->setVisible(false);
    ui->seasonNaming->setVisible(false);
    ui->labelSeasonNaming->setVisible(false);
    ui->labelDirectoryNaming->setVisible(m_separateFolders);
    ui->directoryNaming->setVisible(m_separateFolders);

    setDefaults(Renamer::RenameType::Concerts);

    return exec();
}

void ImportDialog::setDefaults(Renamer::RenameType renameType)
{
    QString fileName;
    QString fileNameMulti;
    QString directoryName;
    QString seasonName;
    bool renameFiles;
    bool renameFolders;
    bool useSeasonDirectories;
    Settings::instance()->renamePatterns(renameType, fileName, fileNameMulti, directoryName, seasonName);
    Settings::instance()->renamings(renameType, renameFiles, renameFolders, useSeasonDirectories);

    ui->fileNaming->setText(fileName);
    ui->directoryNaming->setText(directoryName);
    ui->seasonNaming->setText(seasonName);
    ui->chkSeasonDirectories->setChecked(useSeasonDirectories);
}

void ImportDialog::storeDefaults()
{
    Renamer::RenameType renameType;
    if (m_type == "movie") {
        renameType = Renamer::RenameType::Movies;
    } else if (m_type == "tvshow") {
        renameType = Renamer::RenameType::TvShows;
    } else if (m_type == "concert") {
        renameType = Renamer::RenameType::Concerts;
    } else {
        return;
    }

    QString fileName;
    QString fileNameMulti;
    QString directoryName;
    QString seasonName;
    bool renameFiles;
    bool renameFolders;
    bool useSeasonDirectories;
    Settings::instance()->renamePatterns(renameType, fileName, fileNameMulti, directoryName, seasonName);
    Settings::instance()->renamings(renameType, renameFiles, renameFolders, useSeasonDirectories);

    fileName = ui->fileNaming->text();
    directoryName = ui->directoryNaming->text();
    seasonName = ui->seasonNaming->text();
    useSeasonDirectories = ui->chkSeasonDirectories->isChecked();

    Settings::instance()->setRenamePatterns(renameType, fileName, fileNameMulti, directoryName, seasonName);
    Settings::instance()->setRenamings(renameType, renameFiles, renameFolders, useSeasonDirectories);
}

void ImportDialog::onMovieChosen()
{
    QMap<MovieScraperInterface*, QString> ids;
    QVector<MovieScraperInfos> infosToLoad;
    if (ui->movieSearchWidget->scraperId() == CustomMovieScraper::scraperIdentifier) {
        ids = ui->movieSearchWidget->customScraperIds();
        infosToLoad = Settings::instance()->scraperInfos<MovieScraperInfos>(CustomMovieScraper::scraperIdentifier);
    } else {
        ids.insert(0, ui->movieSearchWidget->scraperMovieId());
        infosToLoad = ui->movieSearchWidget->infosToLoad();
    }

    if (m_movie != nullptr) {
        m_movie->deleteLater();
    }

    ui->stackedWidget->slideInIdx(1);
    ui->loading->setVisible(true);
    ui->labelLoading->setText(tr("Loading movie information..."));
    ui->badgeSuccess->setVisible(false);
    ui->formLayout->setEnabled(false);

    m_movie = new Movie(files());
    m_movie->controller()->loadData(ids, Manager::instance()->scraper(ui->movieSearchWidget->scraperId()), infosToLoad);
    connect(m_movie->controller(), SIGNAL(sigLoadDone(Movie*)), this, SLOT(onLoadDone(Movie*)), Qt::UniqueConnection);
}

void ImportDialog::onConcertChosen()
{
    if (m_concert != nullptr) {
        m_concert->deleteLater();
    }

    ui->stackedWidget->slideInIdx(1, SlidingStackedWidget::RIGHT2LEFT);
    ui->loading->setVisible(true);
    ui->labelLoading->setText(tr("Loading concert information..."));
    ui->badgeSuccess->setVisible(false);
    ui->formLayout->setEnabled(false);

    m_concert = new Concert(files());
    m_concert->controller()->loadData(ui->concertSearchWidget->scraperId(),
        Manager::instance()->concertScrapers().at(ui->concertSearchWidget->scraperNo()),
        ui->concertSearchWidget->infosToLoad());
    connect(
        m_concert->controller(), SIGNAL(sigLoadDone(Concert*)), this, SLOT(onLoadDone(Concert*)), Qt::UniqueConnection);
}

void ImportDialog::onTvShowChosen()
{
    if (m_episode != nullptr) {
        m_episode->deleteLater();
    }

    ui->stackedWidget->slideInIdx(1, SlidingStackedWidget::RIGHT2LEFT);
    ui->loading->setVisible(true);
    ui->labelLoading->setText(tr("Loading episode information..."));
    ui->badgeSuccess->setVisible(false);
    ui->formLayout->setEnabled(false);

    m_episode = new TvShowEpisode(files(), m_show);
    m_episode->setSeason(TvShowFileSearcher::getSeasonNumber(files()));
    QVector<EpisodeNumber> episodes = TvShowFileSearcher::getEpisodeNumbers(files());
    if (!episodes.isEmpty()) {
        m_episode->setEpisode(episodes.first());
    }
    m_episode->loadData(ui->tvShowSearchEpisode->scraperId(),
        Manager::instance()->tvScrapers().at(0),
        ui->tvShowSearchEpisode->infosToLoad());
    connect(m_episode.data(), &TvShowEpisode::sigLoaded, this, &ImportDialog::onEpisodeLoadDone, Qt::UniqueConnection);
}

void ImportDialog::setFiles(QStringList files)
{
    m_files = files;
    m_files.sort();
}

QStringList ImportDialog::files()
{
    return m_files;
}

void ImportDialog::setExtraFiles(QStringList extraFiles)
{
    m_extraFiles = extraFiles;
    m_extraFiles.sort();
}

QStringList ImportDialog::extraFiles()
{
    return m_extraFiles;
}

void ImportDialog::setImportDir(QString dir)
{
    for (const SettingsDir& settingsDir : QVector<SettingsDir>()
                                              << Settings::instance()->directorySettings().movieDirectories()
                                              << Settings::instance()->directorySettings().concertDirectories()) {
        if (settingsDir.path == dir) {
            m_separateFolders = settingsDir.separateFolders;
            break;
        }
    }
    m_importDir = dir;
}

QString ImportDialog::importDir()
{
    return m_importDir;
}

void ImportDialog::onLoadDone(Movie* movie)
{
    if (movie != m_movie) {
        return;
    }

    ui->loading->setVisible(false);
    ui->badgeSuccess->setText(tr("Movie information was loaded"));
    ui->labelLoading->setVisible(false);
    ui->badgeSuccess->setVisible(true);
    ui->btnImport->setEnabled(true);
    ui->formLayout->setEnabled(true);
}

void ImportDialog::onLoadDone(Concert* concert)
{
    if (concert != m_concert) {
        return;
    }

    ui->loading->setVisible(false);
    ui->badgeSuccess->setText(tr("Concert information was loaded"));
    ui->labelLoading->setVisible(false);
    ui->badgeSuccess->setVisible(true);
    ui->btnImport->setEnabled(true);
    ui->formLayout->setEnabled(true);
}

void ImportDialog::onEpisodeLoadDone()
{
    if (!m_episode->thumbnail().isEmpty()) {
        DownloadManagerElement d;
        d.imageType = ImageType::TvShowEpisodeThumb;
        d.url = m_episode->thumbnail();
        d.episode = m_episode;
        d.directDownload = true;
        m_posterDownloadManager->addDownload(d);
    } else {
        ui->loading->setVisible(false);
        ui->badgeSuccess->setText(tr("Episode information was loaded"));
        ui->badgeSuccess->setVisible(true);
        ui->labelLoading->setVisible(false);
        ui->btnImport->setEnabled(true);
        ui->formLayout->setEnabled(true);
    }
}

void ImportDialog::onEpisodeDownloadFinished(DownloadManagerElement elem)
{
    qDebug() << "got image";
    if (m_episode == nullptr) {
        return;
    }

    ImageCache::instance()->invalidateImages(
        Manager::instance()->mediaCenterInterface()->imageFileName(elem.episode, ImageType::TvShowEpisodeThumb));
    elem.episode->setThumbnailImage(elem.data);
    ui->loading->setVisible(false);
    ui->badgeSuccess->setText(tr("Episode information was loaded"));
    ui->badgeSuccess->setVisible(true);
    ui->labelLoading->setVisible(false);
    ui->btnImport->setEnabled(true);
    ui->formLayout->setEnabled(true);
}

void ImportDialog::onImport()
{
    if (ui->fileNaming->text().isEmpty()
        || ((m_type == "movie" || m_type == "concert") && m_separateFolders && ui->directoryNaming->text().isEmpty())
        || (m_type == "tvshow" && ui->chkSeasonDirectories->isChecked() && ui->seasonNaming->text().isEmpty())) {
        QMessageBox::warning(this, tr("Renaming not possible"), tr("Please enter all naming patterns"));
        return;
    }

    m_newFiles.clear();
    m_filesToMove.clear();

    if (m_type == "movie") {
        QDir dir(importDir());
        const auto videoDetails = m_movie->streamDetails()->videoDetails();
        if (m_separateFolders) {
            QString newFolderName = ui->directoryNaming->text();
            Renamer::replace(newFolderName, "title", m_movie->name());
            Renamer::replace(newFolderName, "originalTitle", m_movie->originalName());
            Renamer::replace(newFolderName, "sortTitle", m_movie->sortTitle());
            Renamer::replace(newFolderName, "year", m_movie->released().toString("yyyy"));
            Renamer::replace(newFolderName,
                "resolution",
                helper::matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::ScanType)));
            Renamer::replaceCondition(newFolderName, "bluray", m_movie->discType() == DiscType::BluRay);
            Renamer::replaceCondition(newFolderName, "dvd", m_movie->discType() == DiscType::Dvd);
            Renamer::replaceCondition(
                newFolderName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
            Renamer::replaceCondition(newFolderName, "movieset", m_movie->set().name);
            helper::sanitizeFileName(newFolderName);
            if (!dir.mkdir(newFolderName)) {
                QMessageBox::warning(this,
                    tr("Creating destination directory failed"),
                    tr("The destination directory %1 could not be created")
                        .arg(dir.absolutePath() + QDir::separator() + newFolderName));
            }
            dir.cd(newFolderName);
        }
        for (const QString& file : QStringList() << files() << extraFiles()) {
            QFileInfo fi(file);
            QString newFileName = ui->fileNaming->text();
            Renamer::replace(newFileName, "title", m_movie->name());
            Renamer::replace(newFileName, "originalTitle", m_movie->originalName());
            Renamer::replace(newFileName, "sortTitle", m_movie->sortTitle());
            Renamer::replace(newFileName, "year", m_movie->released().toString("yyyy"));
            Renamer::replace(newFileName, "extension", fi.suffix());
            Renamer::replace(newFileName,
                "resolution",
                helper::matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::ScanType)));
            Renamer::replaceCondition(newFileName, "imdbId", m_movie->imdbId().toString());
            Renamer::replaceCondition(newFileName, "movieset", m_movie->set().name);
            Renamer::replaceCondition(
                newFileName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
            helper::sanitizeFileName(newFileName);
            m_filesToMove.insert(file, dir.absolutePath() + QDir::separator() + newFileName);
            if (files().contains(file)) {
                m_newFiles.append(dir.absolutePath() + QDir::separator() + newFileName);
            }
        }
        ui->labelLoading->setText(tr("Importing movie..."));

    } else if (m_type == "tvshow") {
        const auto videoDetails = m_episode->streamDetails()->videoDetails();
        QDir dir(m_show->dir());
        if (ui->chkSeasonDirectories->isChecked()) {
            QString newFolderName = ui->seasonNaming->text();
            Renamer::replace(newFolderName, "season", m_episode->seasonString());
            helper::sanitizeFileName(newFolderName);
            dir.mkdir(newFolderName);
            dir.cd(newFolderName);
        }

        for (const QString& file : QStringList() << files() << extraFiles()) {
            QFileInfo fi(file);
            QString newFileName = ui->fileNaming->text();

            Renamer::replace(newFileName, "title", m_episode->name());
            Renamer::replace(newFileName, "showTitle", m_episode->showTitle());
            Renamer::replace(newFileName, "year", m_episode->firstAired().toString("yyyy"));
            Renamer::replace(newFileName, "extension", fi.suffix());
            Renamer::replace(newFileName, "episode", m_episode->episodeString());
            Renamer::replace(newFileName, "season", m_episode->seasonString());
            Renamer::replace(newFileName,
                "resolution",
                helper::matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::ScanType)));
            Renamer::replaceCondition(newFileName,
                "3D",
                m_episode->streamDetails()->videoDetails().value(StreamDetails::VideoDetails::StereoMode) != "");
            helper::sanitizeFileName(newFileName);
            m_filesToMove.insert(file, dir.absolutePath() + QDir::separator() + newFileName);
            if (files().contains(file)) {
                m_newFiles.append(dir.absolutePath() + QDir::separator() + newFileName);
            }
        }

        ui->labelLoading->setText(tr("Importing episode..."));

    } else if (m_type == "concert") {
        const auto videoDetails = m_concert->streamDetails()->videoDetails();
        QDir dir(importDir());
        if (m_separateFolders) {
            QString newFolderName = ui->directoryNaming->text();
            Renamer::replace(newFolderName, "title", m_concert->name());
            Renamer::replace(newFolderName, "artist", m_concert->artist());
            Renamer::replace(newFolderName, "album", m_concert->album());
            Renamer::replace(newFolderName, "year", m_concert->released().toString("yyyy"));
            Renamer::replace(newFolderName,
                "resolution",
                helper::matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::ScanType)));
            Renamer::replaceCondition(newFolderName, "bluray", m_concert->discType() == DiscType::BluRay);
            Renamer::replaceCondition(newFolderName, "dvd", m_concert->discType() == DiscType::Dvd);
            Renamer::replaceCondition(
                newFolderName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
            helper::sanitizeFileName(newFolderName);
            if (!dir.mkdir(newFolderName)) {
                QMessageBox::warning(this,
                    tr("Creating destination directory failed"),
                    tr("The destination directory %1 could not be created")
                        .arg(dir.absolutePath() + QDir::separator() + newFolderName));
            }
            dir.cd(newFolderName);
        }
        for (const QString& file : QStringList() << files() << extraFiles()) {
            QFileInfo fi(file);
            QString newFileName = ui->fileNaming->text();
            Renamer::replace(newFileName, "title", m_concert->name());
            Renamer::replace(newFileName, "artist", m_concert->artist());
            Renamer::replace(newFileName, "album", m_concert->album());
            Renamer::replace(newFileName, "year", m_concert->released().toString("yyyy"));
            Renamer::replace(newFileName, "extension", fi.suffix());
            Renamer::replace(newFileName,
                "resolution",
                helper::matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::ScanType)));
            Renamer::replaceCondition(
                newFileName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
            helper::sanitizeFileName(newFileName);
            m_filesToMove.insert(file, dir.absolutePath() + QDir::separator() + newFileName);
            if (files().contains(file)) {
                m_newFiles.append(dir.absolutePath() + QDir::separator() + newFileName);
            }
        }
        ui->labelLoading->setText(tr("Importing concert..."));
    }

    ui->loading->setVisible(true);
    ui->btnImport->setEnabled(false);
    ui->btnReject->setEnabled(false);
    m_worker = new FileWorker();
    m_worker->setFiles(m_filesToMove);
    m_workerThread = new QThread(this);
    if (ui->chkKeepSourceFiles->isChecked()) {
        connect(m_workerThread.data(), &QThread::started, m_worker.data(), &FileWorker::copyFiles);
    } else {
        connect(m_workerThread.data(), &QThread::started, m_worker.data(), &FileWorker::moveFiles);
    }
    connect(m_workerThread.data(), &QThread::finished, m_worker.data(), &QObject::deleteLater);
    connect(m_workerThread.data(), &QThread::finished, m_workerThread.data(), &QObject::deleteLater);
    connect(m_worker.data(), &FileWorker::sigFinished, m_workerThread.data(), &QThread::quit);
    connect(m_worker.data(), &FileWorker::sigFinished, this, &ImportDialog::onMovingFilesFinished);
    m_worker->moveToThread(m_workerThread);
    m_workerThread->start();
    m_timer.start();
}

void ImportDialog::onFileWatcherTimeout()
{
    qint64 sourceSize = 0;
    qint64 destinationSize = 0;
    QMapIterator<QString, QString> it(m_filesToMove);
    while (it.hasNext()) {
        it.next();
        QFileInfo sourceFi(it.key());
        sourceSize += sourceFi.size();

        QFileInfo destFi(it.value());
        if (!destFi.exists()) {
            continue;
        }
        destinationSize += destFi.size();
    }

    if (sourceSize == 0) {
        return;
    }

    ui->progressBar->setValue(qRound(static_cast<float>(destinationSize) * 100.0f / static_cast<float>(sourceSize)));
}

void ImportDialog::onMovingFilesFinished()
{
    ui->progressBar->setValue(100);
    m_timer.stop();
    if (m_type == "movie") {
        m_movie->setFiles(m_newFiles);
        m_movie->setInSeparateFolder(m_separateFolders);
        if (!m_newFiles.isEmpty()) {
            m_movie->setFileLastModified(QFileInfo(m_newFiles.first()).lastModified());
        }
        m_movie->controller()->loadStreamDetailsFromFile();
        m_movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
        m_movie->controller()->loadData(Manager::instance()->mediaCenterInterface());
        Manager::instance()->database()->add(m_movie, importDir());
        Manager::instance()->database()->commit();
        Manager::instance()->movieModel()->addMovie(m_movie);
        m_movie = nullptr;

    } else if (m_type == "tvshow") {
        if (m_show->showMissingEpisodes()) {
            m_show->clearMissingEpisodes();
        }

        m_episode->setFiles(m_newFiles);
        m_episode->loadStreamDetailsFromFile();
        m_show->addEpisode(m_episode);
        m_episode->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
        m_episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
        Manager::instance()->database()->add(m_episode, importDir(), m_show->databaseId());

        if (m_show->showMissingEpisodes()) {
            m_show->fillMissingEpisodes();

        } else {
            // previously it was checked for newSeason == true
            // We now always refresh the show unless there are missing episode
            // in which case the fillMissingEpisodes() updates the model.
            Manager::instance()->tvShowModel()->updateShow(m_show);
            TvShowFilesWidget::instance().renewModel(true);
        }

        m_episode = nullptr;
        m_show = nullptr;

    } else if (m_type == "concert") {
        m_concert->setFiles(m_newFiles);
        m_concert->controller()->loadStreamDetailsFromFile();
        m_concert->controller()->saveData(Manager::instance()->mediaCenterInterface());
        m_concert->controller()->loadData(Manager::instance()->mediaCenterInterface());
        Manager::instance()->database()->add(m_concert, importDir());
        Manager::instance()->database()->commit();
        Manager::instance()->concertModel()->addConcert(m_concert);
        m_concert = nullptr;
    }

    Notificator::instance()->notify(
        Notificator::Information, tr("Import finished"), tr("Import of %n files has finished", "", files().count()));

    ui->loading->setVisible(false);
    ui->badgeSuccess->setText(tr("Import has finished"));

    m_filesToMove.clear();

    ui->btnReject->setVisible(false);
    ui->btnAccept->setVisible(true);
}
