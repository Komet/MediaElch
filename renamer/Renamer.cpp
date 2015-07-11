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
    ui->btnDryRun->setEnabled(true);
    ui->btnRename->setEnabled(true);
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
    ui->results->clear();
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
    ui->results->clear();
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
        if (movie->files().isEmpty() || (movie->files().count() > 1 && filePatternMulti.isEmpty()) || movie->hasChanged())
            continue;

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

        QDir chkDir(fi.canonicalPath());
        chkDir.cdUp();

        bool isBluRay = Helper::instance()->isBluRay(chkDir.path());
        bool isDvd = Helper::instance()->isDvd(chkDir.path());

        if (isBluRay || isDvd)
            dir.cdUp();

        if (!isBluRay && !isDvd && renameFiles) {
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
                Renamer::replace(newFileName, "resolution", Helper::instance()->matchResolution(movie->streamDetails()->videoDetails().value("width").toInt(),
                                                                                                movie->streamDetails()->videoDetails().value("height").toInt()));
                Renamer::replaceCondition(newFileName, "imdbId", movie->id());
                Renamer::replaceCondition(newFileName, "movieset", movie->set());
                Renamer::replaceCondition(newFileName, "3D", movie->streamDetails()->videoDetails().value("stereomode") != "");
                Helper::instance()->sanitizeFileName(newFileName);
                if (fi.fileName() != newFileName) {
                    ui->results->append(tr("<b>Rename File</b> \"%1\" to \"%2\"").arg(fi.fileName()).arg(newFileName));
                    if (!dryRun) {
                        if (!rename(file, fi.canonicalPath() + "/" + newFileName)){
                            ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                            continue;
                        }
                        else
                            FilmFiles.append(newFileName);
                    }
                    else
                        FilmFiles.append(newFileName);

                    foreach (const QString &trailerFile, currentDir.entryList(QStringList() << fi.completeBaseName() + "-trailer.*", QDir::Files | QDir::NoDotAndDotDot)) {
                        QFileInfo trailer(fi.canonicalPath() + "/" + trailerFile);
                        QString newTrailerFileName = newFileName;
                        newTrailerFileName = newTrailerFileName.left(newTrailerFileName.lastIndexOf(".")) + "-trailer." + trailer.suffix();
                        if (trailer.fileName() != newTrailerFileName) {
                            ui->results->append(tr("<b>Rename File</b> \"%1\" to \"%2\"").arg(trailer.fileName()).arg(newTrailerFileName));
                            if (!dryRun) {
                                if (!rename(fi.canonicalPath() + "/" + trailerFile, fi.canonicalPath() + "/" + newTrailerFileName))
                                    ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                                else
                                    FilmFiles.append(newTrailerFileName);
                            }
                            else
                                FilmFiles.append(newTrailerFileName);
                        }
                        else
                            FilmFiles.append(trailer.fileName());
                    }

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
                }
                else
                    FilmFiles.append(fi.fileName());

            }

            // Rename nfo
            if (!nfo.isEmpty()) {
                QString nfoFileName = QFileInfo(nfo).fileName();
                QList<DataFile> nfoFiles = Settings::instance()->dataFiles(DataFileType::MovieNfo);
                if (!nfoFiles.isEmpty()) {
                    QString newNfoFileName = nfoFiles.first().saveFileName(newFileName, -1, movie->files().count() > 1);
                    Helper::instance()->sanitizeFileName(newNfoFileName);
                    if (newNfoFileName != nfoFileName) {
                        ui->results->append(tr("<b>Rename NFO</b> \"%1\" to \"%2\"").arg(nfoFileName).arg(newNfoFileName));
                        if (!dryRun) {
                            if (!rename(nfo, fiCanonicalPath + "/" + newNfoFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
                        ui->results->append(tr("<b>Rename Poster</b> \"%1\" to \"%2\"").arg(posterFileName).arg(newPosterFileName));
                        if (!dryRun) {
                            if (!rename(poster, fiCanonicalPath + "/" + newPosterFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
                        ui->results->append(tr("<b>Rename Fanart</b> \"%1\" to \"%2\"").arg(fanartFileName).arg(newFanartFileName));
                        if (!dryRun) {
                            if (!rename(fanart, fiCanonicalPath + "/" + newFanartFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
                        ui->results->append(tr("<b>Rename Banner</b> \"%1\" to \"%2\"").arg(bannerFileName).arg(newBannerFileName));
                        if (!dryRun) {
                            if (!rename(banner, fiCanonicalPath + "/" + newBannerFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
                        ui->results->append(tr("<b>Rename Thumb</b> \"%1\" to \"%2\"").arg(thumbFileName).arg(newThumbFileName));
                        if (!dryRun) {
                            if (!rename(thumb, fiCanonicalPath + "/" + newThumbFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
                        ui->results->append(tr("<b>Rename Logo</b> \"%1\" to \"%2\"").arg(logoFileName).arg(newLogoFileName));
                        if (!dryRun) {
                            if (!rename(logo, fiCanonicalPath + "/" + newLogoFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
                        ui->results->append(tr("<b>Rename Clear Art</b> \"%1\" to \"%2\"").arg(clearArtFileName).arg(newClearArtFileName));
                        if (!dryRun) {
                            if (!rename(clearArt, fiCanonicalPath + "/" + newClearArtFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
                        ui->results->append(tr("<b>Rename CD Art</b> \"%1\" to \"%2\"").arg(cdArtFileName).arg(newCdArtFileName));
                        if (!dryRun) {
                            if (!rename(cdArt, fiCanonicalPath + "/" + newCdArtFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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

        QString extension = (!movie->files().isEmpty()) ? QFileInfo(movie->files().first()).suffix() : "";
        //rename dir for already existe films dir
        if (renameDirectories && movie->inSeparateFolder()) {
            Renamer::replace(newFolderName, "title", movie->name());
            Renamer::replace(newFolderName, "extension", extension);
            Renamer::replace(newFolderName, "originalTitle", movie->originalName());
            Renamer::replace(newFolderName, "sortTitle", movie->sortTitle());
            Renamer::replace(newFolderName, "year", movie->released().toString("yyyy"));
            Renamer::replace(newFolderName, "resolution", Helper::instance()->matchResolution(movie->streamDetails()->videoDetails().value("width").toInt(),
                                                                                              movie->streamDetails()->videoDetails().value("height").toInt()));
            Renamer::replaceCondition(newFolderName, "bluray", isBluRay);
            Renamer::replaceCondition(newFolderName, "dvd", isDvd);
            Renamer::replaceCondition(newFolderName, "3D", movie->streamDetails()->videoDetails().value("stereomode") != "");
            Renamer::replaceCondition(newFolderName, "movieset", movie->set());
            Helper::instance()->sanitizeFileName(newFolderName);
            if (dir.dirName() != newFolderName)
                ui->results->append(tr("<b>Rename Directory</b> \"%1\" to \"%2\"").arg(dir.dirName()).arg(newFolderName));
        }
        //create dir for new dir structure
        else if (renameDirectories) {
            Renamer::replace(newFolderName, "title", movie->name());
            Renamer::replace(newFolderName, "extension", extension);
            Renamer::replace(newFolderName, "originalTitle", movie->originalName());
            Renamer::replace(newFolderName, "sortTitle", movie->sortTitle());
            Renamer::replace(newFolderName, "year", movie->released().toString("yyyy"));
            Renamer::replace(newFolderName, "resolution", Helper::instance()->matchResolution(movie->streamDetails()->videoDetails().value("width").toInt(),
                                                                                              movie->streamDetails()->videoDetails().value("height").toInt()));
            Renamer::replaceCondition(newFolderName, "bluray", isBluRay);
            Renamer::replaceCondition(newFolderName, "dvd", isDvd);
            Renamer::replaceCondition(newFolderName, "3D", movie->streamDetails()->videoDetails().value("stereomode") != "");
            Renamer::replaceCondition(newFolderName, "movieset", movie->set());
            Helper::instance()->sanitizeFileName(newFolderName);
            if (dir.dirName() != newFolderName){ //check if movie is not already on good folder
                int i = 0 ;
                while (dir.exists(newFolderName)){
                    newFolderName = newFolderName + " " + QString::number(++i);
                }

                ui->results->append(tr("<b>Create Directory</b> \"%2\" into \"%1\"").arg(dir.dirName()).arg(newFolderName));

                if (!dryRun) {
                    if (!dir.mkdir(newFolderName)){
                        ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>"+ tr("Failed") + "</b></span>");
                        continue;
                    }
                }

                foreach (const QString &fileName, FilmFiles) {
                    QFileInfo fi(fileName);
                    if (dir.dirName() != newFolderName){
                        ui->results->append(tr("<b>Move File</b> \"%1\" to \"%2\"").arg(fi.fileName()).arg(dir.dirName() + "/" + newFolderName + "/" + fi.fileName()));
                        if (!dryRun) {
                            if (!rename(dir.absolutePath() + "/" +fileName, dir.absolutePath() + "/" + newFolderName + "/" + fi.fileName()))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");

                        }
                    }
                }
            }
        }


        if (!dryRun && dir.dirName() != newFolderName && renameDirectories && movie->inSeparateFolder()) {
            QDir parentDir(dir.path());
            parentDir.cdUp();
            if (!rename(dir, parentDir.path() + "/" + newFolderName))
                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
                episode->hasChanged() || episodesRenamed.contains(episode))
            continue;

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

        if (!isBluRay && !isDvd && !isDvdWithoutSub && renameFiles) {
            qApp->processEvents();
            QString newFileName;
            episodeFiles.clear();

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
                Renamer::replace(newFileName, "resolution", Helper::instance()->matchResolution(episode->streamDetails()->videoDetails().value("width").toInt(),
                                                                                                episode->streamDetails()->videoDetails().value("height").toInt()));
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
                    ui->results->append(tr("<b>Rename File</b> \"%1\" to \"%2\"").arg(fi.fileName()).arg(newFileName));
                    if (!dryRun) {
                        if (!rename(file, fi.canonicalPath() + "/" + newFileName))
                            ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                    }

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
                        }
                    }

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
                        ui->results->append(tr("<b>Rename NFO</b> \"%1\" to \"%2\"").arg(nfoFileName).arg(newNfoFileName));
                        if (!dryRun) {
                            if (!rename(nfo, fiCanonicalPath + "/" + newNfoFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
                        ui->results->append(tr("<b>Rename Thumbnail</b> \"%1\" to \"%2\"").arg(thumbnailFileName).arg(newThumbnailFileName));
                        if (!dryRun) {
                            if (!rename(thumbnail, fiCanonicalPath + "/" + newThumbnailFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                        }
                    }
                }
            }
        }

        if (useSeasonDirectories) {
            QDir showDir(episode->tvShow()->dir());
            QString seasonDirName = seasonPattern;
            Renamer::replace(seasonDirName, "season", episode->seasonString());
            Helper::instance()->sanitizeFileName(seasonDirName);
            QDir seasonDir(showDir.path() + "/" + seasonDirName);
            if (!seasonDir.exists()) {
                ui->results->append(tr("<b>Create Directory</b> \"%1\"").arg(seasonDirName));
                if (!dryRun) {
                    if (!showDir.mkdir(seasonDirName))
                        ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                }
            }

            if (isBluRay || isDvd || isDvdWithoutSub) {
                QDir dir = fi.dir();
                if (isDvd || isBluRay)
                    dir.cdUp();

                QDir parentDir = dir;
                parentDir.cdUp();
                if (parentDir != seasonDir) {
                    ui->results->append(tr("<b>Move Episode</b> \"%1\" to \"%2\"").arg(dir.dirName()).arg(seasonDirName));
                    if (!dryRun) {
                        if (!rename(dir, seasonDir.absolutePath() + "/" + dir.dirName()))
                            ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                    }
                }
            } else if (fi.dir() != seasonDir) {
                foreach (const QString &fileName, episodeFiles) {
                    QFileInfo fi(fileName);
                    ui->results->append(tr("<b>Move Episode</b> \"%1\" to \"%2\"").arg(fi.fileName()).arg(seasonDirName));
                    if (!dryRun) {
                        if (!rename(fileName, seasonDir.path() + "/" + fi.fileName()))
                            ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                    }
                }

                if (!newNfoFileName.isEmpty() && !nfo.isEmpty()) {
                    ui->results->append(tr("<b>Move NFO</b> \"%1\" to \"%2\"").arg(newNfoFileName).arg(seasonDirName));
                    if (!dryRun) {
                        if (!rename(fiCanonicalPath + "/" + newNfoFileName, seasonDir.path() + "/" + newNfoFileName))
                            ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                    }
                }
                if (!thumbnail.isEmpty() && !newThumbnailFileName.isEmpty()) {
                    ui->results->append(tr("<b>Move Thumbnail</b> \"%1\" to \"%2\"").arg(newThumbnailFileName).arg(seasonDirName));
                    if (!dryRun) {
                        if (!rename(fiCanonicalPath + "/" + newThumbnailFileName, seasonDir.path() + "/" + newThumbnailFileName))
                            ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
        if (show->hasChanged())
            continue;

        QDir dir(show->dir());
        QString newFolderName = directoryPattern;
        Renamer::replace(newFolderName, "title", show->name());
        Renamer::replace(newFolderName, "showTitle", show->name());
        Renamer::replace(newFolderName, "year", show->firstAired().toString("yyyy"));
        Helper::instance()->sanitizeFileName(newFolderName);
        if (newFolderName != dir.dirName()) {
            ui->results->append(tr("<b>Rename Directory</b> \"%1\" to \"%2\"").arg(dir.dirName()).arg(newFolderName));
            QDir parentDir(dir.path());
            parentDir.cdUp();
            if (!dryRun) {
                if (!rename(dir, parentDir.path() + "/" + newFolderName))
                    ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
        if (concert->files().isEmpty() || (concert->files().count() > 1 && filePatternMulti.isEmpty()) || concert->hasChanged())
            continue;

        qApp->processEvents();
        QFileInfo fi(concert->files().first());
        QString fiCanonicalPath = fi.canonicalPath();
        QDir dir(fi.canonicalPath());
        QString newFolderName = directoryPattern;
        QString newFileName;
        QString nfo = Manager::instance()->mediaCenterInterface()->nfoFilePath(concert);
        QString poster = Manager::instance()->mediaCenterInterface()->imageFileName(concert, ImageType::ConcertPoster);
        QString fanart = Manager::instance()->mediaCenterInterface()->imageFileName(concert, ImageType::ConcertBackdrop);

        QDir chkDir(fi.canonicalPath());
        chkDir.cdUp();

        bool isBluRay = Helper::instance()->isBluRay(chkDir.path());
        bool isDvd = Helper::instance()->isDvd(chkDir.path());

        if (isBluRay || isDvd)
            dir.cdUp();

        if (!isBluRay && !isDvd && renameFiles) {
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
                Renamer::replace(newFileName, "resolution", Helper::instance()->matchResolution(concert->streamDetails()->videoDetails().value("width").toInt(),
                                                                                                concert->streamDetails()->videoDetails().value("height").toInt()));
                Renamer::replaceCondition(newFileName, "3D", concert->streamDetails()->videoDetails().value("stereomode") != "");
                Helper::instance()->sanitizeFileName(newFileName);
                if (fi.fileName() != newFileName) {
                    ui->results->append(tr("<b>Rename File</b> \"%1\" to \"%2\"").arg(fi.fileName()).arg(newFileName));
                    if (!dryRun) {
                        if (!rename(file, fi.canonicalPath() + "/" + newFileName))
                            ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                    }

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
                        }
                    }
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
                        ui->results->append(tr("<b>Rename NFO</b> \"%1\" to \"%2\"").arg(nfoFileName).arg(newNfoFileName));
                        if (!dryRun) {
                            if (!rename(nfo, fiCanonicalPath + "/" + newNfoFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
                        ui->results->append(tr("<b>Rename Poster</b> \"%1\" to \"%2\"").arg(posterFileName).arg(newPosterFileName));
                        if (!dryRun) {
                            if (!rename(poster, fiCanonicalPath + "/" + newPosterFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
                        ui->results->append(tr("<b>Rename Fanart</b> \"%1\" to \"%2\"").arg(fanartFileName).arg(newFanartFileName));
                        if (!dryRun) {
                            if (!rename(fanart, fiCanonicalPath + "/" + newFanartFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                        }
                    }
                }
            }
        }

        if (renameDirectories && concert->inSeparateFolder()) {
            Renamer::replace(newFolderName, "title", concert->name());
            Renamer::replace(newFolderName, "artist", concert->artist());
            Renamer::replace(newFolderName, "album", concert->album());
            Renamer::replace(newFolderName, "year", concert->released().toString("yyyy"));
            Renamer::replaceCondition(newFolderName, "bluray", isBluRay);
            Renamer::replaceCondition(newFolderName, "dvd", isDvd);
            Renamer::replaceCondition(newFolderName, "3D", concert->streamDetails()->videoDetails().value("stereomode") != "");
            Renamer::replace(newFolderName, "resolution", Helper::instance()->matchResolution(concert->streamDetails()->videoDetails().value("width").toInt(),
                                                                                              concert->streamDetails()->videoDetails().value("height").toInt()));
            Helper::instance()->sanitizeFileName(newFolderName);
            if (dir.dirName() != newFolderName)
                ui->results->append(tr("<b>Rename Directory</b> \"%1\" to \"%2\"").arg(dir.dirName()).arg(newFolderName));
        }

        if (!dryRun && dir.dirName() != newFolderName && renameDirectories && concert->inSeparateFolder()) {
            QDir parentDir(dir.path());
            parentDir.cdUp();
            if (!rename(dir, parentDir.path() + "/" + newFolderName))
                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
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
