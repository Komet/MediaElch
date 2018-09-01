#include "Renamer.h"
#include "ui_Renamer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "globals/Helper.h"
#include "globals/Manager.h"

Renamer::Renamer(QWidget *parent) : QDialog(parent), ui(new Ui::Renamer)
{
    ui->setupUi(this);

    ui->resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->resultsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
#ifdef Q_OS_MAC
    QFont font = ui->resultsTable->font();
    font.setPointSize(font.pointSize() - 2);
    ui->resultsTable->setFont(font);
#endif

    connect(ui->chkDirectoryNaming, &QCheckBox::stateChanged, this, &Renamer::onChkRenameDirectories);
    connect(ui->chkFileNaming, &QCheckBox::stateChanged, this, &Renamer::onChkRenameFiles);
    connect(ui->chkSeasonDirectories, &QCheckBox::stateChanged, this, &Renamer::onChkUseSeasonDirectories);
    connect(ui->btnDryRun, &QAbstractButton::clicked, this, &Renamer::onDryRun);
    connect(ui->btnRename, &QAbstractButton::clicked, this, &Renamer::onRename);

    onChkRenameDirectories();
    onChkRenameFiles();

    m_extraFiles = Settings::instance()->advanced()->subtitleFilters();
    ui->helpLabel->setText(tr("Please see %1 for help and examples on how to use the renamer.")
                               .arg("<a "
                                    "href=\"https://github.com/Komet/MediaElch/blob/master/doc/"
                                    "RenamingFiles.md\">https://github.com/Komet/MediaElch/blob/master/doc/"
                                    "RenamingFiles.md</a>"));
}

Renamer::~Renamer()
{
    delete ui;
}

int Renamer::exec()
{
    m_filesRenamed = false;
    m_renameErrorOccured = false;

    switch (m_renameType) {
    case RenameType::Movies: ui->infoLabel->setText(tr("%n Movie(s) will be renamed", "", m_movies.count())); break;
    case RenameType::Concerts:
        ui->infoLabel->setText(tr("%n Concert(s) will be renamed", "", m_concerts.count()));
        break;
    case RenameType::TvShows:
        ui->infoLabel->setText(tr("%n TV Show(s) and %1", "", m_shows.count())
                                   .arg(tr("%n Episode(s) will be renamed", "", m_episodes.count())));
        break;
    default: break;
    }

    QString fileName;
    QString fileNameMulti;
    QString directoryName;
    QString seasonName;
    bool renameFiles;
    bool renameFolders;
    bool useSeasonDirectories;
    Settings::instance()->renamePatterns(m_renameType, fileName, fileNameMulti, directoryName, seasonName);
    Settings::instance()->renamings(m_renameType, renameFiles, renameFolders, useSeasonDirectories);
    ui->fileNaming->setText(fileName);
    ui->fileNamingMulti->setText(fileNameMulti);
    ui->directoryNaming->setText(directoryName);
    ui->seasonNaming->setText(seasonName);
    ui->chkFileNaming->setChecked(renameFiles);
    ui->chkDirectoryNaming->setChecked(renameFolders);
    ui->chkSeasonDirectories->setChecked(useSeasonDirectories);

    ui->chkSeasonDirectories->setVisible(m_renameType == RenameType::TvShows);
    ui->seasonNaming->setVisible(m_renameType == RenameType::TvShows);
    ui->labelSeasonDirectory->setVisible(m_renameType == RenameType::TvShows);

    ui->placeholders->setType(m_renameType);

    ui->results->clear();
    ui->resultsTable->setRowCount(0);
    ui->btnDryRun->setEnabled(true);
    ui->btnRename->setEnabled(true);

    ui->tabWidget->setCurrentIndex(0);

    return QDialog::exec();
}

void Renamer::reject()
{
    m_movies.clear();
    m_concerts.clear();
    m_shows.clear();
    m_episodes.clear();

    Settings::instance()->setRenamePatterns(m_renameType,
        ui->fileNaming->text(),
        ui->fileNamingMulti->text(),
        ui->directoryNaming->text(),
        ui->seasonNaming->text());
    Settings::instance()->setRenamings(m_renameType,
        ui->chkFileNaming->isChecked(),
        ui->chkDirectoryNaming->isChecked(),
        ui->chkSeasonDirectories->isChecked());

    QDialog::reject();
    if (m_filesRenamed) {
        QTimer::singleShot(0, this, &Renamer::onRenamed);
    }
}

void Renamer::onRenamed()
{
    emit sigFilesRenamed(m_renameType);
}

bool Renamer::renameErrorOccured() const
{
    return m_renameErrorOccured;
}

void Renamer::setMovies(QList<Movie *> movies)
{
    m_movies = movies;
}

void Renamer::setConcerts(QList<Concert *> concerts)
{
    m_concerts = concerts;
}

void Renamer::setShows(QList<TvShow *> shows)
{
    m_shows = shows;
}

void Renamer::setEpisodes(QList<TvShowEpisode *> episodes)
{
    m_episodes = episodes;
}

void Renamer::setRenameType(RenameType type)
{
    m_renameType = type;
}

void Renamer::onChkRenameDirectories()
{
    ui->directoryNaming->setEnabled(ui->chkDirectoryNaming->isChecked());
}

void Renamer::onChkRenameFiles()
{
    ui->fileNaming->setEnabled(ui->chkFileNaming->isChecked());
    ui->fileNamingMulti->setEnabled(ui->chkFileNaming->isChecked());
}

void Renamer::onChkUseSeasonDirectories()
{
    ui->seasonNaming->setEnabled(ui->chkSeasonDirectories->isChecked());
}

