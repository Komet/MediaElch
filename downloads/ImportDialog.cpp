#include "ImportDialog.h"
#include "ui_ImportDialog.h"

#include <QMessageBox>
#include <QMovie>
#include "globals/Helper.h"
#include "data/ImageCache.h"
#include "data/TvShowFileSearcher.h"
#include "globals/Manager.h"
#include "globals/NameFormatter.h"
#include "notifications/Notificator.h"
#include "renamer/Renamer.h"
#include "settings/Settings.h"

ImportDialog::ImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog)
{
    ui->setupUi(this);

    ui->stackedWidget->setAnimation(QEasingCurve::Linear);
    ui->stackedWidget->setSpeed(200);
    ui->badgeSuccess->setActive(true);
    ui->badgeSuccess->setBadgeType(Badge::LabelSuccess);
    ui->badgeSuccess->setShowActiveMark(true);

    QMovie *loadingMovie = new QMovie(":/img/spinner.gif");
    loadingMovie->start();
    ui->loading->setMovie(loadingMovie);

    m_timer.setInterval(500);

    m_posterDownloadManager = new DownloadManager(this);
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onEpisodeDownloadFinished(DownloadManagerElement)));

    connect(ui->movieSearchWidget, SIGNAL(sigResultClicked()), this, SLOT(onMovieChosen()));
    connect(ui->concertSearchWidget, SIGNAL(sigResultClicked()), this, SLOT(onConcertChosen()));
    connect(ui->tvShowSearchEpisode, SIGNAL(sigResultClicked()), this, SLOT(onTvShowChosen()));
    connect(ui->btnImport, SIGNAL(clicked()), this, SLOT(onImport()));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onFileWatcherTimeout()));
}

ImportDialog::~ImportDialog()
{
    delete ui;
}

void ImportDialog::reject()
{
    if (!m_filesToMove.isEmpty())
        return;

    if (m_movie) {
        m_movie->controller()->abortDownloads();
        m_movie->deleteLater();
    }

    if (m_concert) {
        m_concert->controller()->abortDownloads();
        m_concert->deleteLater();
    }

    if (m_episode)
        m_episode->deleteLater();

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
    QString id;
    QRegExp rx("tt(\\d+)");
    if (rx.indexIn(searchString) != -1) {
        id = rx.cap(0);
        searchString = searchString.replace(rx.cap(0), "").trimmed();
    }

    m_type = "movie";
    m_filesToMove.clear();
    ui->stackedWidget->setCurrentIndex(0);
    ui->movieSearchWidget->search(NameFormatter::instance()->formatName(searchString), id, "");

    ui->placeholders->setType(Renamer::TypeMovies);
    ui->chkSeasonDirectories->setVisible(false);
    ui->labelUseSeasonDirectories->setVisible(false);
    ui->seasonNaming->setVisible(false);
    ui->labelSeasonNaming->setVisible(false);
    ui->labelDirectoryNaming->setVisible(m_separateFolders);
    ui->directoryNaming->setVisible(m_separateFolders);

    setDefaults(Renamer::TypeMovies);

    return exec();
}

int ImportDialog::execTvShow(QString searchString, TvShow *tvShow)
{
    Q_UNUSED(searchString);

    m_type = "tvshow";
    m_show = tvShow;

    // get path
    QString path;
    int index = -1;
    for (int i=0, n=Settings::instance()->tvShowDirectories().count() ; i<n ; ++i) {
        if (tvShow->dir().startsWith(Settings::instance()->tvShowDirectories().at(i).path)) {
            if (index == -1)
                index = i;
            else if (Settings::instance()->tvShowDirectories().at(index).path.length() < Settings::instance()->tvShowDirectories().at(i).path.length())
                index = i;
        }
    }
    if (index != -1)
        path = Settings::instance()->tvShowDirectories().at(index).path;
    m_importDir = path;

    m_filesToMove.clear();

    ui->placeholders->setType(Renamer::TypeTvShows);
    ui->chkSeasonDirectories->setVisible(true);
    ui->labelUseSeasonDirectories->setVisible(true);
    ui->seasonNaming->setVisible(true);
    ui->labelSeasonNaming->setVisible(true);
    ui->labelDirectoryNaming->setVisible(false);
    ui->directoryNaming->setVisible(false);
    ui->stackedWidget->setCurrentIndex(3);
    ui->tvShowSearchEpisode->search(tvShow->name(), tvShow->tvdbId());

    setDefaults(Renamer::TypeTvShows);

    return exec();
}

