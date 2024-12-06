#include "MovieRenamer.h"

#include "data/movie/Movie.h"
#include "database/MoviePersistence.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "media_center/MediaCenterInterface.h"

#include <QDir>
#include <QFileInfo>

MovieRenamer::MovieRenamer(RenamerConfig renamerConfig, RenamerDialog* dialog) : Renamer(renamerConfig, dialog)
{
}

MovieRenamer::RenameError MovieRenamer::renameMovie(Movie& movie)
{
    QFileInfo movieInfo(movie.files().first().toString());
    QString fiCanonicalPath = movieInfo.canonicalPath();
    QDir dir(movieInfo.canonicalPath());
    QString newFolderName = m_config.directoryPattern;

    bool replaceDelimiter = m_config.replaceDelimiter;
    QString oldDelimiter = " ";
    QString newDelimiter = (replaceDelimiter) ? m_config.newDelimiterPattern : oldDelimiter;

    MediaCenterInterface* mediaCenter = Manager::instance()->mediaCenterInterface();
    QString nfo = mediaCenter->nfoFilePath(&movie);

    QString newFileName;
    QStringList FilmFiles;
    QStringList newMovieFiles;
    QString parentDirName;
    bool errorOccurred = false;

    for (const mediaelch::FilePath& file : movie.files()) {
        newMovieFiles.append(file.fileName());
    }

    // Parent directory of this movie's folder
    QString baseDir = [&movieInfo]() {
        QDir chkDir(movieInfo.canonicalPath());
        chkDir.cdUp();
        return chkDir.path();
    }();

    const bool isBluRay = helper::isBluRay(baseDir);
    const bool isDvd = helper::isDvd(baseDir);

    // BluRay and DVD folder content must not be renamed.
    if (isBluRay || isDvd) {
        parentDirName = dir.dirName();
        dir.cdUp();
    }

    if (!isBluRay && !isDvd && m_config.renameFiles) {
        newMovieFiles.clear();
        int partNo = 0;
        const auto videoDetails = movie.streamDetails()->videoDetails();
        const auto& files = movie.files();
        for (const mediaelch::FilePath& file : files) {
            newFileName = (files.count() == 1) ? m_config.filePattern : m_config.filePatternMulti;
            QFileInfo fi(file.toString());
            QString baseName = fi.completeBaseName();
            QDir currentDir = fi.dir();
            MovieRenamer::replace(newFileName, "title", movie.name());
            MovieRenamer::replace(
                newFileName, "originalTitle", movie.originalName().isEmpty() ? movie.name() : movie.originalName());
            MovieRenamer::replace(newFileName, "sortTitle", movie.sortTitle());
            MovieRenamer::replace(newFileName, "director", movie.director());
            // TODO: Let the user decide whether only the first should be used or
            //       if a space should be the separator.
            MovieRenamer::replace(newFileName, "studio", movie.studios().join(","));
            MovieRenamer::replace(newFileName, "year", movie.released().toString("yyyy"));
            MovieRenamer::replace(newFileName, "extension", fi.suffix());
            MovieRenamer::replace(newFileName, "partNo", QString::number(++partNo));
            MovieRenamer::replace(newFileName, "videoCodec", movie.streamDetails()->videoCodec());
            MovieRenamer::replace(newFileName, "audioCodec", movie.streamDetails()->audioCodec());
            // TODO: Let the user decide whether only the first should be used or
            //       if a space should be the separator.
            MovieRenamer::replace(newFileName, "audioLanguage", movie.streamDetails()->allAudioLanguages().join("-"));
            // TODO: Let the user decide whether only the first should be used or
            //       if a space should be the separator.
            Renamer::replace(newFileName, "subtitleLanguage", movie.streamDetails()->allSubtitleLanguages().join("-"));
            MovieRenamer::replace(newFileName, "channels", QString::number(movie.streamDetails()->audioChannels()));
            MovieRenamer::replace(newFileName,
                "resolution",
                helper::matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::ScanType)));
            MovieRenamer::replaceCondition(newFileName, "imdbId", movie.imdbId().toString());
            MovieRenamer::replaceCondition(newFileName, "tmdbId", movie.tmdbId().toString());
            MovieRenamer::replaceCondition(newFileName, "movieset", movie.set().name);
            MovieRenamer::replaceCondition(
                newFileName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");

            // Sanitize + Replace Delimiter with the one chosen by the user
            helper::sanitizeFileName(newFileName, newDelimiter);

            if (fi.fileName() != newFileName) {
                {
                    const int row = m_dialog->addResultToTable(fi.fileName(), newFileName, RenameOperation::Rename);
                    if (!m_config.dryRun) {
                        if (!rename(file.toString(), fi.canonicalPath() + "/" + newFileName)) {
                            m_dialog->setResultStatus(row, RenameResult::Failed);
                            errorOccurred = true;
                            continue;
                        }
                        FilmFiles.append(newFileName);
                        newMovieFiles.append(newFileName);

                    } else {
                        FilmFiles.append(newFileName);
                    }
                }

                for (const QString& trailerFile : currentDir.entryList(
                         QStringList() << baseName + "-trailer.*", QDir::Files | QDir::NoDotAndDotDot)) {
                    QFileInfo trailer(fi.canonicalPath() + "/" + trailerFile);
                    QString newTrailerFileName = newFileName;
                    newTrailerFileName =
                        newTrailerFileName.left(newTrailerFileName.lastIndexOf(".")) + "-trailer." + trailer.suffix();
                    if (trailer.fileName() != newTrailerFileName) {
                        const int row =
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
                for (const QString &extra: m_extraFiles)
                    filters << baseName + extra;
                for (const QString &subFileName: currentDir.entryList(filters, QDir::Files |
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


                bool hasChangedSubTitles = false;
                for (Subtitle* subtitle : movie.subtitles()) {
                    QString subFileName = QFileInfo(newFileName).completeBaseName();
                    if (!subtitle->language().isEmpty()) {
                        subFileName.append("." + subtitle->language());
                    }
                    if (subtitle->forced()) {
                        subFileName.append(".forced");
                    }

                    QStringList newSubFiles;
                    bool hasCurrentNewName = false;
                    for (const QString& subFile : subtitle->files()) {
                        QFileInfo subFi(fi.canonicalPath() + "/" + subFile);
                        QString newSubFileName = subFileName + "." + subFi.suffix();
                        if (subFile != newSubFileName) {
                            hasCurrentNewName = true;
                            hasChangedSubTitles = true;
                            int row = m_dialog->addResultToTable(subFile, newSubFileName, RenameOperation::Rename);
                            if (!m_config.dryRun) {
                                if (!rename(fi.canonicalPath() + "/" + subFile,
                                        fi.canonicalPath() + "/" + newSubFileName)) {
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
                    }
                    if (hasCurrentNewName && !m_config.dryRun) {
                        subtitle->setFiles(newSubFiles, false);
                    }
                }
                if (hasChangedSubTitles && !m_config.dryRun) {
                    movie.setChanged(true);
                }
            } else {
                FilmFiles.append(fi.fileName());
                newMovieFiles.append(fi.fileName());
            }
        }

        // Rename additional files (nfo, poster, etc.)
        const auto renameFileType = [&](QString filePath, DataFileType dataFileType) -> void {
            if (filePath.isEmpty()) {
                // File does not exist, e.g. there is no poster.
                return;
            }

            QVector<DataFile> dataFiles = Settings::instance()->dataFiles(dataFileType);
            if (dataFiles.isEmpty()) {
                return;
            }

            QString fileName = QFileInfo(filePath).fileName();
            QString newDataFileName =
                dataFiles.first().saveFileName(newFileName, SeasonNumber::NoSeason, movie.files().count() > 1);
            // Sanitize + Replace Delimiter with the one chosen by the user
            helper::sanitizeFileName(newDataFileName, newDelimiter);

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

        const auto renameImageType = [&](ImageType imageType) {
            DataFileType fileType = DataFile::dataFileTypeForImageType(imageType);
            renameFileType(mediaCenter->imageFileName(&movie, imageType), fileType);
        };

        renameFileType(nfo, DataFileType::MovieNfo);

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
    QString extension = !movie.files().isEmpty() ? movie.files().first().fileSuffix() : "";
    const auto videoDetails = movie.streamDetails()->videoDetails();
    // rename dir for already existing movie dir
    if (m_config.renameDirectories && movie.inSeparateFolder()) {
        Renamer::replace(newFolderName, "title", movie.name());
        Renamer::replace(newFolderName, "extension", extension);
        Renamer::replace(
            newFolderName, "originalTitle", movie.originalName().isEmpty() ? movie.name() : movie.originalName());
        Renamer::replace(newFolderName, "sortTitle", movie.sortTitle());
        Renamer::replace(newFolderName, "director", movie.director());
        // TODO: Let the user decide whether only the first should be used or
        //       if a space should be the separator.
        Renamer::replace(newFolderName, "studio", movie.studios().join(","));
        Renamer::replace(newFolderName, "year", movie.released().toString("yyyy"));
        Renamer::replace(newFolderName, "videoCodec", movie.streamDetails()->videoCodec());
        // TODO: Let the user decide whether only the first should be used or
        //       if a space should be the separator.
        Renamer::replace(newFolderName, "audioLanguage", movie.streamDetails()->allAudioLanguages().join("-"));
        // TODO: Let the user decide whether only the first should be used or
        //       if a space should be the separator.
        Renamer::replace(newFolderName, "subtitleLanguage", movie.streamDetails()->allSubtitleLanguages().join("-"));
        Renamer::replace(newFolderName, "audioCodec", movie.streamDetails()->audioCodec());
        Renamer::replace(newFolderName, "channels", QString::number(movie.streamDetails()->audioChannels()));
        Renamer::replace(newFolderName,
            "resolution",
            helper::matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                videoDetails.value(StreamDetails::VideoDetails::ScanType)));
        Renamer::replaceCondition(newFolderName, "bluray", isBluRay);
        Renamer::replaceCondition(newFolderName, "dvd", isDvd);
        Renamer::replaceCondition(
            newFolderName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
        Renamer::replaceCondition(newFolderName, "movieset", movie.set().name);
        Renamer::replaceCondition(newFolderName, "imdbId", movie.imdbId().toString());
        Renamer::replaceCondition(newFolderName, "tmdbId", movie.tmdbId().toString());

        // Sanitize + Replace Delimiter with the one chosen by the user
        helper::sanitizeFolderName(newFolderName, newDelimiter);

        if (dir.dirName() != newFolderName) {
            renameRow = m_dialog->addResultToTable(dir.dirName(), newFolderName, RenameOperation::Rename);
        }
    }
    // create dir for new dir structure
    else if (m_config.renameDirectories) {
        Renamer::replace(newFolderName, "title", movie.name());
        Renamer::replace(newFolderName, "extension", extension);
        Renamer::replace(
            newFolderName, "originalTitle", movie.originalName().isEmpty() ? movie.name() : movie.originalName());
        Renamer::replace(newFolderName, "sortTitle", movie.sortTitle());
        Renamer::replace(newFolderName, "director", movie.director());
        // TODO: Let the user decide whether only the first should be used or
        //       if a space should be the separator.
        Renamer::replace(newFolderName, "studio", movie.studios().join(","));
        Renamer::replace(newFolderName, "year", movie.released().toString("yyyy"));
        Renamer::replace(newFolderName, "videoCodec", movie.streamDetails()->videoCodec());
        Renamer::replace(newFolderName, "audioCodec", movie.streamDetails()->audioCodec());
        // TODO: Let the user decide whether only the first should be used or
        //       if a space should be the separator.
        Renamer::replace(newFolderName, "audioLanguage", movie.streamDetails()->allAudioLanguages().join("-"));
        // TODO: Let the user decide whether only the first should be used or
        //       if a space should be the separator.
        Renamer::replace(newFolderName, "subtitleLanguage", movie.streamDetails()->allSubtitleLanguages().join("-"));
        Renamer::replace(newFolderName, "channels", QString::number(movie.streamDetails()->audioChannels()));
        Renamer::replace(newFolderName,
            "resolution",
            helper::matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                videoDetails.value(StreamDetails::VideoDetails::ScanType)));
        Renamer::replaceCondition(newFolderName, "bluray", isBluRay);
        Renamer::replaceCondition(newFolderName, "dvd", isDvd);
        Renamer::replaceCondition(
            newFolderName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
        Renamer::replaceCondition(newFolderName, "movieset", movie.set().name);
        Renamer::replaceCondition(newFolderName, "imdbId", movie.imdbId().toString());
        Renamer::replaceCondition(newFolderName, "tmdbId", movie.tmdbId().toString());

        // Sanitize + Replace Delimiter with the one chosen by the user
        helper::sanitizeFolderName(newFolderName, newDelimiter);

        if (dir.dirName() != newFolderName) { // check if movie is not already on good folder
            int i = 0;
            while (dir.exists(newFolderName)) {
                newFolderName = newFolderName + " " + QString::number(++i);
            }

            if (!m_config.dryRun) {
                const int row = m_dialog->addResultToTable(dir.dirName(), newFolderName, RenameOperation::CreateDir);
                if (!dir.mkdir(newFolderName)) {
                    m_dialog->setResultStatus(row, RenameResult::Failed);
                    return RenameError::Error;
                }
                newMovieFolder = dir.path() + "/" + newFolderName;
            }

            for (const QString& fileName : FilmFiles) {
                QFileInfo fi(fileName);
                if (dir.dirName() != newFolderName) {
                    const int row = m_dialog->addResultToTable(fi.fileName(),
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
            errorOccurred = true;
        } else {
            newMovieFolder = parentDir.path() + "/" + newFolderName;
        }
    }

    if (!errorOccurred && !m_config.dryRun) {
        QStringList files;
        for (const QString& file : newMovieFiles) {
            QString f = newMovieFolder;
            if (isBluRay || isDvd) {
                f += "/" + parentDirName;
            }
            f += "/" + file;
            files << f;
        }
        movie.setFiles(files);
        mediaelch::MoviePersistence persistence{*Manager::instance()->database()};
        persistence.update(&movie);
    }

    if (errorOccurred) {
        return RenameError::Error;
    }
    return RenameError::None;
}