void Renamer::onRename()
{
    ui->tabWidget->setCurrentIndex(1);
    ui->results->clear();
    ui->resultsTable->setRowCount(0);
    ui->btnRename->setEnabled(false);
    ui->btnDryRun->setEnabled(false);

    RenamerConfig config;
    config.dryRun = false;
    config.filePattern = ui->fileNaming->text();
    config.filePatternMulti = ui->fileNamingMulti->text();
    config.renameFiles = ui->chkFileNaming->isChecked();

    if (m_renameType == RenameType::Movies) {
        config.directoryPattern = ui->directoryNaming->text();
        config.renameDirectories = ui->chkDirectoryNaming->isChecked();
        renameMovies(m_movies, config);

    } else if (m_renameType == RenameType::Concerts) {
        config.directoryPattern = ui->directoryNaming->text();
        config.renameDirectories = ui->chkDirectoryNaming->isChecked();
        renameConcerts(m_concerts, config);

    } else if (m_renameType == RenameType::TvShows) {
        config.directoryPattern = ui->seasonNaming->text();
        config.renameDirectories = ui->chkSeasonDirectories->isChecked();
        renameEpisodes(m_episodes, config);
        renameShows(m_shows, ui->directoryNaming->text(), ui->chkDirectoryNaming->isChecked());
    }
    m_filesRenamed = true;
    ui->results->append("<span style=\"color:#01a800;\"><b>" + tr("Finished") + "</b></span>");
}

void Renamer::onDryRun()
{
    ui->tabWidget->setCurrentIndex(1);
    ui->results->clear();
    ui->resultsTable->setRowCount(0);

    RenamerConfig config;
    config.dryRun = true;
    config.filePattern = ui->fileNaming->text();
    config.filePatternMulti = ui->fileNamingMulti->text();
    config.renameFiles = ui->chkFileNaming->isChecked();

    if (m_renameType == RenameType::Movies) {
        config.directoryPattern = ui->directoryNaming->text();
        config.renameDirectories = ui->chkDirectoryNaming->isChecked();
        renameMovies(m_movies, config);

    } else if (m_renameType == RenameType::Concerts) {
        config.directoryPattern = ui->directoryNaming->text();
        config.renameDirectories = ui->chkDirectoryNaming->isChecked();
        renameConcerts(m_concerts, config);

    } else if (m_renameType == RenameType::TvShows) {
        config.directoryPattern = ui->seasonNaming->text();
        config.renameDirectories = ui->chkSeasonDirectories->isChecked();
        renameEpisodes(m_episodes, config);
        renameShows(m_shows, ui->directoryNaming->text(), ui->chkDirectoryNaming->isChecked(), true);
    }
    ui->results->append("<span style=\"color:#01a800;\"><b>" + tr("Finished") + "</b></span>");
}

