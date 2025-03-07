#include "ImportDialog.h"
#include "ui_ImportDialog.h"

#include "database/ConcertPersistence.h"
#include "database/MoviePersistence.h"
#include "database/TvShowPersistence.h"
#include "file_search/TvShowFileSearcher.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "log/Log.h"
#include "media/ImageCache.h"
#include "media/NameFormatter.h"
#include "renamer/ConcertRenamer.h"
#include "renamer/EpisodeRenamer.h"
#include "renamer/MovieRenamer.h"
#include "scrapers/movie/custom/CustomMovieScraper.h"
#include "settings/Settings.h"
#include "ui/notifications/Notificator.h"

#include <QMessageBox>
#include <QMovie>
#include <QRegularExpression>

ImportDialog::ImportDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ImportDialog)
{
    ui->setupUi(this);

    ui->stackedWidget->setAnimation(QEasingCurve::Linear);
    ui->stackedWidget->setSpeed(200);
    ui->badgeSuccess->setActive(true);
    ui->badgeSuccess->setBadgeType(Badge::Type::LabelSuccess);
    ui->badgeSuccess->setShowActiveMark(true);

    auto* loadingMovie = new QMovie(":/img/spinner.gif", QByteArray(), this);
    loadingMovie->start();
    ui->loading->setMovie(loadingMovie);

    m_timer.setInterval(500);

    m_posterDownloadManager = new DownloadManager(this);
    connect(m_posterDownloadManager,
        &DownloadManager::sigDownloadFinished,
        this,
        &ImportDialog::onEpisodeDownloadFinished,
        static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));

    connect(ui->movieSearchWidget, &MovieSearchWidget::sigResultClicked, this, &ImportDialog::onMovieChosen);
    connect(ui->concertSearchWidget, &ConcertSearchWidget::sigResultClicked, this, &ImportDialog::onConcertChosen);
    connect(ui->tvShowSearchWidget, &TvShowSearchWidget::sigResultClicked, this, &ImportDialog::onTvShowChosen);
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
    QRegularExpression rx("tt\\d+");
    QRegularExpressionMatch match = rx.match(searchString);
    if (match.hasMatch()) {
        id = ImdbId(match.captured(0));
        searchString = searchString.replace(match.captured(0), "").trimmed();
    }

    m_type = "movie";
    m_filesToMove.clear();
    ui->stackedWidget->setCurrentIndex(0);

    NameFormatter::setExcludeWords(Settings::instance()->excludeWords());
    ui->movieSearchWidget->openAndSearch(NameFormatter::formatName(searchString), id, TmdbId::NoId);

    mediaelch::MovieRenamerPlaceholders placeholders;
    ui->placeholders->setPlaceholders(placeholders);

    ui->chkSeasonDirectories->setVisible(false);
    ui->labelUseSeasonDirectories->setVisible(false);
    ui->seasonNaming->setVisible(false);
    ui->labelSeasonNaming->setVisible(false);
    ui->labelDirectoryNaming->setVisible(m_separateFolders);
    ui->directoryNaming->setVisible(m_separateFolders);

    setDefaults(RenameType::Movies);

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
    const QVector<mediaelch::MediaDirectory>& dirs = Settings::instance()->directorySettings().tvShowDirectories();
    for (int i = 0, n = qsizetype_to_int(dirs.count()); i < n; ++i) {
        if (tvShow->dir().isParentFolderOf(mediaelch::DirectoryPath(dirs.at(i).path))) {
            if (index == -1 || dirs.at(index).path.path().length() < dirs.at(i).path.path().length()) {
                index = i;
            }
        }
    }
    if (index != -1) {
        path = Settings::instance()->directorySettings().tvShowDirectories().at(index).path.path();
    }
    m_importDir = path;

    m_filesToMove.clear();

    mediaelch::EpisodeRenamerPlaceholders placeholders;
    ui->placeholders->setPlaceholders(placeholders);

    ui->chkSeasonDirectories->setVisible(true);
    ui->labelUseSeasonDirectories->setVisible(true);
    ui->seasonNaming->setVisible(true);
    ui->labelSeasonNaming->setVisible(true);
    ui->labelDirectoryNaming->setVisible(false);
    ui->directoryNaming->setVisible(false);
    ui->stackedWidget->setCurrentIndex(3);

    QString search = tvShow->tvdbId().isValid() ? tvShow->tvdbId().toString() : tvShow->title();
    ui->tvShowSearchWidget->search(search);

    setDefaults(RenameType::TvShows);

    return exec();
}

