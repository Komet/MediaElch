#include "Renamer.h"
#include "ui_Renamer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "globals/Helper.h"
#include "globals/Manager.h"

Renamer::Renamer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Renamer)
{
    ui->setupUi(this);

    ui->resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->resultsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
#ifdef Q_OS_MAC
    QFont font = ui->resultsTable->font();
    font.setPointSize(font.pointSize()-2);
    ui->resultsTable->setFont(font);
#endif

    connect(ui->chkDirectoryNaming, SIGNAL(stateChanged(int)), this, SLOT(onChkRenameDirectories()));
    connect(ui->chkFileNaming, SIGNAL(stateChanged(int)), this, SLOT(onChkRenameFiles()));
    connect(ui->chkSeasonDirectories, SIGNAL(stateChanged(int)), this, SLOT(onChkUseSeasonDirectories()));
    connect(ui->btnDryRun, SIGNAL(clicked()), this, SLOT(onDryRun()));
    connect(ui->btnRename, SIGNAL(clicked()), this, SLOT(onRename()));

    onChkRenameDirectories();
    onChkRenameFiles();

    m_extraFiles = Settings::instance()->advanced()->subtitleFilters();
    ui->helpLabel->setText(tr("Please see %1 for help and examples on how to use the renamer.")
                           .arg("<a href=\"http://community.kvibes.de/topic/show/renamer\">http://community.kvibes.de/topic/show/renamer</a>"));
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
    case TypeMovies:
        ui->infoLabel->setText(tr("%n Movie(s) will be renamed", "", m_movies.count()));
        break;
    case TypeConcerts:
        ui->infoLabel->setText(tr("%n Concert(s) will be renamed", "", m_concerts.count()));
        break;
    case TypeTvShows:
        ui->infoLabel->setText(tr("%n TV Show(s) and %1", "", m_shows.count()).arg(tr("%n Episode(s) will be renamed", "", m_episodes.count())));
        break;
    default:
        break;
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

    ui->chkSeasonDirectories->setVisible(m_renameType == TypeTvShows);
    ui->seasonNaming->setVisible(m_renameType == TypeTvShows);
    ui->labelSeasonDirectory->setVisible(m_renameType == TypeTvShows);

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

    Settings::instance()->setRenamePatterns(m_renameType, ui->fileNaming->text(), ui->fileNamingMulti->text(), ui->directoryNaming->text(), ui->seasonNaming->text());
    Settings::instance()->setRenamings(m_renameType, ui->chkFileNaming->isChecked(), ui->chkDirectoryNaming->isChecked(), ui->chkSeasonDirectories->isChecked());

    QDialog::reject();
    if (m_filesRenamed)
        QTimer::singleShot(0, this, SLOT(onRenamed()));
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
    if (m_renameType == TypeMovies) {
        renameMovies(m_movies, ui->fileNaming->text(), ui->fileNamingMulti->text(),
                     ui->directoryNaming->text(), ui->chkFileNaming->isChecked(), ui->chkDirectoryNaming->isChecked());
    } else if (m_renameType == TypeConcerts) {
        renameConcerts(m_concerts, ui->fileNaming->text(), ui->fileNamingMulti->text(),
                     ui->directoryNaming->text(), ui->chkFileNaming->isChecked(), ui->chkDirectoryNaming->isChecked());
    } else if (m_renameType == TypeTvShows) {
        renameEpisodes(m_episodes, ui->fileNaming->text(), ui->fileNamingMulti->text(), ui->seasonNaming->text(),
                       ui->chkFileNaming->isChecked(), ui->chkSeasonDirectories->isChecked());
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
    if (m_renameType == TypeMovies) {
        renameMovies(m_movies, ui->fileNaming->text(), ui->fileNamingMulti->text(),
                     ui->directoryNaming->text(), ui->chkFileNaming->isChecked(), ui->chkDirectoryNaming->isChecked(), true);
    } else if (m_renameType == TypeConcerts) {
        renameConcerts(m_concerts, ui->fileNaming->text(), ui->fileNamingMulti->text(),
                       ui->directoryNaming->text(), ui->chkFileNaming->isChecked(), ui->chkDirectoryNaming->isChecked(), true);
    } else if (m_renameType == TypeTvShows) {
        renameEpisodes(m_episodes, ui->fileNaming->text(), ui->fileNamingMulti->text(), ui->seasonNaming->text(),
                       ui->chkFileNaming->isChecked(), ui->chkSeasonDirectories->isChecked(), true);
        renameShows(m_shows, ui->directoryNaming->text(), ui->chkDirectoryNaming->isChecked(), true);
    }
    ui->results->append("<span style=\"color:#01a800;\"><b>" + tr("Finished") + "</b></span>");
}

void Renamer::renameMovies(QList<Movie*> movies, const QString &filePattern, const QString &filePatternMulti,
                           const QString &directoryPattern, const bool &renameFiles, const bool &renameDirectories, const bool &dryRun)
{
    if ((renameFiles && filePattern.isEmpty()) || (renameDirectories && directoryPattern.isEmpty()))
        return;

    foreach (Movie *movie, movies) {
        if (movie->files().isEmpty() || (movie->files().count() > 1 && filePatternMulti.isEmpty()))
            continue;

        if (movie->hasChanged()) {
            ui->results->append(tr("<b>Movie</b> \"%1\" has been edited but is not saved").arg(movie->name()));
            continue;
        }

        qApp->processEvents();
        QFileInfo fi(movie->files().first());
        QString fiCanonicalPath = fi.canonicalPath();
        QDir dir(fi.canonicalPath());
        QString newFolderName = directoryPattern;
        QString newFileName;
        QString nfo = Manager::instance()->mediaCenterInterface()->nfoFilePath(movie);
        QString poster = Manager::instance()->mediaCenterInterface()->imageFileName(movie, ImageType::MoviePoster);
        QString fanart = Manager::instance()->mediaCenterInterface()->imageFileName(movie, ImageType::MovieBackdrop);
        QString banner = Manager::instance()->mediaCenterInterface()->imageFileName(movie, ImageType::MovieBanner);
        QString thumb = Manager::instance()->mediaCenterInterface()->imageFileName(movie, ImageType::MovieThumb);
        QString logo = Manager::instance()->mediaCenterInterface()->imageFileName(movie, ImageType::MovieLogo);
        QString clearArt = Manager::instance()->mediaCenterInterface()->imageFileName(movie, ImageType::MovieClearArt);
        QString cdArt = Manager::instance()->mediaCenterInterface()->imageFileName(movie, ImageType::MovieCdArt);
        QStringList FilmFiles;
        QStringList newMovieFiles;
        QString parentDirName;
        bool errorOccured = false;

        foreach (const QString &file, movie->files()) {
            QFileInfo fi(file);
            newMovieFiles.append(fi.fileName());
        }

        QDir chkDir(fi.canonicalPath());
        chkDir.cdUp();

        bool isBluRay = Helper::instance()->isBluRay(chkDir.path());
        bool isDvd = Helper::instance()->isDvd(chkDir.path());

        if (isBluRay || isDvd) {
            parentDirName = dir.dirName();
            dir.cdUp();
        }

        if (!isBluRay && !isDvd && renameFiles) {
            newMovieFiles.clear();
            int partNo = 0;
            foreach (const QString &file, movie->files()) {
                newFileName = (movie->files().count() == 1) ? filePattern : filePatternMulti;
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
                Renamer::replace(newFileName, "resolution", Helper::instance()->matchResolution(movie->streamDetails()->videoDetails().value("width").toInt(),
                                                                                                movie->streamDetails()->videoDetails().value("height").toInt(),
                                                                                                movie->streamDetails()->videoDetails().value("scantype")));
                Renamer::replaceCondition(newFileName, "imdbId", movie->id());
                Renamer::replaceCondition(newFileName, "movieset", movie->set());
                Renamer::replaceCondition(newFileName, "3D", movie->streamDetails()->videoDetails().value("stereomode") != "");
                Helper::instance()->sanitizeFileName(newFileName);
                if (fi.fileName() != newFileName) {
                    int row = addResult(fi.fileName(), newFileName, OperationRename);
                    if (!dryRun) {
                        if (!rename(file, fi.canonicalPath() + "/" + newFileName)){
                            setResultStatus(row, RenameFailed);
                            errorOccured = true;
                            continue;
                        } else {
                            FilmFiles.append(newFileName);
                            newMovieFiles.append(newFileName);
                        }
                    } else {
                        FilmFiles.append(newFileName);
                    }

                    foreach (const QString &trailerFile, currentDir.entryList(QStringList() << fi.completeBaseName() + "-trailer.*", QDir::Files | QDir::NoDotAndDotDot)) {
                        QFileInfo trailer(fi.canonicalPath() + "/" + trailerFile);
                        QString newTrailerFileName = newFileName;
                        newTrailerFileName = newTrailerFileName.left(newTrailerFileName.lastIndexOf(".")) + "-trailer." + trailer.suffix();
                        if (trailer.fileName() != newTrailerFileName) {
                            int row = addResult(trailer.fileName(), newTrailerFileName, OperationRename);
                            if (!dryRun) {
                                if (!rename(fi.canonicalPath() + "/" + trailerFile, fi.canonicalPath() + "/" + newTrailerFileName))
                                    setResultStatus(row, RenameFailed);
                                else
                                    FilmFiles.append(newTrailerFileName);
                            }
                            else
                                FilmFiles.append(newTrailerFileName);
                        }
                        else
                            FilmFiles.append(trailer.fileName());
                    }

                    /*
                    QStringList filters;
                    foreach (const QString &extra, m_extraFiles)
                        filters << baseName + extra;
                    foreach (const QString &subFileName, currentDir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot)) {
                        QString subSuffix = subFileName.mid(baseName.length());
                        QString newBaseName = newFileName.left(newFileName.lastIndexOf("."));
                        QString newSubName = newBaseName + subSuffix;
                        ui->results->append(tr("<b>Rename File</b> \"%1\" to \"%2\"").arg(subFileName).arg(newSubName));
                        if (!dryRun) {
                            if (!rename(currentDir.canonicalPath() + "/" + subFileName, currentDir.canonicalPath() + "/" + newSubName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                            else
                                FilmFiles.append(newSubName);
                        }
                        else
                            FilmFiles.append(newSubName);
                    }
                    */

                    foreach (Subtitle *subtitle, movie->subtitles()) {
                        QString subFileName = QFileInfo(newFileName).completeBaseName();
                        if (!subtitle->language().isEmpty())
                            subFileName.append("." + subtitle->language());
                        if (subtitle->forced())
                            subFileName.append(".forced");

                        QStringList newSubFiles;
                        foreach (const QString &subFile, subtitle->files()) {
                            QFileInfo subFi(fi.canonicalPath() + "/" + subFile);
                            QString newSubFileName = subFileName + "." + subFi.suffix();
                            int row = addResult(subFile, newSubFileName, OperationRename);
                            if (!dryRun) {
                                if (!rename(fi.canonicalPath() + "/" + subFile, fi.canonicalPath() + "/" + newSubFileName)) {
                                    newSubFiles << subFile;
                                    setResultStatus(row, RenameFailed);
                                } else {
                                    newSubFiles << newSubFileName;
                                    FilmFiles.append(newSubFileName);
                                }
                            } else {
                                FilmFiles.append(newSubFileName);
                            }
                        }
                        if (!dryRun)
                            subtitle->setFiles(newSubFiles, false);
                    }
                } else {
                    FilmFiles.append(fi.fileName());
                    newMovieFiles.append(fi.fileName());
                }

            }

            // Rename nfo
            if (!nfo.isEmpty()) {
                QString nfoFileName = QFileInfo(nfo).fileName();
                QList<DataFile> nfoFiles = Settings::instance()->dataFiles(DataFileType::MovieNfo);
                if (!nfoFiles.isEmpty()) {
                    QString newNfoFileName = nfoFiles.first().saveFileName(newFileName, -1, movie->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newNfoFileName);
                    if (newNfoFileName != nfoFileName) {
                        int row = addResult(nfoFileName, newNfoFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(nfo, fiCanonicalPath + "/" + newNfoFileName))
                                setResultStatus(row, RenameFailed);
                            else
                                FilmFiles.append(newNfoFileName);
                        }
                        else
                            FilmFiles.append(newNfoFileName);
                    }
                    else
                        FilmFiles.append(nfoFileName);
                }
            }

            // Rename Poster
            if (!poster.isEmpty()) {
                QString posterFileName = QFileInfo(poster).fileName();
                QList<DataFile> posterFiles = Settings::instance()->dataFiles(DataFileType::MoviePoster);
                if (!posterFiles.isEmpty()) {
                    QString newPosterFileName = posterFiles.first().saveFileName(newFileName, -1, movie->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newPosterFileName);
                    if (newPosterFileName != posterFileName) {
                        int row = addResult(posterFileName, newPosterFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(poster, fiCanonicalPath + "/" + newPosterFileName))
                                setResultStatus(row, RenameFailed);
                            else
                                FilmFiles.append(newPosterFileName);
                        }
                        else
                            FilmFiles.append(newPosterFileName);
                    }
                    else
                        FilmFiles.append(posterFileName);
                }
            }

            // Rename Fanart
            if (!fanart.isEmpty()) {
                QString fanartFileName = QFileInfo(fanart).fileName();
                QList<DataFile> fanartFiles = Settings::instance()->dataFiles(DataFileType::MovieBackdrop);
                if (!fanartFiles.isEmpty()) {
                    QString newFanartFileName = fanartFiles.first().saveFileName(newFileName, -1, movie->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newFanartFileName);
                    if (newFanartFileName != fanartFileName) {
                        int row = addResult(fanartFileName, newFanartFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(fanart, fiCanonicalPath + "/" + newFanartFileName))
                                setResultStatus(row, RenameFailed);
                            else
                                FilmFiles.append(newFanartFileName);
                        }
                        else
                            FilmFiles.append(newFanartFileName);
                    }
                    else
                        FilmFiles.append(fanartFileName);
                }
            }

            // Rename Banner
            if (!banner.isEmpty()) {
                QString bannerFileName = QFileInfo(banner).fileName();
                QList<DataFile> bannerFiles = Settings::instance()->dataFiles(DataFileType::MovieBanner);
                if (!bannerFiles.isEmpty()) {
                    QString newBannerFileName = bannerFiles.first().saveFileName(newFileName, -1, movie->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newBannerFileName);
                    if (newBannerFileName != bannerFileName) {
                        int row = addResult(bannerFileName, newBannerFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(banner, fiCanonicalPath + "/" + newBannerFileName))
                                setResultStatus(row, RenameFailed);
                            else
                                FilmFiles.append(newBannerFileName);
                        }
                        else
                            FilmFiles.append(newBannerFileName);
                    }
                    else
                        FilmFiles.append(bannerFileName);
                }
            }

            // Rename Thumb
            if (!thumb.isEmpty()) {
                QString thumbFileName = QFileInfo(thumb).fileName();
                QList<DataFile> thumbFiles = Settings::instance()->dataFiles(DataFileType::MovieThumb);
                if (!thumbFiles.isEmpty()) {
                    QString newThumbFileName = thumbFiles.first().saveFileName(newFileName, -1, movie->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newThumbFileName);
                    if (newThumbFileName != thumbFileName) {
                        int row = addResult(thumbFileName, newThumbFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(thumb, fiCanonicalPath + "/" + newThumbFileName))
                                setResultStatus(row, RenameFailed);
                            else
                                FilmFiles.append(newThumbFileName);
                        }
                        else
                            FilmFiles.append(newThumbFileName);
                    }
                    else
                        FilmFiles.append(thumbFileName);
                }
            }

            // Rename Logo
            if (!logo.isEmpty()) {
                QString logoFileName = QFileInfo(logo).fileName();
                QList<DataFile> logoFiles = Settings::instance()->dataFiles(DataFileType::MovieLogo);
                if (!logoFiles.isEmpty()) {
                    QString newLogoFileName = logoFiles.first().saveFileName(newFileName, -1, movie->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newLogoFileName);
                    if (newLogoFileName != logoFileName) {
                        int row = addResult(logoFileName, newLogoFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(logo, fiCanonicalPath + "/" + newLogoFileName))
                                setResultStatus(row, RenameFailed);
                            else
                                FilmFiles.append(newLogoFileName);
                        }
                        else
                            FilmFiles.append(newLogoFileName);
                    }
                    else
                        FilmFiles.append(logoFileName);
                }
            }

            // Rename ClearArt
            if (!clearArt.isEmpty()) {
                QString clearArtFileName = QFileInfo(clearArt).fileName();
                QList<DataFile> clearArtFiles = Settings::instance()->dataFiles(DataFileType::MovieClearArt);
                if (!clearArtFiles.isEmpty()) {
                    QString newClearArtFileName = clearArtFiles.first().saveFileName(newFileName, -1, movie->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newClearArtFileName);
                    if (newClearArtFileName != clearArtFileName) {
                        int row = addResult(clearArtFileName, newClearArtFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(clearArt, fiCanonicalPath + "/" + newClearArtFileName))
                                setResultStatus(row, RenameFailed);
                            else
                                FilmFiles.append(newClearArtFileName);
                        }
                        else
                            FilmFiles.append(newClearArtFileName);
                    }
                    else
                        FilmFiles.append(clearArtFileName);
                }
            }

            // Rename CdArt
            if (!cdArt.isEmpty()) {
                QString cdArtFileName = QFileInfo(cdArt).fileName();
                QList<DataFile> cdArtFiles = Settings::instance()->dataFiles(DataFileType::MovieCdArt);
                if (!cdArtFiles.isEmpty()) {
                    QString newCdArtFileName = cdArtFiles.first().saveFileName(newFileName, -1, movie->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newCdArtFileName);
                    if (newCdArtFileName != cdArtFileName) {
                        int row = addResult(cdArtFileName, newCdArtFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(cdArt, fiCanonicalPath + "/" + newCdArtFileName))
                                setResultStatus(row, RenameFailed);
                            else
                                FilmFiles.append(newCdArtFileName);
                        }
                        else
                            FilmFiles.append(newCdArtFileName);
                    }
                    else
                        FilmFiles.append(cdArtFileName);
                }
            }
        }

        int renameRow = -1;
        QString newMovieFolder = dir.path();
        QString extension = (!movie->files().isEmpty()) ? QFileInfo(movie->files().first()).suffix() : "";
        //rename dir for already existe films dir
        if (renameDirectories && movie->inSeparateFolder()) {
            Renamer::replace(newFolderName, "title", movie->name());
            Renamer::replace(newFolderName, "extension", extension);
            Renamer::replace(newFolderName, "originalTitle", movie->originalName());
            Renamer::replace(newFolderName, "sortTitle", movie->sortTitle());
            Renamer::replace(newFolderName, "year", movie->released().toString("yyyy"));
            Renamer::replace(newFolderName, "videoCodec", movie->streamDetails()->videoCodec());
            Renamer::replace(newFolderName, "audioCodec", movie->streamDetails()->audioCodec());
            Renamer::replace(newFolderName, "channels", QString::number(movie->streamDetails()->audioChannels()));
            Renamer::replace(newFolderName, "resolution", Helper::instance()->matchResolution(movie->streamDetails()->videoDetails().value("width").toInt(),
                                                                                              movie->streamDetails()->videoDetails().value("height").toInt(),
                                                                                              movie->streamDetails()->videoDetails().value("scantype")));
            Renamer::replaceCondition(newFolderName, "bluray", isBluRay);
            Renamer::replaceCondition(newFolderName, "dvd", isDvd);
            Renamer::replaceCondition(newFolderName, "3D", movie->streamDetails()->videoDetails().value("stereomode") != "");
            Renamer::replaceCondition(newFolderName, "movieset", movie->set());
            Renamer::replaceCondition(newFolderName, "imdbId", movie->id());
            Helper::instance()->sanitizeFileName(newFolderName);
            if (dir.dirName() != newFolderName)
                renameRow = addResult(dir.dirName(), newFolderName, OperationRename);
        }
        //create dir for new dir structure
        else if (renameDirectories) {
            Renamer::replace(newFolderName, "title", movie->name());
            Renamer::replace(newFolderName, "extension", extension);
            Renamer::replace(newFolderName, "originalTitle", movie->originalName());
            Renamer::replace(newFolderName, "sortTitle", movie->sortTitle());
            Renamer::replace(newFolderName, "year", movie->released().toString("yyyy"));
            Renamer::replace(newFolderName, "videoCodec", movie->streamDetails()->videoCodec());
            Renamer::replace(newFolderName, "audioCodec", movie->streamDetails()->audioCodec());
            Renamer::replace(newFolderName, "channels", QString::number(movie->streamDetails()->audioChannels()));
            Renamer::replace(newFolderName, "resolution", Helper::instance()->matchResolution(movie->streamDetails()->videoDetails().value("width").toInt(),
                                                                                              movie->streamDetails()->videoDetails().value("height").toInt(),
                                                                                              movie->streamDetails()->videoDetails().value("scantype")));
            Renamer::replaceCondition(newFolderName, "bluray", isBluRay);
            Renamer::replaceCondition(newFolderName, "dvd", isDvd);
            Renamer::replaceCondition(newFolderName, "3D", movie->streamDetails()->videoDetails().value("stereomode") != "");
            Renamer::replaceCondition(newFolderName, "movieset", movie->set());
            Renamer::replaceCondition(newFolderName, "imdbId", movie->id());
            Helper::instance()->sanitizeFileName(newFolderName);

            if (dir.dirName() != newFolderName){ //check if movie is not already on good folder
                int i = 0 ;
                while (dir.exists(newFolderName)){
                    newFolderName = newFolderName + " " + QString::number(++i);
                }

                int row = addResult(dir.dirName(), newFolderName, OperationCreateDir);

                if (!dryRun) {
                    if (!dir.mkdir(newFolderName)){
                        setResultStatus(row, RenameFailed);
                        errorOccured = true;
                        continue;
                    } else {
                        newMovieFolder = dir.path() + "/" + newFolderName;
                    }
                }

                foreach (const QString &fileName, FilmFiles) {
                    QFileInfo fi(fileName);
                    if (dir.dirName() != newFolderName) {
                        int row = addResult(fi.fileName(), dir.dirName() + "/" + newFolderName + "/" + fi.fileName(), OperationMove);
                        ui->results->append(tr("<b>Move File</b> \"%1\" to \"%2\"").arg(fi.fileName()).arg(dir.dirName() + "/" + newFolderName + "/" + fi.fileName()));
                        if (!dryRun) {
                            if (!rename(dir.absolutePath() + "/" +fileName, dir.absolutePath() + "/" + newFolderName + "/" + fi.fileName()))
                                setResultStatus(row, RenameFailed);

                        }
                    }
                }
            }
        }


        if (!dryRun && dir.dirName() != newFolderName && renameDirectories && movie->inSeparateFolder()) {
            QDir parentDir(dir.path());
            parentDir.cdUp();
            if (!rename(dir, parentDir.path() + "/" + newFolderName)) {
                setResultStatus(renameRow, RenameFailed);
                errorOccured = true;
            } else {
                newMovieFolder = parentDir.path() + "/" + newFolderName;
            }
        }

        if (errorOccured)
            m_renameErrorOccured = true;

        if (!errorOccured && !dryRun) {
            QStringList files;
            foreach (const QString &file, newMovieFiles) {
                QString f = newMovieFolder;
                if (isBluRay || isDvd)
                    f += "/" + parentDirName;
                f += "/" + file;
                files << f;
            }
            movie->setFiles(files);
            Manager::instance()->database()->update(movie);
        }
    }
}