void Renamer::renameMovies(QList<Movie *> movies, const RenamerConfig &config)
{
    if ((config.renameFiles && config.filePattern.isEmpty())
        || (config.renameDirectories && config.directoryPattern.isEmpty())) {
        return;
    }

    for (Movie *movie : movies) {
        if (movie->files().isEmpty() || (movie->files().count() > 1 && config.filePatternMulti.isEmpty())) {
            continue;
        }

        if (movie->hasChanged()) {
            ui->results->append(tr("<b>Movie</b> \"%1\" has been edited but is not saved").arg(movie->name()));
            continue;
        }

        qApp->processEvents();
        QFileInfo fi(movie->files().first());
        QString fiCanonicalPath = fi.canonicalPath();
        QDir dir(fi.canonicalPath());
        QString newFolderName = config.directoryPattern;
        QString newFileName;
        QStringList FilmFiles;
        QStringList newMovieFiles;
        QString parentDirName;
        bool errorOccured = false;

        for (const QString &file : movie->files()) {
            QFileInfo fi(file);
            newMovieFiles.append(fi.fileName());
        }

        QDir chkDir(fi.canonicalPath());
        chkDir.cdUp();

        const bool isBluRay = Helper::instance()->isBluRay(chkDir.path());
        const bool isDvd = Helper::instance()->isDvd(chkDir.path());

        if (isBluRay || isDvd) {
            parentDirName = dir.dirName();
            dir.cdUp();
        }

        if (!isBluRay && !isDvd && config.renameFiles) {
            newMovieFiles.clear();
            int partNo = 0;
            const auto videoDetails = movie->streamDetails()->videoDetails();
            for (const QString &file : movie->files()) {
                newFileName = (movie->files().count() == 1) ? config.filePattern : config.filePatternMulti;
                QFileInfo fi(file);
                QString baseName = fi.completeBaseName();
                QDir currentDir = fi.dir();
                Renamer::replace(newFileName, "title", movie->name());
                Renamer::replace(newFileName, "originalTitle", movie->originalName());
                Renamer::replace(newFileName, "sortTitle", movie->sortTitle());
                Renamer::replace(newFileName, "year", movie->released().toString("yyyy"));
                Renamer::replace(newFileName, "extension", fi.suffix());
                Renamer::replace(newFileName, "partNo", QString::number(++partNo));
                Renamer::replace(newFileName, "videoCodec", movie->streamDetails()->videoCodec());
                Renamer::replace(newFileName, "audioCodec", movie->streamDetails()->audioCodec());
                Renamer::replace(newFileName, "channels", QString::number(movie->streamDetails()->audioChannels()));
                Renamer::replace(newFileName,
                    "resolution",
                    Helper::instance()->matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                        videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                        videoDetails.value(StreamDetails::VideoDetails::ScanType)));
                Renamer::replaceCondition(newFileName, "imdbId", movie->id());
                Renamer::replaceCondition(newFileName, "movieset", movie->set());
                Renamer::replaceCondition(
                    newFileName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
                Helper::instance()->sanitizeFileName(newFileName);
                if (fi.fileName() != newFileName) {
                    int row = addResult(fi.fileName(), newFileName, RenameOperation::Rename);
                    if (!config.dryRun) {
                        if (!rename(file, fi.canonicalPath() + "/" + newFileName)) {
                            setResultStatus(row, RenameResult::Failed);
                            errorOccured = true;
                            continue;
                        } else {
                            FilmFiles.append(newFileName);
                            newMovieFiles.append(newFileName);
                        }
                    } else {
                        FilmFiles.append(newFileName);
                    }

                    foreach (const QString &trailerFile,
                        currentDir.entryList(QStringList() << fi.completeBaseName() + "-trailer.*",
                            QDir::Files | QDir::NoDotAndDotDot)) {
                        QFileInfo trailer(fi.canonicalPath() + "/" + trailerFile);
                        QString newTrailerFileName = newFileName;
                        newTrailerFileName = newTrailerFileName.left(newTrailerFileName.lastIndexOf(".")) + "-trailer."
                                             + trailer.suffix();
                        if (trailer.fileName() != newTrailerFileName) {
                            int row = addResult(trailer.fileName(), newTrailerFileName, RenameOperation::Rename);
                            if (!config.dryRun) {
                                if (!rename(fi.canonicalPath() + "/" + trailerFile,
                                        fi.canonicalPath() + "/" + newTrailerFileName)) {
                                    setResultStatus(row, RenameResult::Failed);
                                } else {
                                    FilmFiles.append(newTrailerFileName);
                                }
                            } else {
                                FilmFiles.append(newTrailerFileName);
                            }
                        } else {
                            FilmFiles.append(trailer.fileName());
                        }
                    }

                    /*
                    QStringList filters;
                    foreach (const QString &extra, m_extraFiles)
                        filters << baseName + extra;
                    foreach (const QString &subFileName, currentDir.entryList(filters, QDir::Files |
                    QDir::NoDotAndDotDot)) { QString subSuffix = subFileName.mid(baseName.length()); QString newBaseName
                    = newFileName.left(newFileName.lastIndexOf(".")); QString newSubName = newBaseName + subSuffix;
                        ui->results->append(tr("<b>Rename File</b> \"%1\" to \"%2\"").arg(subFileName).arg(newSubName));
                        if (!dryRun) {
                            if (!rename(currentDir.canonicalPath() + "/" + subFileName, currentDir.canonicalPath() + "/"
                    + newSubName)) ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") +
                    "</b></span>"); else FilmFiles.append(newSubName);
                        }
                        else
                            FilmFiles.append(newSubName);
                    }
                    */

                    foreach (Subtitle *subtitle, movie->subtitles()) {
                        QString subFileName = QFileInfo(newFileName).completeBaseName();
                        if (!subtitle->language().isEmpty()) {
                            subFileName.append("." + subtitle->language());
                        }
                        if (subtitle->forced()) {
                            subFileName.append(".forced");
                        }

                        QStringList newSubFiles;
                        foreach (const QString &subFile, subtitle->files()) {
                            QFileInfo subFi(fi.canonicalPath() + "/" + subFile);
                            QString newSubFileName = subFileName + "." + subFi.suffix();
                            int row = addResult(subFile, newSubFileName, RenameOperation::Rename);
                            if (!config.dryRun) {
                                if (!rename(fi.canonicalPath() + "/" + subFile,
                                        fi.canonicalPath() + "/" + newSubFileName)) {
                                    newSubFiles << subFile;
                                    setResultStatus(row, RenameResult::Failed);
                                } else {
                                    newSubFiles << newSubFileName;
                                    FilmFiles.append(newSubFileName);
                                }
                            } else {
                                FilmFiles.append(newSubFileName);
                            }
                        }
                        if (!config.dryRun) {
                            subtitle->setFiles(newSubFiles, false);
                        }
                    }
                } else {
                    FilmFiles.append(fi.fileName());
                    newMovieFiles.append(fi.fileName());
                }
            }

            // Rename additional files (nfo, poster, etc)
            const auto renameFileType = [&](QString filePath, DataFileType dataFileType) -> void {
                if (filePath.isEmpty()) {
                    // File does not exist, e.g. there is no poster.
                    return;
                }

                QList<DataFile> files = Settings::instance()->dataFiles(dataFileType);
                if (files.isEmpty()) {
                    return;
                }

                QString fileName = QFileInfo(filePath).fileName();
                QString newDataFileName = files.first().saveFileName(newFileName, -1, movie->files().count() > 1);
                Helper::instance()->sanitizeFileName(newDataFileName);

                if (newDataFileName == fileName) {
                    // File already has correct name
                    FilmFiles.append(fileName);
                    return;
                }

                int row = addResult(fileName, newDataFileName, RenameOperation::Rename);
                if (config.dryRun) {
                    FilmFiles.append(newDataFileName);
                    return;
                }

                if (rename(filePath, fiCanonicalPath + "/" + newDataFileName)) {
                    FilmFiles.append(newDataFileName);
                } else {
                    setResultStatus(row, RenameResult::Failed);
                }
            };

            MediaCenterInterface *mediaCenter = Manager::instance()->mediaCenterInterface();
            const auto renameImageType = [&](ImageType imageType) {
                DataFileType fileType = DataFile::dataFileTypeForImageType(imageType);
                renameFileType(mediaCenter->imageFileName(movie, imageType), fileType);
            };

            renameFileType(mediaCenter->nfoFilePath(movie), DataFileType::MovieNfo);

            renameImageType(ImageType::MoviePoster);
            renameImageType(ImageType::MovieBackdrop);
            renameImageType(ImageType::MovieBanner);
            renameImageType(ImageType::MovieThumb);
            renameImageType(ImageType::MovieLogo);
            renameImageType(ImageType::MovieClearArt);
            renameImageType(ImageType::MovieCdArt);
        }

        int renameRow = -1;
        QString newMovieFolder = dir.path();
        QString extension = (!movie->files().isEmpty()) ? QFileInfo(movie->files().first()).suffix() : "";
        const auto videoDetails = movie->streamDetails()->videoDetails();
        // rename dir for already existe films dir
        if (config.renameDirectories && movie->inSeparateFolder()) {
            Renamer::replace(newFolderName, "title", movie->name());
            Renamer::replace(newFolderName, "extension", extension);
            Renamer::replace(newFolderName, "originalTitle", movie->originalName());
            Renamer::replace(newFolderName, "sortTitle", movie->sortTitle());
            Renamer::replace(newFolderName, "year", movie->released().toString("yyyy"));
            Renamer::replace(newFolderName, "videoCodec", movie->streamDetails()->videoCodec());
            Renamer::replace(newFolderName, "audioCodec", movie->streamDetails()->audioCodec());
            Renamer::replace(newFolderName, "channels", QString::number(movie->streamDetails()->audioChannels()));
            Renamer::replace(newFolderName,
                "resolution",
                Helper::instance()->matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::ScanType)));
            Renamer::replaceCondition(newFolderName, "bluray", isBluRay);
            Renamer::replaceCondition(newFolderName, "dvd", isDvd);
            Renamer::replaceCondition(
                newFolderName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
            Renamer::replaceCondition(newFolderName, "movieset", movie->set());
            Renamer::replaceCondition(newFolderName, "imdbId", movie->id());
            Helper::instance()->sanitizeFileName(newFolderName);
            if (dir.dirName() != newFolderName) {
                renameRow = addResult(dir.dirName(), newFolderName, RenameOperation::Rename);
            }
        }
        // create dir for new dir structure
        else if (config.renameDirectories) {
            Renamer::replace(newFolderName, "title", movie->name());
            Renamer::replace(newFolderName, "extension", extension);
            Renamer::replace(newFolderName, "originalTitle", movie->originalName());
            Renamer::replace(newFolderName, "sortTitle", movie->sortTitle());
            Renamer::replace(newFolderName, "year", movie->released().toString("yyyy"));
            Renamer::replace(newFolderName, "videoCodec", movie->streamDetails()->videoCodec());
            Renamer::replace(newFolderName, "audioCodec", movie->streamDetails()->audioCodec());
            Renamer::replace(newFolderName, "channels", QString::number(movie->streamDetails()->audioChannels()));
            Renamer::replace(newFolderName,
                "resolution",
                Helper::instance()->matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::ScanType)));
            Renamer::replaceCondition(newFolderName, "bluray", isBluRay);
            Renamer::replaceCondition(newFolderName, "dvd", isDvd);
            Renamer::replaceCondition(
                newFolderName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
            Renamer::replaceCondition(newFolderName, "movieset", movie->set());
            Renamer::replaceCondition(newFolderName, "imdbId", movie->id());
            Helper::instance()->sanitizeFileName(newFolderName);

            if (dir.dirName() != newFolderName) { // check if movie is not already on good folder
                int i = 0;
                while (dir.exists(newFolderName)) {
                    newFolderName = newFolderName + " " + QString::number(++i);
                }

                int row = addResult(dir.dirName(), newFolderName, RenameOperation::CreateDir);

                if (!config.dryRun) {
                    if (!dir.mkdir(newFolderName)) {
                        setResultStatus(row, RenameResult::Failed);
                        errorOccured = true;
                        continue;
                    } else {
                        newMovieFolder = dir.path() + "/" + newFolderName;
                    }
                }

                foreach (const QString &fileName, FilmFiles) {
                    QFileInfo fi(fileName);
                    if (dir.dirName() != newFolderName) {
                        int row = addResult(fi.fileName(),
                            dir.dirName() + "/" + newFolderName + "/" + fi.fileName(),
                            RenameOperation::Move);
                        ui->results->append(tr(R"(<b>Move File</b> "%1" to "%2")")
                                                .arg(fi.fileName())
                                                .arg(dir.dirName() + "/" + newFolderName + "/" + fi.fileName()));
                        if (!config.dryRun) {
                            if (!rename(dir.absolutePath() + "/" + fileName,
                                    dir.absolutePath() + "/" + newFolderName + "/" + fi.fileName())) {
                                setResultStatus(row, RenameResult::Failed);
                            }
                        }
                    }
                }
            }
        }


        if (!config.dryRun && dir.dirName() != newFolderName && config.renameDirectories && movie->inSeparateFolder()) {
            QDir parentDir(dir.path());
            parentDir.cdUp();
            if (!rename(dir, parentDir.path() + "/" + newFolderName)) {
                setResultStatus(renameRow, RenameResult::Failed);
                errorOccured = true;
            } else {
                newMovieFolder = parentDir.path() + "/" + newFolderName;
            }
        }

        if (errorOccured) {
            m_renameErrorOccured = true;
        }

        if (!errorOccured && !config.dryRun) {
            QStringList files;
            foreach (const QString &file, newMovieFiles) {
                QString f = newMovieFolder;
                if (isBluRay || isDvd) {
                    f += "/" + parentDirName;
                }
                f += "/" + file;
                files << f;
            }
            movie->setFiles(files);
            Manager::instance()->database()->update(movie);
        }
    }
}