int ImportDialog::execConcert(QString searchString)
{
    m_type = "concert";
    m_filesToMove.clear();
    ui->stackedWidget->setCurrentIndex(2);
    ui->concertSearchWidget->search(NameFormatter::instance()->formatName(searchString));

    ui->placeholders->setType(Renamer::TypeConcerts);
    ui->chkSeasonDirectories->setVisible(false);
    ui->labelUseSeasonDirectories->setVisible(false);
    ui->seasonNaming->setVisible(false);
    ui->labelSeasonNaming->setVisible(false);
    ui->labelDirectoryNaming->setVisible(m_separateFolders);
    ui->directoryNaming->setVisible(m_separateFolders);

    setDefaults(Renamer::TypeConcerts);

    return exec();
}

void ImportDialog::setDefaults(int renameType)
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
    int renameType;
    if (m_type == "movie")
        renameType = Renamer::TypeMovies;
    else if (m_type == "tvshow")
        renameType = Renamer::TypeTvShows;
    else if (m_type == "concert")
        renameType = Renamer::TypeConcerts;
    else
        return;

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
    QMap<ScraperInterface*, QString> ids;
    QList<int> infosToLoad;
    if (ui->movieSearchWidget->scraperId() == "custom-movie") {
        ids = ui->movieSearchWidget->customScraperIds();
        infosToLoad = Settings::instance()->scraperInfos(WidgetMovies, "custom-movie");
    } else {
        ids.insert(0, ui->movieSearchWidget->scraperMovieId());
        infosToLoad = ui->movieSearchWidget->infosToLoad();
    }

    if (m_movie)
        m_movie->deleteLater();

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
    if (m_concert)
        m_concert->deleteLater();

    ui->stackedWidget->slideInIdx(1, SlidingStackedWidget::RIGHT2LEFT);
    ui->loading->setVisible(true);
    ui->labelLoading->setText(tr("Loading concert information..."));
    ui->badgeSuccess->setVisible(false);
    ui->formLayout->setEnabled(false);

    m_concert = new Concert(files());
    m_concert->controller()->loadData(ui->concertSearchWidget->scraperId(),
                                      Manager::instance()->concertScrapers().at(ui->concertSearchWidget->scraperNo()),
                                      ui->concertSearchWidget->infosToLoad());
    connect(m_concert->controller(), SIGNAL(sigLoadDone(Concert*)), this, SLOT(onLoadDone(Concert*)), Qt::UniqueConnection);
}