int ImportDialog::execConcert(QString searchString)
{
    m_type = "concert";
    m_filesToMove.clear();
    ui->stackedWidget->setCurrentIndex(2);

    NameFormatter::setExcludeWords(Settings::instance()->excludeWords());
    ui->concertSearchWidget->search(NameFormatter::formatName(searchString));

    mediaelch::ConcertRenamerPlaceholders placeholders;
    ui->placeholders->setPlaceholders(placeholders);

    ui->chkSeasonDirectories->setVisible(false);
    ui->labelUseSeasonDirectories->setVisible(false);
    ui->seasonNaming->setVisible(false);
    ui->labelSeasonNaming->setVisible(false);
    ui->labelDirectoryNaming->setVisible(m_separateFolders);
    ui->directoryNaming->setVisible(m_separateFolders);

    setDefaults(RenameType::Concerts);

    return exec();
}

void ImportDialog::setDefaults(RenameType renameType)
{
    QString fileName;
    QString fileNameMulti;
    QString directoryName;
    QString seasonName;
    QString replacementDelimiter;
    bool renameFiles = false;
    bool renameFolders = false;
    bool useSeasonDirectories = false;
    bool replaceDelimiter = false;
    Settings::instance()->renamePatterns(
        renameType, fileName, fileNameMulti, directoryName, seasonName, replacementDelimiter);
    Settings::instance()->renamings(renameType, renameFiles, renameFolders, useSeasonDirectories, replaceDelimiter);

    ui->fileNaming->setText(fileName);
    ui->directoryNaming->setText(directoryName);
    ui->seasonNaming->setText(seasonName);
    ui->chkSeasonDirectories->setChecked(useSeasonDirectories);
}

void ImportDialog::storeDefaults()
{
    RenameType renameType;
    if (m_type == "movie") {
        renameType = RenameType::Movies;
    } else if (m_type == "tvshow") {
        renameType = RenameType::TvShows;
    } else if (m_type == "concert") {
        renameType = RenameType::Concerts;
    } else {
        return;
    }

    QString fileName;
    QString fileNameMulti;
    QString directoryName;
    QString seasonName;
    QString replacementDelimiter;
    bool renameFiles = false;
    bool renameFolders = false;
    bool useSeasonDirectories = false;
    bool replaceDelimiter = false;
    Settings::instance()->renamePatterns(
        renameType, fileName, fileNameMulti, directoryName, seasonName, replacementDelimiter);
    Settings::instance()->renamings(renameType, renameFiles, renameFolders, useSeasonDirectories, replaceDelimiter);

    fileName = ui->fileNaming->text();
    directoryName = ui->directoryNaming->text();
    seasonName = ui->seasonNaming->text();
    useSeasonDirectories = ui->chkSeasonDirectories->isChecked();

    Settings::instance()->setRenamePatterns(
        renameType, fileName, fileNameMulti, directoryName, seasonName, replacementDelimiter);
    Settings::instance()->setRenamings(renameType, renameFiles, renameFolders, useSeasonDirectories, replaceDelimiter);
}