void Renamer::renameEpisodes(QList<TvShowEpisode *> episodes, const RenamerConfig &config)
{
    if (config.renameFiles && config.filePattern.isEmpty()) {
        return;
    }

    const QString &seasonPattern = config.directoryPattern;
    const bool useSeasonDirectories = config.renameDirectories;

    QList<TvShowEpisode *> episodesRenamed;

    for (TvShowEpisode *episode : episodes) {
        if (episode->files().isEmpty() || (episode->files().count() > 1 && config.filePatternMulti.isEmpty())
            || episodesRenamed.contains(episode)) {
            continue;
        }

        if (episode->hasChanged()) {
            ui->results->append(tr("<b>Episode</b> \"%1\" has been edited but is not saved").arg(episode->name()));
            continue;
        }

        QList<TvShowEpisode *> multiEpisodes;
        for (TvShowEpisode *subEpisode : episode->tvShow()->episodes()) {
            if (subEpisode->files() == episode->files()) {
                multiEpisodes.append(subEpisode);
                episodesRenamed.append(subEpisode);
            }
        }

        const QString &firstEpisode = episode->files().first();
        const bool isBluRay = Helper::instance()->isBluRay(firstEpisode);
        const bool isDvd = Helper::instance()->isDvd(firstEpisode);
        const bool isDvdWithoutSub = Helper::instance()->isDvd(firstEpisode, true);

        QFileInfo fi(episode->files().first());
        QString fiCanonicalPath = fi.canonicalPath();
        QStringList episodeFiles = episode->files();
        QString nfo = Manager::instance()->mediaCenterInterface()->nfoFilePath(episode);
        QString newNfoFileName = nfo;
        QString thumbnail =
            Manager::instance()->mediaCenterInterface()->imageFileName(episode, ImageType::TvShowEpisodeThumb);
        QString newThumbnailFileName = thumbnail;
        QStringList newEpisodeFiles;

        for (const QString &file : episode->files()) {
            QFileInfo fi(file);
            newEpisodeFiles << fi.fileName();
        }


        if (!isBluRay && !isDvd && !isDvdWithoutSub && config.renameFiles) {
            qApp->processEvents();
            QString newFileName;
            episodeFiles.clear();

            newEpisodeFiles.clear();
            int partNo = 0;
            const auto videoDetails = episode->streamDetails()->videoDetails();
            foreach (const QString &file, episode->files()) {
                newFileName = (episode->files().count() == 1) ? config.filePattern : config.filePatternMulti;
                QFileInfo fi(file);
                QString baseName = fi.completeBaseName();
                QDir currentDir = fi.dir();
                Renamer::replace(newFileName, "title", episode->name());
                Renamer::replace(newFileName, "showTitle", episode->showTitle());
                Renamer::replace(newFileName, "year", episode->firstAired().toString("yyyy"));
                Renamer::replace(newFileName, "extension", fi.suffix());
                Renamer::replace(newFileName, "season", episode->seasonString());
                Renamer::replace(newFileName, "partNo", QString::number(++partNo));
                Renamer::replace(newFileName, "videoCodec", episode->streamDetails()->videoCodec());
                Renamer::replace(newFileName, "audioCodec", episode->streamDetails()->audioCodec());
                Renamer::replace(newFileName, "channels", QString::number(episode->streamDetails()->audioChannels()));
                Renamer::replace(newFileName,
                    "resolution",
                    Helper::instance()->matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                        videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                        videoDetails.value(StreamDetails::VideoDetails::ScanType)));
                Renamer::replaceCondition(
                    newFileName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");

                if (multiEpisodes.count() > 1) {
                    QStringList episodeStrings;
                    foreach (TvShowEpisode *subEpisode, multiEpisodes)
                        episodeStrings.append(subEpisode->episodeString());
                    qSort(episodeStrings);
                    Renamer::replace(newFileName, "episode", episodeStrings.join("-"));
                } else {
                    Renamer::replace(newFileName, "episode", episode->episodeString());
                }

                Helper::instance()->sanitizeFileName(newFileName);
                if (fi.fileName() != newFileName) {
                    int row = addResult(fi.fileName(), newFileName, RenameOperation::Rename);
                    if (!config.dryRun) {
                        if (!rename(file, fi.canonicalPath() + "/" + newFileName)) {
                            setResultStatus(row, RenameResult::Failed);
                            m_renameErrorOccured = true;
                        } else {
                            newEpisodeFiles << newFileName;
                        }
                    }

                    QStringList filters;
                    foreach (const QString &extra, m_extraFiles)
                        filters << baseName + extra;
                    foreach (
                        const QString &subFileName, currentDir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot)) {
                        QString subSuffix = subFileName.mid(baseName.length());
                        QString newBaseName = newFileName.left(newFileName.lastIndexOf("."));
                        QString newSubName = newBaseName + subSuffix;
                        int row = addResult(subFileName, newSubName, RenameOperation::Rename);
                        if (!config.dryRun) {
                            if (!rename(currentDir.canonicalPath() + "/" + subFileName,
                                    currentDir.canonicalPath() + "/" + newSubName)) {
                                setResultStatus(row, RenameResult::Failed);
                                m_renameErrorOccured = true;
                            }
                        }
                    }
                } else {
                    newEpisodeFiles << fi.fileName();
                }
                episodeFiles << fi.canonicalPath() + "/" + newFileName;
            }

            // Rename nfo
            if (!nfo.isEmpty()) {
                QString nfoFileName = QFileInfo(nfo).fileName();
                QList<DataFile> nfoFiles = Settings::instance()->dataFiles(DataFileType::TvShowEpisodeNfo);
                if (!nfoFiles.isEmpty()) {
                    newNfoFileName = nfoFiles.first().saveFileName(newFileName);
                    Helper::instance()->sanitizeFileName(newNfoFileName);
                    if (newNfoFileName != nfoFileName) {
                        int row = addResult(nfoFileName, newNfoFileName, RenameOperation::Rename);
                        if (!config.dryRun) {
                            if (!rename(nfo, fiCanonicalPath + "/" + newNfoFileName)) {
                                setResultStatus(row, RenameResult::Failed);
                            }
                        }
                    }
                }
            }

            // Rename Thumbnail
            if (!thumbnail.isEmpty()) {
                QString thumbnailFileName = QFileInfo(thumbnail).fileName();
                QList<DataFile> thumbnailFiles = Settings::instance()->dataFiles(DataFileType::TvShowEpisodeThumb);
                if (!thumbnailFiles.isEmpty()) {
                    newThumbnailFileName =
                        thumbnailFiles.first().saveFileName(newFileName, -1, episode->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newThumbnailFileName);
                    if (newThumbnailFileName != thumbnailFileName) {
                        int row = addResult(thumbnailFileName, newThumbnailFileName, RenameOperation::Rename);
                        if (!config.dryRun) {
                            if (!rename(thumbnail, fiCanonicalPath + "/" + newThumbnailFileName)) {
                                setResultStatus(row, RenameResult::Failed);
                            }
                        }
                    }
                }
            }
        }

        if (!config.dryRun) {
            QStringList files;
            for (const QString &file : newEpisodeFiles) {
                files << fi.path() + "/" + file;
            }
            episode->setFiles(files);
            Manager::instance()->database()->update(episode);
        }

        if (useSeasonDirectories) {
            QDir showDir(episode->tvShow()->dir());
            QString seasonDirName = seasonPattern;
            Renamer::replace(seasonDirName, "season", episode->seasonString());
            Renamer::replace(seasonDirName, "showTitle", episode->showTitle());
            Helper::instance()->sanitizeFileName(seasonDirName);
            QDir seasonDir(showDir.path() + "/" + seasonDirName);
            if (!seasonDir.exists()) {
                int row = addResult(seasonDirName, "", RenameOperation::CreateDir);
                if (!config.dryRun) {
                    if (!showDir.mkdir(seasonDirName)) {
                        setResultStatus(row, RenameResult::Failed);
                    }
                }
            }

            if (isBluRay || isDvd || isDvdWithoutSub) {
                QDir dir = fi.dir();
                if (isDvd || isBluRay) {
                    dir.cdUp();
                }

                QDir parentDir = dir;
                parentDir.cdUp();
                if (parentDir != seasonDir) {
                    int row = addResult(dir.dirName(), seasonDirName, RenameOperation::Move);
                    if (!config.dryRun) {
                        if (!rename(dir, seasonDir.absolutePath() + "/" + dir.dirName())) {
                            setResultStatus(row, RenameResult::Failed);
                            m_renameErrorOccured = true;
                        } else {
                            newEpisodeFiles.clear();
                            QString oldDir = dir.path();
                            QString newDir = seasonDir.absolutePath() + "/" + dir.dirName();
                            foreach (const QString &file, episode->files())
                                newEpisodeFiles << newDir + file.mid(oldDir.length());
                            episode->setFiles(newEpisodeFiles);
                            Manager::instance()->database()->update(episode);
                        }
                    }
                }
            } else if (fi.dir() != seasonDir) {
                newEpisodeFiles.clear();
                foreach (const QString &fileName, episode->files()) {
                    QFileInfo fi(fileName);
                    int row = addResult(fi.fileName(), seasonDirName, RenameOperation::Move);
                    if (!config.dryRun) {
                        if (!rename(fileName, seasonDir.path() + "/" + fi.fileName())) {
                            setResultStatus(row, RenameResult::Failed);
                            m_renameErrorOccured = true;
                        } else {
                            newEpisodeFiles << seasonDir.path() + "/" + fi.fileName();
                        }
                    }
                }
                if (!config.dryRun) {
                    episode->setFiles(newEpisodeFiles);
                    Manager::instance()->database()->update(episode);
                }

                if (!newNfoFileName.isEmpty() && !nfo.isEmpty()) {
                    int row = addResult(newNfoFileName, seasonDirName, RenameOperation::Move);
                    if (!config.dryRun) {
                        if (!rename(fiCanonicalPath + "/" + newNfoFileName, seasonDir.path() + "/" + newNfoFileName)) {
                            setResultStatus(row, RenameResult::Failed);
                        }
                    }
                }
                if (!thumbnail.isEmpty() && !newThumbnailFileName.isEmpty()) {
                    int row = addResult(newThumbnailFileName, seasonDirName, RenameOperation::Move);
                    if (!config.dryRun) {
                        if (!rename(fiCanonicalPath + "/" + newThumbnailFileName,
                                seasonDir.path() + "/" + newThumbnailFileName)) {
                            setResultStatus(row, RenameResult::Failed);
                        }
                    }
                }
            }
        }
    }
}