void Renamer::renameEpisodes(QList<TvShowEpisode *> episodes, const QString &filePattern, const QString &filePatternMulti, const QString &seasonPattern,
                             const bool &renameFiles, const bool &useSeasonDirectories, const bool &dryRun)
{
    if (renameFiles && filePattern.isEmpty())
        return;

    QList<TvShowEpisode*> episodesRenamed;

    foreach (TvShowEpisode *episode, episodes) {
        if (episode->files().isEmpty() || (episode->files().count() > 1 && filePatternMulti.isEmpty()) ||
                episodesRenamed.contains(episode))
            continue;

        if (episode->hasChanged()) {
            ui->results->append(tr("<b>Episode</b> \"%1\" has been edited but is not saved").arg(episode->name()));
            continue;
        }

        QList<TvShowEpisode*> multiEpisodes;
        foreach (TvShowEpisode *subEpisode, episode->tvShow()->episodes()) {
            if (subEpisode->files() == episode->files()) {
                multiEpisodes.append(subEpisode);
                episodesRenamed.append(subEpisode);
            }
        }

        bool isBluRay = Helper::instance()->isBluRay(episode->files().at(0));
        bool isDvd = Helper::instance()->isDvd(episode->files().at(0));
        bool isDvdWithoutSub = Helper::instance()->isDvd(episode->files().at(0), true);
        QFileInfo fi(episode->files().first());
        QString fiCanonicalPath = fi.canonicalPath();
        QStringList episodeFiles = episode->files();
        QString nfo = Manager::instance()->mediaCenterInterface()->nfoFilePath(episode);
        QString newNfoFileName = nfo;
        QString thumbnail = Manager::instance()->mediaCenterInterface()->imageFileName(episode, ImageType::TvShowEpisodeThumb);
        QString newThumbnailFileName = thumbnail;
        QStringList newEpisodeFiles;
        foreach (const QString &file, episode->files()) {
            QFileInfo fi(file);
            newEpisodeFiles << fi.fileName();
        }


        if (!isBluRay && !isDvd && !isDvdWithoutSub && renameFiles) {
            qApp->processEvents();
            QString newFileName;
            episodeFiles.clear();

            newEpisodeFiles.clear();
            int partNo = 0;
            foreach (const QString &file, episode->files()) {
                newFileName = (episode->files().count() == 1) ? filePattern : filePatternMulti;
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
                Renamer::replace(newFileName, "resolution", Helper::instance()->matchResolution(episode->streamDetails()->videoDetails().value("width").toInt(),
                                                                                                episode->streamDetails()->videoDetails().value("height").toInt(),
                                                                                                episode->streamDetails()->videoDetails().value("scantype")));
                Renamer::replaceCondition(newFileName, "3D", episode->streamDetails()->videoDetails().value("stereomode") != "");

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
                    int row = addResult(fi.fileName(), newFileName, OperationRename);
                    if (!dryRun) {
                        if (!rename(file, fi.canonicalPath() + "/" + newFileName)) {
                            setResultStatus(row, RenameFailed);
                            m_renameErrorOccured = true;
                        } else {
                            newEpisodeFiles << newFileName;
                        }
                    }

                    QStringList filters;
                    foreach (const QString &extra, m_extraFiles)
                        filters << baseName + extra;
                    foreach (const QString &subFileName, currentDir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot)) {
                        QString subSuffix = subFileName.mid(baseName.length());
                        QString newBaseName = newFileName.left(newFileName.lastIndexOf("."));
                        QString newSubName = newBaseName + subSuffix;
                        int row = addResult(subFileName, newSubName, OperationRename);
                        if (!dryRun) {
                            if (!rename(currentDir.canonicalPath() + "/" + subFileName, currentDir.canonicalPath() + "/" + newSubName)) {
                                setResultStatus(row, RenameFailed);
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
                        int row = addResult(nfoFileName, newNfoFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(nfo, fiCanonicalPath + "/" + newNfoFileName))
                                setResultStatus(row, RenameFailed);
                        }
                    }
                }
            }

            // Rename Thumbnail
            if (!thumbnail.isEmpty()) {
                QString thumbnailFileName = QFileInfo(thumbnail).fileName();
                QList<DataFile> thumbnailFiles = Settings::instance()->dataFiles(DataFileType::TvShowEpisodeThumb);
                if (!thumbnailFiles.isEmpty()) {
                    newThumbnailFileName = thumbnailFiles.first().saveFileName(newFileName, -1, episode->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newThumbnailFileName);
                    if (newThumbnailFileName != thumbnailFileName) {
                        int row = addResult(thumbnailFileName, newThumbnailFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(thumbnail, fiCanonicalPath + "/" + newThumbnailFileName))
                                setResultStatus(row, RenameFailed);
                        }
                    }
                }
            }
        }

        if (!dryRun) {
            QStringList files;
            foreach (const QString &file, newEpisodeFiles)
                files << fi.path() + "/" + file;
            episode->setFiles(files);
            Manager::instance()->database()->update(episode);
        }

        if (useSeasonDirectories) {
            QDir showDir(episode->tvShow()->dir());
            QString seasonDirName = seasonPattern;
            Renamer::replace(seasonDirName, "season", episode->seasonString());
            Helper::instance()->sanitizeFileName(seasonDirName);
            QDir seasonDir(showDir.path() + "/" + seasonDirName);
            if (!seasonDir.exists()) {
                int row = addResult(seasonDirName, "", OperationCreateDir);
                if (!dryRun) {
                    if (!showDir.mkdir(seasonDirName))
                        setResultStatus(row, RenameFailed);
                }
            }

            if (isBluRay || isDvd || isDvdWithoutSub) {
                QDir dir = fi.dir();
                if (isDvd || isBluRay)
                    dir.cdUp();

                QDir parentDir = dir;
                parentDir.cdUp();
                if (parentDir != seasonDir) {
                    int row = addResult(dir.dirName(), seasonDirName, OperationMove);
                    if (!dryRun) {
                        if (!rename(dir, seasonDir.absolutePath() + "/" + dir.dirName())) {
                            setResultStatus(row, RenameFailed);
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
                    int row = addResult(fi.fileName(), seasonDirName, OperationMove);
                    if (!dryRun) {
                        if (!rename(fileName, seasonDir.path() + "/" + fi.fileName())) {
                            setResultStatus(row, RenameFailed);
                            m_renameErrorOccured = true;
                        } else {
                            newEpisodeFiles << seasonDir.path() + "/" + fi.fileName();
                        }
                    }
                }
                if (!dryRun) {
                    episode->setFiles(newEpisodeFiles);
                    Manager::instance()->database()->update(episode);
                }

                if (!newNfoFileName.isEmpty() && !nfo.isEmpty()) {
                    int row = addResult(newNfoFileName, seasonDirName, OperationMove);
                    if (!dryRun) {
                        if (!rename(fiCanonicalPath + "/" + newNfoFileName, seasonDir.path() + "/" + newNfoFileName))
                            setResultStatus(row, RenameFailed);
                    }
                }
                if (!thumbnail.isEmpty() && !newThumbnailFileName.isEmpty()) {
                    int row = addResult(newThumbnailFileName, seasonDirName, OperationMove);
                    if (!dryRun) {
                        if (!rename(fiCanonicalPath + "/" + newThumbnailFileName, seasonDir.path() + "/" + newThumbnailFileName))
                            setResultStatus(row, RenameFailed);
                    }
                }
            }
        }
    }
}