void ImportDialog::onMovieChosen()
{
    using namespace mediaelch::scraper;

    MovieIdentifier id(ui->movieSearchWidget->scraperMovieId());
    QSet<MovieScraperInfo> infosToLoad = ui->movieSearchWidget->infosToLoad();
    if (ui->movieSearchWidget->scraperId() == CustomMovieScraper::ID) {
        // TODO ANDRE: ids = ui->movieSearchWidget->customScraperIds();
        infosToLoad = Settings::instance()->scraperInfos<MovieScraperInfo>(CustomMovieScraper::ID);
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

    MovieScraper* scraper = Manager::instance()->scrapers().movieScraper(ui->movieSearchWidget->scraperId());
    MediaElch_Assert(scraper != nullptr);
    QHash<MovieScraper*, MovieIdentifier> ids{{scraper, id}};
    m_movie->controller()->loadData(ids, ui->movieSearchWidget->scraperLocale(), infosToLoad);

    connect(m_movie->controller(),
        &MovieController::sigInfoLoadDone,
        this,
        &ImportDialog::onMovieLoadDone,
        Qt::UniqueConnection);
}

void ImportDialog::onConcertChosen()
{
    if (m_concert != nullptr) {
        m_concert->deleteLater();
    }

    ui->stackedWidget->slideInIdx(1, SlidingStackedWidget::direction::RIGHT2LEFT);
    ui->loading->setVisible(true);
    ui->labelLoading->setText(tr("Loading concert information..."));
    ui->badgeSuccess->setVisible(false);
    ui->formLayout->setEnabled(false);

    m_concert = new Concert(files());
    // TODO: Not only TmdbId
    m_concert->controller()->loadData(TmdbId(ui->concertSearchWidget->concertIdentifier()),
        ui->concertSearchWidget->scraper(),
        ui->concertSearchWidget->concertDetailsToLoad());
    connect(m_concert->controller(),
        &ConcertController::sigLoadDone,
        this,
        &ImportDialog::onConcertLoadDone,
        Qt::UniqueConnection);
}

void ImportDialog::onTvShowChosen()
{
    if (m_episode != nullptr) {
        m_episode->deleteLater();
    }

    ui->stackedWidget->slideInIdx(1, SlidingStackedWidget::direction::RIGHT2LEFT);
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

    connect(m_episode.data(), &TvShowEpisode::sigLoaded, this, &ImportDialog::onEpisodeLoadDone, Qt::UniqueConnection);
    m_episode->scrapeData(ui->tvShowSearchWidget->scraper(),
        ui->tvShowSearchWidget->scraperLocale(),
        mediaelch::scraper::ShowIdentifier(ui->tvShowSearchWidget->showIdentifier()),
        ui->tvShowSearchWidget->seasonOrder(),
        ui->tvShowSearchWidget->episodeDetailsToLoad());
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
    QDir dirPath(dir);

    const auto settingsDirs = QVector<mediaelch::MediaDirectory>() //
                              << Settings::instance()->directorySettings().movieDirectories()
                              << Settings::instance()->directorySettings().concertDirectories();

    for (const mediaelch::MediaDirectory& settingsDir : settingsDirs) {
        if (settingsDir.path.path() == dirPath.path()) {
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

void ImportDialog::onMovieLoadDone(Movie* movie)
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

void ImportDialog::onConcertLoadDone(Concert* concert)
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

void ImportDialog::onEpisodeLoadDone(TvShowEpisode* episode)
{
    Q_UNUSED(episode)
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
    qCDebug(generic) << "got image";
    if (m_episode == nullptr) {
        return;
    }
    const QString filePath =
        Manager::instance()->mediaCenterInterface()->imageFileName(elem.episode, ImageType::TvShowEpisodeThumb);
    ImageCache::instance()->invalidateImages(mediaelch::FilePath(filePath));
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
        mediaelch::MovieRenamerPlaceholders renamerPlaceholder;
        mediaelch::MovieRenamerData renamerData{*m_movie};
        renamerData.setIsBluRay(m_movie->discType() == DiscType::BluRay);
        renamerData.setIsDvd(m_movie->discType() == DiscType::Dvd);

        QDir dir(importDir());
        const auto videoDetails = m_movie->streamDetails()->videoDetails();
        renamerData.setVideoDetails(videoDetails);
        if (m_separateFolders) {
            QString newFolderName = ui->directoryNaming->text();

            renamerData.setExtension("");
            newFolderName = renamerPlaceholder.replace(newFolderName, renamerData);

            helper::sanitizeFolderName(newFolderName);
            /// \todo Should also check whether the directory exists.
            if (!dir.mkdir(newFolderName)) {
                QMessageBox::warning(this,
                    tr("Creating destination directory failed"),
                    tr("The destination directory %1 could not be created")
                        .arg(dir.absolutePath() + QDir::separator() + newFolderName));
            }
            dir.cd(newFolderName);
        }
        const auto importFiles = QStringList() << files() << extraFiles();
        for (const QString& file : importFiles) {
            QFileInfo fi(file);
            QString newFileName = ui->fileNaming->text();

            renamerData.setExtension(fi.suffix());
            newFileName = renamerPlaceholder.replace(newFileName, renamerData);

            helper::sanitizeFileName(newFileName);
            m_filesToMove.insert(file, dir.absolutePath() + QDir::separator() + newFileName);
            if (files().contains(file)) {
                m_newFiles.append(dir.absolutePath() + QDir::separator() + newFileName);
            }
        }
        ui->labelLoading->setText(tr("Importing movie..."));

    } else if (m_type == "tvshow") {
        mediaelch::EpisodeRenamerPlaceholders renamerPlaceholder;
        mediaelch::EpisodeRenamerData renamerData{*m_episode};

        const auto videoDetails = m_episode->streamDetails()->videoDetails();
        renamerData.setVideoDetails(videoDetails);
        QDir dir(m_show->dir().toString());
        if (ui->chkSeasonDirectories->isChecked()) {
            QString newFolderName = ui->seasonNaming->text();
            Renamer::replace(newFolderName, "season", m_episode->seasonString());
            helper::sanitizeFolderName(newFolderName);
            dir.mkdir(newFolderName);
            dir.cd(newFolderName);
        }

        auto importFiles = QStringList() << files() << extraFiles();
        for (const QString& file : importFiles) {
            QFileInfo fi(file);
            QString newFileName = ui->fileNaming->text();

            renamerData.setExtension(fi.suffix());
            newFileName = renamerPlaceholder.replace(newFileName, renamerData);

            helper::sanitizeFileName(newFileName);
            m_filesToMove.insert(file, dir.absolutePath() + QDir::separator() + newFileName);
            if (files().contains(file)) {
                m_newFiles.append(dir.absolutePath() + QDir::separator() + newFileName);
            }
        }

        ui->labelLoading->setText(tr("Importing episode..."));

    } else if (m_type == "concert") {
        mediaelch::ConcertRenamerPlaceholders renamerPlaceholder;
        mediaelch::ConcertRenamerData renamerData{*m_concert};

        renamerData.setIsDvd(m_concert->discType() == DiscType::Dvd);
        renamerData.setIsBluRay(m_concert->discType() == DiscType::BluRay);

        const auto videoDetails = m_concert->streamDetails()->videoDetails();
        renamerData.setVideoDetails(videoDetails);
        QDir dir(importDir());
        if (m_separateFolders) {
            QString newFolderName = ui->directoryNaming->text();

            renamerData.setExtension("");
            newFolderName = renamerPlaceholder.replace(newFolderName, renamerData);

            helper::sanitizeFolderName(newFolderName);
            /// \todo Should also check whether the directory exists.
            if (!dir.mkdir(newFolderName)) {
                QMessageBox::warning(this,
                    tr("Creating destination directory failed"),
                    tr("The destination directory %1 could not be created")
                        .arg(dir.absolutePath() + QDir::separator() + newFolderName));
            }
            dir.cd(newFolderName);
        }
        const auto importFiles = QStringList() << files() << extraFiles();
        for (const QString& file : importFiles) {
            QFileInfo fi(file);
            QString newFileName = ui->fileNaming->text();

            renamerData.setExtension(fi.suffix());
            newFileName = renamerPlaceholder.replace(newFileName, renamerData);

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
        Q_UNUSED(m_movie->controller()->loadStreamDetailsFromFile());
        m_movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
        m_movie->controller()->loadData(Manager::instance()->mediaCenterInterface());

        mediaelch::MoviePersistence persistence{*Manager::instance()->database()};
        persistence.addMovie(m_movie, mediaelch::DirectoryPath(importDir()));
        Manager::instance()->database()->db().commit();
        Manager::instance()->movieModel()->addMovie(m_movie);
        m_movie = nullptr;

    } else if (m_type == "tvshow") {
        if (m_show->showMissingEpisodes()) {
            m_show->clearMissingEpisodes();
        }

        m_episode->setFiles(m_newFiles);
        Q_UNUSED(m_episode->loadStreamDetailsFromFile());
        m_show->addEpisode(m_episode);
        m_episode->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
        m_episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow(), true, false);
        mediaelch::TvShowPersistence persistence{*Manager::instance()->database()};
        persistence.add(m_episode, mediaelch::DirectoryPath(importDir()), m_show->databaseId());

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
        Q_UNUSED(m_concert->controller()->loadStreamDetailsFromFile());
        m_concert->controller()->saveData(Manager::instance()->mediaCenterInterface());
        m_concert->controller()->loadData(Manager::instance()->mediaCenterInterface());
        mediaelch::ConcertPersistence persistence{*Manager::instance()->database()};
        persistence.add(m_concert, mediaelch::DirectoryPath(importDir()));
        Manager::instance()->database()->db().commit();
        Manager::instance()->concertModel()->addConcert(m_concert);
        m_concert = nullptr;
    }

    Notificator::instance()->notify(Notificator::Class::Information,
        tr("Import finished"),
        tr("Import of %n files has finished", "", qsizetype_to_int(files().count())));

    ui->loading->setVisible(false);
    ui->badgeSuccess->setText(tr("Import has finished"));

    m_filesToMove.clear();

    ui->btnReject->setVisible(false);
    ui->btnAccept->setVisible(true);
}