void Renamer::renameShows(QList<TvShow *> shows,
    const QString &directoryPattern,
    const bool &renameDirectories,
    const bool &dryRun)
{
    if ((renameDirectories && directoryPattern.isEmpty()) || !renameDirectories) {
        return;
    }

    for (TvShow *show : shows) {
        if (show->hasChanged()) {
            ui->results->append(tr("<b>TV Show</b> \"%1\" has been edited but is not saved").arg(show->name()));
            continue;
        }

        QDir dir(show->dir());
        QString newFolderName = directoryPattern;
        Renamer::replace(newFolderName, "title", show->name());
        Renamer::replace(newFolderName, "showTitle", show->name());
        Renamer::replace(newFolderName, "year", show->firstAired().toString("yyyy"));
        Helper::instance()->sanitizeFileName(newFolderName);
        if (newFolderName != dir.dirName()) {
            int row = addResult(dir.dirName(), newFolderName, RenameOperation::Rename);
            QDir parentDir(dir.path());
            parentDir.cdUp();
            if (!dryRun) {
                if (!rename(dir, parentDir.path() + "/" + newFolderName)) {
                    setResultStatus(row, RenameResult::Failed);
                    m_renameErrorOccured = true;
                } else {
                    QString newShowDir = parentDir.path() + "/" + newFolderName;
                    QString oldShowDir = show->dir();
                    show->setDir(newShowDir);
                    Manager::instance()->database()->update(show);
                    for (TvShowEpisode *episode : show->episodes()) {
                        QStringList files;
                        foreach (const QString &file, episode->files())
                            files << newShowDir + file.mid(oldShowDir.length());
                        episode->setFiles(files);
                        Manager::instance()->database()->update(episode);
                    }
                }
            }
        }
    }
}

