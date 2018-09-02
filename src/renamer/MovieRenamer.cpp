#include "MovieRenamer.h"

#include "data/MediaCenterInterface.h"
#include "data/Movie.h"
#include "globals/Helper.h"
#include "globals/Manager.h"

#include <QDir>
#include <QFileInfo>

MovieRenamer::MovieRenamer(RenamerConfig renamerConfig, RenamerDialog *dialog) : Renamer(renamerConfig, dialog)
{
}

MovieRenamer::RenameError MovieRenamer::renameMovie(Movie &movie)
{
    QFileInfo fi(movie.files().first());
    QString fiCanonicalPath = fi.canonicalPath();
    QDir dir(fi.canonicalPath());
    QString newFolderName = m_config.directoryPattern;
    QString newFileName;
    QStringList FilmFiles;
    QStringList newMovieFiles;
    QString parentDirName;
    bool errorOccured = false;

    for (const QString &file : movie.files()) {
        QFileInfo fi(file);
        newMovieFiles.append(fi.fileName());
    }

    // Parent directory of this movie's folder
    QString baseDir = [&fi]() {
        QDir chkDir(fi.canonicalPath());
        chkDir.cdUp();
        return chkDir.path();
    }();

    const bool isBluRay = Helper::instance()->isBluRay(baseDir);
    const bool isDvd = Helper::instance()->isDvd(baseDir);

    // BlueRay and DVD folder content must not be renamed.
    if (isBluRay || isDvd) {
        parentDirName = dir.dirName();
        dir.cdUp();
    }

    if (!isBluRay && !isDvd && m_config.renameFiles) {
        newMovieFiles.clear();
        int partNo = 0;
        const auto videoDetails = movie.streamDetails()->videoDetails();
        for (const QString &file : movie.files()) {
            newFileName = (movie.files().count() == 1) ? m_config.filePattern : m_config.filePatternMulti;
            QFileInfo fi(file);
            QString baseName = fi.completeBaseName();
            QDir currentDir = fi.dir();
            MovieRenamer::replace(newFileName, "title", movie.name());
            MovieRenamer::replace(newFileName, "originalTitle", movie.originalName());
            MovieRenamer::replace(newFileName, "sortTitle", movie.sortTitle());
            MovieRenamer::replace(newFileName, "year", movie.released().toString("yyyy"));
            MovieRenamer::replace(newFileName, "extension", fi.suffix());
            MovieRenamer::replace(newFileName, "partNo", QString::number(++partNo));
            MovieRenamer::replace(newFileName, "videoCodec", movie.streamDetails()->videoCodec());
            MovieRenamer::replace(newFileName, "audioCodec", movie.streamDetails()->audioCodec());
            MovieRenamer::replace(newFileName, "channels", QString::number(movie.streamDetails()->audioChannels()));
            MovieRenamer::replace(newFileName,
                "resolution",
                Helper::instance()->matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::ScanType)));
            MovieRenamer::replaceCondition(newFileName, "imdbId", movie.id());
            MovieRenamer::replaceCondition(newFileName, "movieset", movie.set());
            MovieRenamer::replaceCondition(
                newFileName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
            Helper::instance()->sanitizeFileName(newFileName);
            if (fi.fileName() != newFileName) {
                int row = m_dialog->addResultToTable(fi.fileName(), newFileName, RenameOperation::Rename);
                if (!m_config.dryRun) {
                    if (!rename(file, fi.canonicalPath() + "/" + newFileName)) {
                        m_dialog->setResultStatus(row, RenameResult::Failed);
                        errorOccured = true;
                        continue;
                    } else {
                        FilmFiles.append(newFileName);
                        newMovieFiles.append(newFileName);
                    }
                } else {
                    FilmFiles.append(newFileName);
                }

                for (const QString &trailerFile : currentDir.entryList(
                         QStringList() << fi.completeBaseName() + "-trailer.*", QDir::Files | QDir::NoDotAndDotDot)) {
                    QFileInfo trailer(fi.canonicalPath() + "/" + trailerFile);
                    QString newTrailerFileName = newFileName;
                    newTrailerFileName =
                        newTrailerFileName.left(newTrailerFileName.lastIndexOf(".")) + "-trailer." + trailer.suffix();
                    if (trailer.fileName() != newTrailerFileName) {
                        int row =
                            m_dialog->addResultToTable(trailer.fileName(), newTrailerFileName, RenameOperation::Rename);
                        if (!m_config.dryRun) {
                            if (!rename(fi.canonicalPath() + "/" + trailerFile,
                                    fi.canonicalPath() + "/" + newTrailerFileName)) {
                                m_dialog->setResultStatus(row, RenameResult::Failed);
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

                for (Subtitle *subtitle : movie.subtitles()) {
                    QString subFileName = QFileInfo(newFileName).completeBaseName();
                    if (!subtitle->language().isEmpty()) {
                        subFileName.append("." + subtitle->language());
                    }
                    if (subtitle->forced()) {
                        subFileName.append(".forced");
                    }

                    QStringList newSubFiles;
                    for (const QString &subFile : subtitle->files()) {
                        QFileInfo subFi(fi.canonicalPath() + "/" + subFile);
                        QString newSubFileName = subFileName + "." + subFi.suffix();
                        int row = m_dialog->addResultToTable(subFile, newSubFileName, RenameOperation::Rename);
                        if (!m_config.dryRun) {
                            if (!rename(
                                    fi.canonicalPath() + "/" + subFile, fi.canonicalPath() + "/" + newSubFileName)) {
                                newSubFiles << subFile;
                                m_dialog->setResultStatus(row, RenameResult::Failed);
                            } else {
                                newSubFiles << newSubFileName;
                                FilmFiles.append(newSubFileName);
                            }
                        } else {
                            FilmFiles.append(newSubFileName);
                        }
                    }
                    if (!m_config.dryRun) {
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
            QString newDataFileName = files.first().saveFileName(newFileName, -1, movie.files().count() > 1);
            Helper::instance()->sanitizeFileName(newDataFileName);

            if (newDataFileName == fileName) {
                // File already has correct name
                FilmFiles.append(fileName);
                return;
            }

            int row = m_dialog->addResultToTable(fileName, newDataFileName, RenameOperation::Rename);
            if (m_config.dryRun) {
                FilmFiles.append(newDataFileName);
                return;
            }

            if (rename(filePath, fiCanonicalPath + "/" + newDataFileName)) {
                FilmFiles.append(newDataFileName);
            } else {
                m_dialog->setResultStatus(row, RenameResult::Failed);
            }
        };

        MediaCenterInterface *mediaCenter = Manager::instance()->mediaCenterInterface();
        const auto renameImageType = [&](ImageType imageType) {
            DataFileType fileType = DataFile::dataFileTypeForImageType(imageType);
            renameFileType(mediaCenter->imageFileName(&movie, imageType), fileType);
        };

        renameFileType(mediaCenter->nfoFilePath(&movie), DataFileType::MovieNfo);

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
    QString extension = (!movie.files().isEmpty()) ? QFileInfo(movie.files().first()).suffix() : "";
    const auto videoDetails = movie.streamDetails()->videoDetails();
    // rename dir for already existe films dir
    if (m_config.renameDirectories && movie.inSeparateFolder()) {
        MovieRenamer::replace(newFolderName, "title", movie.name());
        MovieRenamer::replace(newFolderName, "extension", extension);
        MovieRenamer::replace(newFolderName, "originalTitle", movie.originalName());
        MovieRenamer::replace(newFolderName, "sortTitle", movie.sortTitle());
        MovieRenamer::replace(newFolderName, "year", movie.released().toString("yyyy"));
        MovieRenamer::replace(newFolderName, "videoCodec", movie.streamDetails()->videoCodec());
        MovieRenamer::replace(newFolderName, "audioCodec", movie.streamDetails()->audioCodec());
        MovieRenamer::replace(newFolderName, "channels", QString::number(movie.streamDetails()->audioChannels()));
        MovieRenamer::replace(newFolderName,
            "resolution",
            Helper::instance()->matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                videoDetails.value(StreamDetails::VideoDetails::ScanType)));
        MovieRenamer::replaceCondition(newFolderName, "bluray", isBluRay);
        MovieRenamer::replaceCondition(newFolderName, "dvd", isDvd);
        MovieRenamer::replaceCondition(
            newFolderName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
        MovieRenamer::replaceCondition(newFolderName, "movieset", movie.set());
        MovieRenamer::replaceCondition(newFolderName, "imdbId", movie.id());
        Helper::instance()->sanitizeFileName(newFolderName);
        if (dir.dirName() != newFolderName) {
            renameRow = m_dialog->addResultToTable(dir.dirName(), newFolderName, RenameOperation::Rename);
        }
    }
    // create dir for new dir structure
    else if (m_config.renameDirectories) {
        MovieRenamer::replace(newFolderName, "title", movie.name());
        MovieRenamer::replace(newFolderName, "extension", extension);
        MovieRenamer::replace(newFolderName, "originalTitle", movie.originalName());
        MovieRenamer::replace(newFolderName, "sortTitle", movie.sortTitle());
        MovieRenamer::replace(newFolderName, "year", movie.released().toString("yyyy"));
        MovieRenamer::replace(newFolderName, "videoCodec", movie.streamDetails()->videoCodec());
        MovieRenamer::replace(newFolderName, "audioCodec", movie.streamDetails()->audioCodec());
        MovieRenamer::replace(newFolderName, "channels", QString::number(movie.streamDetails()->audioChannels()));
        MovieRenamer::replace(newFolderName,
            "resolution",
            Helper::instance()->matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                videoDetails.value(StreamDetails::VideoDetails::ScanType)));
        MovieRenamer::replaceCondition(newFolderName, "bluray", isBluRay);
        MovieRenamer::replaceCondition(newFolderName, "dvd", isDvd);
        MovieRenamer::replaceCondition(
            newFolderName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
        MovieRenamer::replaceCondition(newFolderName, "movieset", movie.set());
        MovieRenamer::replaceCondition(newFolderName, "imdbId", movie.id());
        Helper::instance()->sanitizeFileName(newFolderName);

        if (dir.dirName() != newFolderName) { // check if movie is not already on good folder
            int i = 0;
            while (dir.exists(newFolderName)) {
                newFolderName = newFolderName + " " + QString::number(++i);
            }

            int row = m_dialog->addResultToTable(dir.dirName(), newFolderName, RenameOperation::CreateDir);

            if (!m_config.dryRun) {
                if (!dir.mkdir(newFolderName)) {
                    m_dialog->setResultStatus(row, RenameResult::Failed);
                    return RenameError::Error;
                } else {
                    newMovieFolder = dir.path() + "/" + newFolderName;
                }
            }

            for (const QString &fileName : FilmFiles) {
                QFileInfo fi(fileName);
                if (dir.dirName() != newFolderName) {
                    int row = m_dialog->addResultToTable(fi.fileName(),
                        dir.dirName() + "/" + newFolderName + "/" + fi.fileName(),
                        RenameOperation::Move);
                    m_dialog->appendResultText(QObject::tr(R"(<b>Move File</b> "%1" to "%2")")
                                                   .arg(fi.fileName())
                                                   .arg(dir.dirName() + "/" + newFolderName + "/" + fi.fileName()));
                    if (!m_config.dryRun) {
                        if (!rename(dir.absolutePath() + "/" + fileName,
                                dir.absolutePath() + "/" + newFolderName + "/" + fi.fileName())) {
                            m_dialog->setResultStatus(row, RenameResult::Failed);
                        }
                    }
                }
            }
        }
    }


    if (!m_config.dryRun && dir.dirName() != newFolderName && m_config.renameDirectories && movie.inSeparateFolder()) {
        QDir parentDir(dir.path());
        parentDir.cdUp();
        if (!rename(dir, parentDir.path() + "/" + newFolderName)) {
            m_dialog->setResultStatus(renameRow, RenameResult::Failed);
            errorOccured = true;
        } else {
            newMovieFolder = parentDir.path() + "/" + newFolderName;
        }
    }

    if (!errorOccured && !m_config.dryRun) {
        QStringList files;
        for (const QString &file : newMovieFiles) {
            QString f = newMovieFolder;
            if (isBluRay || isDvd) {
                f += "/" + parentDirName;
            }
            f += "/" + file;
            files << f;
        }
        movie.setFiles(files);
        Manager::instance()->database()->update(&movie);
    }

    if (errorOccured) {
        return RenameError::Error;
    }
    return RenameError::None;
}