void ImportDialog::onTvShowChosen()
{
    if (m_episode)
        m_episode->deleteLater();

    ui->stackedWidget->slideInIdx(1, SlidingStackedWidget::RIGHT2LEFT);
    ui->loading->setVisible(true);
    ui->labelLoading->setText(tr("Loading episode information..."));
    ui->badgeSuccess->setVisible(false);
    ui->formLayout->setEnabled(false);

    m_episode = new TvShowEpisode(files(), m_show);
    m_episode->setSeason(TvShowFileSearcher::getSeasonNumber(files()));
    QList<int> episodes = TvShowFileSearcher::getEpisodeNumbers(files());
    if (!episodes.isEmpty())
        m_episode->setEpisode(episodes.first());
    m_episode->loadData(ui->tvShowSearchEpisode->scraperId(), Manager::instance()->tvScrapers().at(0), ui->tvShowSearchEpisode->infosToLoad());
    connect(m_episode, SIGNAL(sigLoaded()), this, SLOT(onEpisodeLoadDone()), Qt::UniqueConnection);
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
    foreach (SettingsDir settingsDir, QList<SettingsDir>() << Settings::instance()->movieDirectories() << Settings::instance()->concertDirectories()) {
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

void ImportDialog::onLoadDone(Movie *movie)
{
    if (movie != m_movie)
        return;

    ui->loading->setVisible(false);
    ui->badgeSuccess->setText(tr("Movie information was loaded"));
    ui->labelLoading->setVisible(false);
    ui->badgeSuccess->setVisible(true);
    ui->btnImport->setEnabled(true);
    ui->formLayout->setEnabled(true);
}

void ImportDialog::onLoadDone(Concert *concert)
{
    if (concert != m_concert)
        return;

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
    if (!m_episode)
        return;

    ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->imageFileName(elem.episode, ImageType::TvShowEpisodeThumb));
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
        if (m_separateFolders) {
            QString newFolderName = ui->directoryNaming->text();
            Renamer::replace(newFolderName, "title", m_movie->name());
            Renamer::replace(newFolderName, "originalTitle", m_movie->originalName());
            Renamer::replace(newFolderName, "sortTitle", m_movie->sortTitle());
            Renamer::replace(newFolderName, "year", m_movie->released().toString("yyyy"));
            Renamer::replace(newFolderName, "resolution", Helper::instance()->matchResolution(m_movie->streamDetails()->videoDetails().value("width").toInt(),
                                                                                              m_movie->streamDetails()->videoDetails().value("height").toInt(),
                                                                                              m_movie->streamDetails()->videoDetails().value("scantype")));
            Renamer::replaceCondition(newFolderName, "bluray", m_movie->discType() == DiscBluRay);
            Renamer::replaceCondition(newFolderName, "dvd", m_movie->discType() == DiscDvd);
            Renamer::replaceCondition(newFolderName, "3D", m_movie->streamDetails()->videoDetails().value("stereomode") != "");
            Renamer::replaceCondition(newFolderName, "movieset", m_movie->set());
            Helper::instance()->sanitizeFileName(newFolderName);
            if (!dir.mkdir(newFolderName)) {
                QMessageBox::warning(this, tr("Creating destination directory failed"),
                                     tr("The destination directory %1 could not be created").arg(dir.absolutePath() + QDir::separator() + newFolderName));
            }
            dir.cd(newFolderName);
        }
        foreach (QString file, QStringList() << files() << extraFiles()) {
            QFileInfo fi(file);
            QString newFileName = ui->fileNaming->text();
            Renamer::replace(newFileName, "title", m_movie->name());
            Renamer::replace(newFileName, "originalTitle", m_movie->originalName());
            Renamer::replace(newFileName, "sortTitle", m_movie->sortTitle());
            Renamer::replace(newFileName, "year", m_movie->released().toString("yyyy"));
            Renamer::replace(newFileName, "extension", fi.suffix());
            Renamer::replace(newFileName, "resolution", Helper::instance()->matchResolution(m_movie->streamDetails()->videoDetails().value("width").toInt(),
                                                                                            m_movie->streamDetails()->videoDetails().value("height").toInt(),
                                                                                            m_movie->streamDetails()->videoDetails().value("scantype")));
            Renamer::replaceCondition(newFileName, "imdbId", m_movie->id());
            Renamer::replaceCondition(newFileName, "movieset", m_movie->set());
            Renamer::replaceCondition(newFileName, "3D", m_movie->streamDetails()->videoDetails().value("stereomode") != "");
            Helper::instance()->sanitizeFileName(newFileName);
            m_filesToMove.insert(file, dir.absolutePath() + QDir::separator() + newFileName);
            if (files().contains(file))
                m_newFiles.append(dir.absolutePath() + QDir::separator() + newFileName);
        }
        ui->labelLoading->setText(tr("Importing movie..."));

    } else if (m_type == "tvshow") {

        QDir dir(m_show->dir());
        if (ui->chkSeasonDirectories->isChecked()) {
            QString newFolderName = ui->seasonNaming->text();
            Renamer::replace(newFolderName, "season", m_episode->seasonString());
            Helper::instance()->sanitizeFileName(newFolderName);
            dir.mkdir(newFolderName);
            dir.cd(newFolderName);
        }

        foreach (QString file, QStringList() << files() << extraFiles()) {
            QFileInfo fi(file);
            QString newFileName = ui->fileNaming->text();

            Renamer::replace(newFileName, "title", m_episode->name());
            Renamer::replace(newFileName, "showTitle", m_episode->showTitle());
            Renamer::replace(newFileName, "year", m_episode->firstAired().toString("yyyy"));
            Renamer::replace(newFileName, "extension", fi.suffix());
            Renamer::replace(newFileName, "episode", m_episode->episodeString());
            Renamer::replace(newFileName, "season", m_episode->seasonString());
            Renamer::replace(newFileName, "resolution", Helper::instance()->matchResolution(m_episode->streamDetails()->videoDetails().value("width").toInt(),
                                                                                            m_episode->streamDetails()->videoDetails().value("height").toInt(),
                                                                                            m_episode->streamDetails()->videoDetails().value("scantype")));
            Renamer::replaceCondition(newFileName, "3D", m_episode->streamDetails()->videoDetails().value("stereomode") != "");
            Helper::instance()->sanitizeFileName(newFileName);
            m_filesToMove.insert(file, dir.absolutePath() + QDir::separator() + newFileName);
            if (files().contains(file))
                m_newFiles.append(dir.absolutePath() + QDir::separator() + newFileName);
        }

        ui->labelLoading->setText(tr("Importing episode..."));

    } else if (m_type == "concert") {

        QDir dir(importDir());
        if (m_separateFolders) {
            QString newFolderName = ui->directoryNaming->text();
            Renamer::replace(newFolderName, "title", m_concert->name());
            Renamer::replace(newFolderName, "artist", m_concert->artist());
            Renamer::replace(newFolderName, "album", m_concert->album());
            Renamer::replace(newFolderName, "year", m_concert->released().toString("yyyy"));
            Renamer::replace(newFolderName, "resolution", Helper::instance()->matchResolution(m_concert->streamDetails()->videoDetails().value("width").toInt(),
                                                                                              m_concert->streamDetails()->videoDetails().value("height").toInt(),
                                                                                              m_concert->streamDetails()->videoDetails().value("scantype")));
            Renamer::replaceCondition(newFolderName, "bluray", m_concert->discType() == DiscBluRay);
            Renamer::replaceCondition(newFolderName, "dvd", m_concert->discType() == DiscDvd);
            Renamer::replaceCondition(newFolderName, "3D", m_concert->streamDetails()->videoDetails().value("stereomode") != "");
            Helper::instance()->sanitizeFileName(newFolderName);
            if (!dir.mkdir(newFolderName)) {
                QMessageBox::warning(this, tr("Creating destination directory failed"),
                                     tr("The destination directory %1 could not be created").arg(dir.absolutePath() + QDir::separator() + newFolderName));
            }
            dir.cd(newFolderName);
        }
        foreach (QString file, QStringList() << files() << extraFiles()) {
            QFileInfo fi(file);
            QString newFileName = ui->fileNaming->text();
            Renamer::replace(newFileName, "title", m_concert->name());
            Renamer::replace(newFileName, "artist", m_concert->artist());
            Renamer::replace(newFileName, "album", m_concert->album());
            Renamer::replace(newFileName, "year", m_concert->released().toString("yyyy"));
            Renamer::replace(newFileName, "extension", fi.suffix());
            Renamer::replace(newFileName, "resolution", Helper::instance()->matchResolution(m_concert->streamDetails()->videoDetails().value("width").toInt(),
                                                                                            m_concert->streamDetails()->videoDetails().value("height").toInt(),
                                                                                            m_concert->streamDetails()->videoDetails().value("scantype")));
            Renamer::replaceCondition(newFileName, "3D", m_concert->streamDetails()->videoDetails().value("stereomode") != "");
            Helper::instance()->sanitizeFileName(newFileName);
            m_filesToMove.insert(file, dir.absolutePath() + QDir::separator() + newFileName);
            if (files().contains(file))
                m_newFiles.append(dir.absolutePath() + QDir::separator() + newFileName);
        }
        ui->labelLoading->setText(tr("Importing concert..."));

    }

    ui->loading->setVisible(true);
    ui->btnImport->setEnabled(false);
    ui->btnReject->setEnabled(false);
    m_worker = new FileWorker();
    m_worker->setFiles(m_filesToMove);
    m_workerThread = new QThread(this);
    if (ui->chkKeepSourceFiles->isChecked())
        connect(m_workerThread, SIGNAL(started()), m_worker, SLOT(copyFiles()));
    else
        connect(m_workerThread, SIGNAL(started()), m_worker, SLOT(moveFiles()));
    connect(m_workerThread, SIGNAL(finished()), m_worker, SLOT(deleteLater()));
    connect(m_workerThread, SIGNAL(finished()), m_workerThread, SLOT(deleteLater()));
    connect(m_worker, SIGNAL(sigFinished()), m_workerThread, SLOT(quit()));
    connect(m_worker, SIGNAL(sigFinished()), this, SLOT(onMovingFilesFinished()));
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
        if (!destFi.exists())
            continue;
        destinationSize += destFi.size();
    }

    if (sourceSize == 0)
        return;

    ui->progressBar->setValue(qRound((float)destinationSize*100/sourceSize));
}