void Renamer::renameConcerts(QList<Concert *> concerts, const RenamerConfig &config)
{
    if ((config.renameFiles && config.filePattern.isEmpty())
        || (config.renameDirectories && config.directoryPattern.isEmpty())) {
        return;
    }

    for (Concert *concert : concerts) {
        if (concert->files().isEmpty() || (concert->files().count() > 1 && config.filePatternMulti.isEmpty())) {
            continue;
        }

        if (concert->hasChanged()) {
            ui->results->append(tr("<b>Concert</b> \"%1\" has been edited but is not saved").arg(concert->name()));
            continue;
        }

        qApp->processEvents();
        QFileInfo fi(concert->files().first());
        QString fiCanonicalPath = fi.canonicalPath();
        QDir dir(fi.canonicalPath());
        QString newFolderName = config.directoryPattern;
        QString newFileName;
        QStringList newConcertFiles;
        QString parentDirName;

        bool errorOccured = false;

        foreach (const QString &file, concert->files()) {
            QFileInfo fi(file);
            newConcertFiles.append(fi.fileName());
        }

        QDir chkDir(fi.canonicalPath());
        chkDir.cdUp();

        bool isBluRay = Helper::instance()->isBluRay(chkDir.path());
        bool isDvd = Helper::instance()->isDvd(chkDir.path());

        if (isBluRay || isDvd) {
            parentDirName = dir.dirName();
            dir.cdUp();
        }

        if (!isBluRay && !isDvd && config.renameFiles) {
            newConcertFiles.clear();
            int partNo = 0;
            const auto videoDetails = concert->streamDetails()->videoDetails();
            for (const QString &file : concert->files()) {
                newFileName = (concert->files().count() == 1) ? config.filePattern : config.filePatternMulti;
                QFileInfo fi(file);
                QString baseName = fi.completeBaseName();
                QDir currentDir = fi.dir();

                Renamer::replace(newFileName, "title", concert->name());
                Renamer::replace(newFileName, "artist", concert->artist());
                Renamer::replace(newFileName, "album", concert->album());
                Renamer::replace(newFileName, "year", concert->released().toString("yyyy"));
                Renamer::replace(newFileName, "extension", fi.suffix());
                Renamer::replace(newFileName, "partNo", QString::number(++partNo));
                Renamer::replace(newFileName, "videoCodec", concert->streamDetails()->videoCodec());
                Renamer::replace(newFileName, "audioCodec", concert->streamDetails()->audioCodec());
                Renamer::replace(newFileName, "channels", QString::number(concert->streamDetails()->audioChannels()));
                Renamer::replace(newFileName,
                    "resolution",
                    Helper::instance()->matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                        videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                        videoDetails.value(StreamDetails::VideoDetails::ScanType)));
                Renamer::replaceCondition(
                    newFileName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
                Helper::instance()->sanitizeFileName(newFileName);
                if (fi.fileName() != newFileName) {
                    int row = addResult(fi.fileName(), newFileName, RenameOperation::Rename);
                    if (!config.dryRun) {
                        if (!rename(file, fi.canonicalPath() + "/" + newFileName)) {
                            setResultStatus(row, RenameResult::Failed);
                            errorOccured = true;
                            continue;
                        } else {
                            newConcertFiles.append(newFileName);
                        }
                    }

                    QStringList filters;
                    foreach (const QString &extra, m_extraFiles)
                        filters << baseName + extra;
                    foreach (
                        const QString &subFileName, currentDir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot)) {
                        QString subSuffix = subFileName.mid(baseName.length());
                        QString newBaseName = newFileName.left(newFileName.lastIndexOf("."));
                        QString newSubName = newBaseName + subSuffix;
                        int row = addResult(subFileName, newSubName, RenameOperation::Rename);
                        if (!config.dryRun) {
                            if (!rename(currentDir.canonicalPath() + "/" + subFileName,
                                    currentDir.canonicalPath() + "/" + newSubName)) {
                                setResultStatus(row, RenameResult::Failed);
                            }
                        }
                    }
                } else {
                    newConcertFiles.append(fi.fileName());
                }
            }

            const auto renameFileType = [&](QString filePath, DataFileType dataFileType) -> void {
                if (filePath.isEmpty()) {
                    // File does not exist, e.g. there is no poster.
                    return;
                }

                QList<DataFile> files = Settings::instance()->dataFiles(dataFileType);
                if (files.isEmpty()) {
                    return;
                }

                QString fileName = QFileInfo(filePath).fileName();
                QString newDataFileName = files.first().saveFileName(newFileName, -1, concert->files().count() > 1);
                Helper::instance()->sanitizeFileName(newDataFileName);
                if (newDataFileName == fileName) {
                    // File already has correct name
                    return;
                }

                int row = addResult(fileName, newDataFileName, RenameOperation::Rename);
                if (config.dryRun) {
                    return;
                }

                if (!rename(filePath, fiCanonicalPath + "/" + newDataFileName)) {
                    setResultStatus(row, RenameResult::Failed);
                }
            };

            MediaCenterInterface *mediaCenter = Manager::instance()->mediaCenterInterface();
            const auto renameImageType = [&](ImageType imageType) {
                DataFileType fileType = DataFile::dataFileTypeForImageType(imageType);
                renameFileType(mediaCenter->imageFileName(concert, imageType), fileType);
            };

            renameFileType(mediaCenter->nfoFilePath(concert), DataFileType::ConcertNfo);

            renameImageType(ImageType::ConcertPoster);
            renameImageType(ImageType::ConcertBackdrop);
        }

        int renameRow = -1;
        if (config.renameDirectories && concert->inSeparateFolder()) {
            const auto videoDetails = concert->streamDetails()->videoDetails();
            Renamer::replace(newFolderName, "title", concert->name());
            Renamer::replace(newFolderName, "artist", concert->artist());
            Renamer::replace(newFolderName, "album", concert->album());
            Renamer::replace(newFolderName, "year", concert->released().toString("yyyy"));
            Renamer::replaceCondition(newFolderName, "bluray", isBluRay);
            Renamer::replaceCondition(newFolderName, "dvd", isDvd);
            Renamer::replaceCondition(
                newFolderName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
            Renamer::replace(newFolderName, "videoCodec", concert->streamDetails()->videoCodec());
            Renamer::replace(newFolderName, "audioCodec", concert->streamDetails()->audioCodec());
            Renamer::replace(newFolderName, "channels", QString::number(concert->streamDetails()->audioChannels()));
            Renamer::replace(newFolderName,
                "resolution",
                Helper::instance()->matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::ScanType)));
            Helper::instance()->sanitizeFileName(newFolderName);
            if (dir.dirName() != newFolderName) {
                renameRow = addResult(dir.dirName(), newFolderName, RenameOperation::Rename);
            }
        }

        QString newConcertFolder = dir.path();
        if (!config.dryRun && dir.dirName() != newFolderName && config.renameDirectories
            && concert->inSeparateFolder()) {
            QDir parentDir(dir.path());
            parentDir.cdUp();
            if (!rename(dir, parentDir.path() + "/" + newFolderName)) {
                setResultStatus(renameRow, RenameResult::Failed);
                errorOccured = true;
            } else {
                newConcertFolder = parentDir.path() + "/" + newFolderName;
            }
        }

        if (errorOccured) {
            m_renameErrorOccured = true;
        }

        if (!errorOccured && !config.dryRun) {
            QStringList files;
            for (const QString &file : newConcertFiles) {
                QString f = newConcertFolder;
                if (isBluRay || isDvd) {
                    f += "/" + parentDirName;
                }
                f += "/" + file;
                files << f;
            }
            concert->setFiles(files);
            Manager::instance()->database()->update(concert);
        }
    }
}