void Renamer::renameShows(QList<TvShow *> shows, const QString &directoryPattern, const bool &renameDirectories, const bool &dryRun)
{
    if ((renameDirectories && directoryPattern.isEmpty()) || !renameDirectories)
        return;

    foreach (TvShow *show, shows) {
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
            int row = addResult(dir.dirName(), newFolderName, OperationRename);
            QDir parentDir(dir.path());
            parentDir.cdUp();
            if (!dryRun) {
                if (!rename(dir, parentDir.path() + "/" + newFolderName)) {
                    setResultStatus(row, RenameFailed);
                    m_renameErrorOccured = true;
                } else {
                    QString newShowDir = parentDir.path() + "/" + newFolderName;
                    QString oldShowDir = show->dir();
                    show->setDir(newShowDir);
                    Manager::instance()->database()->update(show);
                    foreach (TvShowEpisode *episode, show->episodes()) {
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

void Renamer::renameConcerts(QList<Concert*> concerts, const QString &filePattern, const QString &filePatternMulti,
                           const QString &directoryPattern, const bool &renameFiles, const bool &renameDirectories, const bool &dryRun)
{
    if ((renameFiles && filePattern.isEmpty()) || (renameDirectories && directoryPattern.isEmpty()))
        return;

    foreach (Concert *concert, concerts) {
        if (concert->files().isEmpty() || (concert->files().count() > 1 && filePatternMulti.isEmpty()))
            continue;

        if (concert->hasChanged()) {
            ui->results->append(tr("<b>Concert</b> \"%1\" has been edited but is not saved").arg(concert->name()));
            continue;
        }

        qApp->processEvents();
        QFileInfo fi(concert->files().first());
        QString fiCanonicalPath = fi.canonicalPath();
        QDir dir(fi.canonicalPath());
        QString newFolderName = directoryPattern;
        QString newFileName;
        QString nfo = Manager::instance()->mediaCenterInterface()->nfoFilePath(concert);
        QString poster = Manager::instance()->mediaCenterInterface()->imageFileName(concert, ImageType::ConcertPoster);
        QString fanart = Manager::instance()->mediaCenterInterface()->imageFileName(concert, ImageType::ConcertBackdrop);
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

        if (!isBluRay && !isDvd && renameFiles) {
            newConcertFiles.clear();
            int partNo = 0;
            foreach (const QString &file, concert->files()) {
                newFileName = (concert->files().count() == 1) ? filePattern : filePatternMulti;
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
                Renamer::replace(newFileName, "resolution", Helper::instance()->matchResolution(concert->streamDetails()->videoDetails().value("width").toInt(),
                                                                                                concert->streamDetails()->videoDetails().value("height").toInt(),
                                                                                                concert->streamDetails()->videoDetails().value("scantype")));
                Renamer::replaceCondition(newFileName, "3D", concert->streamDetails()->videoDetails().value("stereomode") != "");
                Helper::instance()->sanitizeFileName(newFileName);
                if (fi.fileName() != newFileName) {
                    int row = addResult(fi.fileName(), newFileName, OperationRename);
                    if (!dryRun) {
                        if (!rename(file, fi.canonicalPath() + "/" + newFileName)) {
                            setResultStatus(row, RenameFailed);
                            errorOccured = true;
                            continue;
                        } else {
                            newConcertFiles.append(newFileName);
                        }
                    }

                    QStringList filters;
                    foreach (const QString &extra, m_extraFiles)
                        filters << baseName + extra;
                    foreach (const QString &subFileName, currentDir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot)) {
                        QString subSuffix = subFileName.mid(baseName.length());
                        QString newBaseName = newFileName.left(newFileName.lastIndexOf("."));
                        QString newSubName = newBaseName + subSuffix;
                        int row = addResult(subFileName, newSubName, OperationRename);
                        if (!dryRun) {
                            if (!rename(currentDir.canonicalPath() + "/" + subFileName, currentDir.canonicalPath() + "/" + newSubName))
                                setResultStatus(row, RenameFailed);
                        }
                    }
                } else {
                    newConcertFiles.append(fi.fileName());
                }
            }

            // Rename nfo
            if (!nfo.isEmpty()) {
                QString nfoFileName = QFileInfo(nfo).fileName();
                QList<DataFile> nfoFiles = Settings::instance()->dataFiles(DataFileType::ConcertNfo);
                if (!nfoFiles.isEmpty()) {
                    QString newNfoFileName = nfoFiles.first().saveFileName(newFileName, -1, concert->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newNfoFileName);
                    if (newNfoFileName != nfoFileName) {
                        int row = addResult(nfoFileName, newNfoFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(nfo, fiCanonicalPath + "/" + newNfoFileName))
                                setResultStatus(row, RenameFailed);
                        }
                    }
                }
            }

            // Rename Poster
            if (!poster.isEmpty()) {
                QString posterFileName = QFileInfo(poster).fileName();
                QList<DataFile> posterFiles = Settings::instance()->dataFiles(DataFileType::ConcertPoster);
                if (!posterFiles.isEmpty()) {
                    QString newPosterFileName = posterFiles.first().saveFileName(newFileName, -1, concert->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newPosterFileName);
                    if (newPosterFileName != posterFileName) {
                        int row = addResult(posterFileName, newPosterFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(poster, fiCanonicalPath + "/" + newPosterFileName))
                                setResultStatus(row, RenameFailed);
                        }
                    }
                }
            }

            // Rename Fanart
            if (!fanart.isEmpty()) {
                QString fanartFileName = QFileInfo(fanart).fileName();
                QList<DataFile> fanartFiles = Settings::instance()->dataFiles(DataFileType::ConcertBackdrop);
                if (!fanartFiles.isEmpty()) {
                    QString newFanartFileName = fanartFiles.first().saveFileName(newFileName, -1, concert->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newFanartFileName);
                    if (newFanartFileName != fanartFileName) {
                        int row = addResult(fanartFileName, newFanartFileName, OperationRename);
                        if (!dryRun) {
                            if (!rename(fanart, fiCanonicalPath + "/" + newFanartFileName))
                                setResultStatus(row, RenameFailed);
                        }
                    }
                }
            }
        }

        int renameRow = -1;
        if (renameDirectories && concert->inSeparateFolder()) {
            Renamer::replace(newFolderName, "title", concert->name());
            Renamer::replace(newFolderName, "artist", concert->artist());
            Renamer::replace(newFolderName, "album", concert->album());
            Renamer::replace(newFolderName, "year", concert->released().toString("yyyy"));
            Renamer::replaceCondition(newFolderName, "bluray", isBluRay);
            Renamer::replaceCondition(newFolderName, "dvd", isDvd);
            Renamer::replaceCondition(newFolderName, "3D", concert->streamDetails()->videoDetails().value("stereomode") != "");
            Renamer::replace(newFolderName, "videoCodec", concert->streamDetails()->videoCodec());
            Renamer::replace(newFolderName, "audioCodec", concert->streamDetails()->audioCodec());
            Renamer::replace(newFolderName, "channels", QString::number(concert->streamDetails()->audioChannels()));
            Renamer::replace(newFolderName, "resolution", Helper::instance()->matchResolution(concert->streamDetails()->videoDetails().value("width").toInt(),
                                                                                              concert->streamDetails()->videoDetails().value("height").toInt(),
                                                                                              concert->streamDetails()->videoDetails().value("scantype")));
            Helper::instance()->sanitizeFileName(newFolderName);
            if (dir.dirName() != newFolderName)
                renameRow = addResult(dir.dirName(), newFolderName, OperationRename);
        }

        QString newConcertFolder = dir.path();
        if (!dryRun && dir.dirName() != newFolderName && renameDirectories && concert->inSeparateFolder()) {
            QDir parentDir(dir.path());
            parentDir.cdUp();
            if (!rename(dir, parentDir.path() + "/" + newFolderName)) {
                setResultStatus(renameRow, RenameFailed);
                errorOccured = true;
            } else {
                newConcertFolder = parentDir.path() + "/" + newFolderName;
            }
        }

        if (errorOccured)
            m_renameErrorOccured = true;

        if (!errorOccured && !dryRun) {
            QStringList files;
            foreach (const QString &file, newConcertFiles) {
                QString f = newConcertFolder;
                if (isBluRay || isDvd)
                    f += "/" + parentDirName;
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
    if (!f.exists())
        return false;

    QFile newFile(newName);
    if (newFile.exists() && QString::compare(file, newName, Qt::CaseInsensitive) != 0)
        return false;

    if (newFile.exists()) {
        if (!f.rename(newName + ".tmp"))
            return false;
        return f.rename(newName);
    } else {
        return f.rename(newName);
    }
}

bool Renamer::rename(QDir &dir, QString newName)
{
    if (QString::compare(dir.path(), newName, Qt::CaseInsensitive) == 0) {
        QDir tmpDir;
        if (!tmpDir.rename(dir.path(), dir.path() + "tmp"))
            return false;
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
    if (rx.indexIn(text) == -1)
        return Renamer::replace(text, condition, replace);

    QString search = QString("{%1}%2{/%1}").arg(condition).arg(rx.cap(1));
    text.replace(search, !replace.isEmpty() ? rx.cap(1) : "");
    return Renamer::replace(text, condition, replace);
}

QString Renamer::replaceCondition(QString &text, const QString &condition, bool hasCondition)
{
    QRegExp rx("\\{" + condition + "\\}(.*)\\{/" + condition + "\\}");
    rx.setMinimal(true);
    if (rx.indexIn(text) == -1)
        return text;

    QString search = QString("{%1}%2{/%1}").arg(condition).arg(rx.cap(1));
    text.replace(search, hasCondition ? rx.cap(1) : "");
    return text;
}

int Renamer::addResult(const QString &oldFileName, const QString &newFileName, RenameOperation operation)
{
    QString opString;
    switch (operation) {
    case OperationCreateDir: opString = tr("Create dir"); break;
    case OperationMove: opString = tr("Move"); break;
    case OperationRename: opString = tr("Rename"); break;
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
    for (int col=0, n=ui->resultsTable->columnCount() ; col<n ; ++col) {
        if (result == RenameFailed) {
            ui->resultsTable->item(row, col)->setBackgroundColor(QColor(242, 222, 222));
            ui->resultsTable->item(row, col)->setTextColor(QColor(0, 0, 0));
        }
    }
}
