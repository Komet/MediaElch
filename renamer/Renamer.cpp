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

    // Movies
    ui->labelOriginalTitle->setVisible(m_renameType == TypeMovies);
    ui->descOriginalTitle->setVisible(m_renameType == TypeMovies);

    // Concerts
    ui->labelAlbum->setVisible(m_renameType == TypeConcerts);
    ui->descAlbum->setVisible(m_renameType == TypeConcerts);
    ui->labelArtist->setVisible(m_renameType == TypeConcerts);
    ui->descArtist->setVisible(m_renameType == TypeConcerts);

    // TV Shows
    ui->labelEpisode->setVisible(m_renameType == TypeTvShows);
    ui->descEpisode->setVisible(m_renameType == TypeTvShows);
    ui->labelSeason->setVisible(m_renameType == TypeTvShows);
    ui->descSeason->setVisible(m_renameType == TypeTvShows);
    ui->labelShowTitle->setVisible(m_renameType == TypeTvShows);
    ui->descShowTitle->setVisible(m_renameType == TypeTvShows);

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
        QString poster = Manager::instance()->mediaCenterInterface()->posterImageName(movie);
        QString fanart = Manager::instance()->mediaCenterInterface()->backdropImageName(movie);

        QDir chkDir(fi.canonicalPath());
        chkDir.cdUp();

        bool isBluRay = Helper::isBluRay(chkDir.path());
        bool isDvd = Helper::isDvd(chkDir.path());

        if (isBluRay || isDvd)
            dir.cdUp();

        if (!isBluRay && !isDvd && renameFiles) {
            int partNo = 0;
            foreach (const QString &file, movie->files()) {
                newFileName = (movie->files().count() == 1) ? filePattern : filePatternMulti;
                QFileInfo fi(file);
                newFileName.replace("<title>", movie->name());
                newFileName.replace("<originalTitle>", movie->originalName());
                newFileName.replace("<year>", movie->released().toString("yyyy"));
                newFileName.replace("<extension>", fi.suffix());
                newFileName.replace("<partNo>", QString::number(++partNo));
                Helper::sanitizeFileName(newFileName);
                if (fi.fileName() != newFileName) {
                    ui->results->append(tr("<b>Rename File</b> \"%1\" to \"%2\"").arg(fi.fileName()).arg(newFileName));
                    if (!dryRun) {
                        if (!rename(file, fi.canonicalPath() + "/" + newFileName))
                            ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                    }
                }
            }

            // Rename nfo
            if (!nfo.isEmpty()) {
                QString nfoFileName = QFileInfo(nfo).fileName();
                QList<DataFile> nfoFiles = Settings::instance()->dataFiles(DataFileType::MovieNfo);
                if (!nfoFiles.isEmpty()) {
                    QString newNfoFileName = nfoFiles.first().saveFileName(newFileName);
                    Helper::sanitizeFileName(newNfoFileName);
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
                QList<DataFile> posterFiles = Settings::instance()->dataFiles(DataFileType::MoviePoster);
                if (!posterFiles.isEmpty()) {
                    QString newPosterFileName = posterFiles.first().saveFileName(newFileName, -1, movie->files().count() > 1);
                    Helper::sanitizeFileName(newPosterFileName);
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
                QList<DataFile> fanartFiles = Settings::instance()->dataFiles(DataFileType::MovieBackdrop);
                if (!fanartFiles.isEmpty()) {
                    QString newFanartFileName = fanartFiles.first().saveFileName(newFileName, -1, movie->files().count() > 1);
                    Helper::sanitizeFileName(newFanartFileName);
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

        if (renameDirectories && movie->inSeparateFolder()) {
            newFolderName.replace("<title>", movie->name());
            newFolderName.replace("<originalTitle>", movie->originalName());
            newFolderName.replace("<year>", movie->released().toString("yyyy"));
            Helper::sanitizeFileName(newFolderName);
            if (dir.dirName() != newFolderName)
                ui->results->append(tr("<b>Rename Directory</b> \"%1\" to \"%2\"").arg(dir.dirName()).arg(newFolderName));
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

    foreach (TvShowEpisode *episode, episodes) {
        if (episode->files().isEmpty() || (episode->files().count() > 1 && filePatternMulti.isEmpty()) || episode->hasChanged())
            continue;

        if (renameFiles) {
            qApp->processEvents();
            QFileInfo fi(episode->files().first());
            QString fiCanonicalPath = fi.canonicalPath();
            QString newFileName;
            QStringList episodeFiles;

            QString thumbnail = Manager::instance()->mediaCenterInterface()->thumbnailImageName(episode);
            QString nfo = Manager::instance()->mediaCenterInterface()->nfoFilePath(episode);

            int partNo = 0;
            foreach (const QString &file, episode->files()) {
                newFileName = (episode->files().count() == 1) ? filePattern : filePatternMulti;
                QFileInfo fi(file);
                newFileName.replace("<title>", episode->name());
                newFileName.replace("<showTitle>", episode->showTitle());
                newFileName.replace("<year>", episode->firstAired().toString("yyyy"));
                newFileName.replace("<extension>", fi.suffix());
                newFileName.replace("<episode>", episode->episodeString());
                newFileName.replace("<season>", episode->seasonString());
                newFileName.replace("<partNo>", QString::number(++partNo));
                Helper::sanitizeFileName(newFileName);
                if (fi.fileName() != newFileName) {
                    ui->results->append(tr("<b>Rename File</b> \"%1\" to \"%2\"").arg(fi.fileName()).arg(newFileName));
                    if (!dryRun) {
                        if (!rename(file, fi.canonicalPath() + "/" + newFileName))
                            ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                    }
                }
                episodeFiles << fi.canonicalPath() + "/" + newFileName;
            }

            // Rename nfo
            QString newNfoFileName;
            if (!nfo.isEmpty()) {
                QString nfoFileName = QFileInfo(nfo).fileName();
                QList<DataFile> nfoFiles = Settings::instance()->dataFiles(DataFileType::TvShowEpisodeNfo);
                if (!nfoFiles.isEmpty()) {
                    newNfoFileName = nfoFiles.first().saveFileName(newFileName);
                    Helper::sanitizeFileName(newNfoFileName);
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
            QString newThumbnailFileName;
            if (!thumbnail.isEmpty()) {
                QString thumbnailFileName = QFileInfo(thumbnail).fileName();
                QList<DataFile> thumbnailFiles = Settings::instance()->dataFiles(DataFileType::TvShowEpisodeThumb);
                if (!thumbnailFiles.isEmpty()) {
                    newThumbnailFileName = thumbnailFiles.first().saveFileName(newFileName, -1, episode->files().count() > 1);
                    Helper::sanitizeFileName(newThumbnailFileName);
                    if (newThumbnailFileName != thumbnailFileName) {
                        ui->results->append(tr("<b>Rename Thumbnail</b> \"%1\" to \"%2\"").arg(thumbnailFileName).arg(newThumbnailFileName));
                        if (!dryRun) {
                            if (!rename(thumbnail, fiCanonicalPath + "/" + newThumbnailFileName))
                                ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                        }
                    }
                }
            }

            if (useSeasonDirectories) {
                QDir showDir(episode->tvShow()->dir());
                QString seasonDirName = seasonPattern;
                seasonDirName.replace("<season>", episode->seasonString());
                Helper::sanitizeFileName(seasonDirName);
                QDir seasonDir(showDir.path() + "/" + seasonDirName);
                if (!seasonDir.exists()) {
                    ui->results->append(tr("<b>Create Directory</b> \"%1\"").arg(seasonDirName));
                    if (!dryRun) {
                        if (!showDir.mkdir(seasonDirName))
                            ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                    }
                }

                if (fi.dir() != seasonDir) {
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
}

void Renamer::renameShows(QList<TvShow *> shows, const QString &directoryPattern, const bool &renameDirectories, const bool &dryRun)
{
    if (renameDirectories && directoryPattern.isEmpty())
        return;

    foreach (TvShow *show, shows) {
        if (show->hasChanged())
            continue;

        QDir dir(show->dir());
        QString newFolderName = directoryPattern;
        newFolderName.replace("<title>", show->name());
        newFolderName.replace("<showTitle>", show->name());
        newFolderName.replace("<year>", show->firstAired().toString("yyyy"));
        Helper::sanitizeFileName(newFolderName);
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
        QString poster = Manager::instance()->mediaCenterInterface()->posterImageName(concert);
        QString fanart = Manager::instance()->mediaCenterInterface()->backdropImageName(concert);

        QDir chkDir(fi.canonicalPath());
        chkDir.cdUp();

        bool isBluRay = Helper::isBluRay(chkDir.path());
        bool isDvd = Helper::isDvd(chkDir.path());

        if (isBluRay || isDvd)
            dir.cdUp();

        if (!isBluRay && !isDvd && renameFiles) {
            int partNo = 0;
            foreach (const QString &file, concert->files()) {
                newFileName = (concert->files().count() == 1) ? filePattern : filePatternMulti;
                QFileInfo fi(file);
                newFileName.replace("<title>", concert->name());
                newFileName.replace("<artist>", concert->artist());
                newFileName.replace("<album>", concert->album());
                newFileName.replace("<year>", concert->released().toString("yyyy"));
                newFileName.replace("<extension>", fi.suffix());
                newFileName.replace("<partNo>", QString::number(++partNo));
                Helper::sanitizeFileName(newFileName);
                if (fi.fileName() != newFileName) {
                    ui->results->append(tr("<b>Rename File</b> \"%1\" to \"%2\"").arg(fi.fileName()).arg(newFileName));
                    if (!dryRun) {
                        if (!rename(file, fi.canonicalPath() + "/" + newFileName))
                            ui->results->append("&nbsp;&nbsp;<span style=\"color:#ff0000;\"><b>" + tr("Failed") + "</b></span>");
                    }
                }
            }

            // Rename nfo
            if (!nfo.isEmpty()) {
                QString nfoFileName = QFileInfo(nfo).fileName();
                QList<DataFile> nfoFiles = Settings::instance()->dataFiles(DataFileType::ConcertNfo);
                if (!nfoFiles.isEmpty()) {
                    QString newNfoFileName = nfoFiles.first().saveFileName(newFileName);
                    Helper::sanitizeFileName(newNfoFileName);
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
                    Helper::sanitizeFileName(newPosterFileName);
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
                    Helper::sanitizeFileName(newFanartFileName);
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
            newFolderName.replace("<title>", concert->name());
            newFolderName.replace("<artist>", concert->artist());
            newFolderName.replace("<album>", concert->album());
            newFolderName.replace("<year>", concert->released().toString("yyyy"));
            Helper::sanitizeFileName(newFolderName);
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
    if (newFile.exists())
        return false;

    return f.rename(newName);
}

bool Renamer::rename(QDir &dir, QString newName)
{
    QDir tmpDir;
    return tmpDir.rename(dir.path(), newName);
}