bool Renamer::rename(const QString &file, const QString &newName)
{
    QFile f(file);
    if (!f.exists()) {
        return false;
    }

    QFile newFile(newName);
    if (newFile.exists() && QString::compare(file, newName, Qt::CaseInsensitive) != 0) {
        return false;
    }

    if (newFile.exists()) {
        if (!f.rename(newName + ".tmp")) {
            return false;
        }
        return f.rename(newName);
    } else {
        return f.rename(newName);
    }
}

bool Renamer::rename(QDir &dir, QString newName)
{
    if (QString::compare(dir.path(), newName, Qt::CaseInsensitive) == 0) {
        QDir tmpDir;
        if (!tmpDir.rename(dir.path(), dir.path() + "tmp")) {
            return false;
        }
        return tmpDir.rename(dir.path() + "tmp", newName);
    } else {
        QDir tmpDir;
        return tmpDir.rename(dir.path(), newName);
    }
}

QString Renamer::replace(QString &text, const QString &search, const QString &replace)
{
    text.replace("<" + search + ">", replace);
    return text;
}

QString Renamer::replaceCondition(QString &text, const QString &condition, const QString &replace)
{
    QRegExp rx("\\{" + condition + "\\}(.*)\\{/" + condition + "\\}");
    rx.setMinimal(true);
    if (rx.indexIn(text) == -1) {
        return Renamer::replace(text, condition, replace);
    }

    QString search = QString("{%1}%2{/%1}").arg(condition).arg(rx.cap(1));
    text.replace(search, !replace.isEmpty() ? rx.cap(1) : "");
    return Renamer::replace(text, condition, replace);
}