void ImportDialog::onMovingFilesFinished()
{
    ui->progressBar->setValue(100);
    m_timer.stop();
    if (m_type == "movie") {
        m_movie->setFiles(m_newFiles);
        m_movie->setInSeparateFolder(m_separateFolders);
        if (!m_newFiles.isEmpty())
            m_movie->setFileLastModified(QFileInfo(m_newFiles.first()).lastModified());
        m_movie->controller()->loadStreamDetailsFromFile();
        m_movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
        m_movie->controller()->loadData(Manager::instance()->mediaCenterInterface());
        Manager::instance()->database()->add(m_movie, importDir());
        Manager::instance()->database()->commit();
        Manager::instance()->movieModel()->addMovie(m_movie);
        m_movie = 0;
    } else if (m_type == "tvshow") {
        if (m_show->showMissingEpisodes())
            m_show->clearMissingEpisodes();

        m_episode->setFiles(m_newFiles);
        m_episode->loadStreamDetailsFromFile();
        m_show->addEpisode(m_episode);
        m_episode->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
        m_episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
        Manager::instance()->database()->add(m_episode, importDir(), m_show->databaseId());
        bool newSeason = true;
        foreach (TvShowEpisode *episode, m_show->episodes()) {
            if (episode->season() == m_episode->season() && episode != m_episode) {
                newSeason = false;
                break;
            }
        }

        if (newSeason) {
            m_show->modelItem()->appendChild(m_episode->season(), m_episode->seasonString(), m_show)->appendChild(m_episode);
        } else {
            for (int i=0, n=m_show->modelItem()->childCount() ; i<n ; ++i) {
                TvShowModelItem *item = m_show->modelItem()->child(i);
                if (item->type() == TypeSeason && item->season() == m_episode->seasonString()) {
                    item->appendChild(m_episode);
                    break;
                }
            }
        }

        if (m_show->showMissingEpisodes())
            m_show->fillMissingEpisodes();
        else if (newSeason)
            TvShowFilesWidget::instance()->renewModel(true);

        m_episode = 0;
        m_show = 0;
    } else if (m_type == "concert") {
        m_concert->setFiles(m_newFiles);
        m_concert->controller()->loadStreamDetailsFromFile();
        m_concert->controller()->saveData(Manager::instance()->mediaCenterInterface());
        m_concert->controller()->loadData(Manager::instance()->mediaCenterInterface());
        Manager::instance()->database()->add(m_concert, importDir());
        Manager::instance()->database()->commit();
        Manager::instance()->concertModel()->addConcert(m_concert);
        m_concert = 0;
    }

    Notificator::instance()->notify(Notificator::Information, tr("Import finished"), tr("Import of %n file(s) has finished", "", files().count()));

    ui->loading->setVisible(false);
    ui->badgeSuccess->setText(tr("Import has finished"));

    m_filesToMove.clear();

    ui->btnReject->setVisible(false);
    ui->btnAccept->setVisible(true);
}