QString Renamer::replaceCondition(QString &text, const QString &condition, bool hasCondition)
{
    QRegExp rx("\\{" + condition + "\\}(.*)\\{/" + condition + "\\}");
    rx.setMinimal(true);
    if (rx.indexIn(text) == -1) {
        return text;
    }

    QString search = QString("{%1}%2{/%1}").arg(condition).arg(rx.cap(1));
    text.replace(search, hasCondition ? rx.cap(1) : "");
    return text;
}

QString Renamer::typeToString(Renamer::RenameType type)
{
    switch (type) {
    case Renamer::RenameType::All: return "All";
    case Renamer::RenameType::Movies: return "Movies";
    case Renamer::RenameType::Concerts: return "Concerts";
    case Renamer::RenameType::TvShows: return "TvShows";
    }
    qWarning() << "Unknown RenamerType";
    return "unknown";
}

int Renamer::addResult(const QString &oldFileName, const QString &newFileName, RenameOperation operation)
{
    QString opString;
    switch (operation) {
    case RenameOperation::CreateDir: opString = tr("Create dir"); break;
    case RenameOperation::Move: opString = tr("Move"); break;
    case RenameOperation::Rename: opString = tr("Rename"); break;
    }

    QFont font = ui->resultsTable->font();
    font.setBold(true);

    int row = ui->resultsTable->rowCount();
    ui->resultsTable->insertRow(row);
    ui->resultsTable->setItem(row, 0, new QTableWidgetItem(opString));
    ui->resultsTable->setItem(row, 1, new QTableWidgetItem(oldFileName));
    ui->resultsTable->setItem(row, 2, new QTableWidgetItem(newFileName));
    ui->resultsTable->item(row, 0)->setFont(font);

    return row;
}

void Renamer::setResultStatus(int row, RenameResult result)
{
    for (int col = 0, n = ui->resultsTable->columnCount(); col < n; ++col) {
        if (result == RenameResult::Failed) {
            ui->resultsTable->item(row, col)->setBackgroundColor(QColor(242, 222, 222));
            ui->resultsTable->item(row, col)->setTextColor(QColor(0, 0, 0));
        }
    }
}
