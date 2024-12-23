#include "MovieRenamer.h"

#include "RenamerPlaceholders.h"
#include "RenamerUtils.h"
#include "data/movie/Movie.h"
#include "database/MoviePersistence.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "log/Log.h"
#include "media_center/MediaCenterInterface.h"

#include <QDir>
#include <QFileInfo>

namespace mediaelch {

MovieRenamerPlaceholders::~MovieRenamerPlaceholders() = default;

QVector<Placeholder> MovieRenamerPlaceholders::placeholders()
{
    return {
        // clang-format off
        { "extension",        true,  false, QObject::tr("File extension") },
        { "bluray",           false, true,  QObject::tr("File/directory is BluRay") },
        { "dvd",              false, true,  QObject::tr("File/directory is DVD") },
        { "partNo",           true,  false, QObject::tr("Part number of the current file") },
        { "title",            true,  false, QObject::tr("Title") },
        { "originalTitle",    true,  false, QObject::tr("Original Title") },
        { "sortTitle",        true,  false, QObject::tr("Sort Title") },
        { "director",         true,  false, QObject::tr("Director(s)") },
        { "studio",           true,  false, QObject::tr("Studio(s) (separated by a comma)") },
        { "year",             true,  false, QObject::tr("Year") },
        { "videoCodec",       true,  false, QObject::tr("Video Codec") },
        { "audioLanguage",    true,  false, QObject::tr("Audio Language(s) (separated by a minus)") },
        { "subtitleLanguage", true,  false, QObject::tr("Subtitle Language(s) (separated by a minus)") },
        { "audioCodec",       true,  false, QObject::tr("Audio Codec") },
        { "channels",         true,  false, QObject::tr("Number of audio channels") },
        { "resolution",       true,  false, QObject::tr("Resolution (1080p, 720p, ...)") },
        { "3D",               false, true,  QObject::tr("File is 3D") },
        { "movieset",         true,  true,  QObject::tr("Movie set name") },
        { "imdbId",           true,  true,  QObject::tr("IMDb ID") },
        { "tmdbId",           true,  true,  QObject::tr("TMDB ID") },
        // clang-format on
    };
}

MovieRenamerData::~MovieRenamerData() = default;

ELCH_NODISCARD QString MovieRenamerData::value(const QString& name) const
{
    const QMap<QString, std::function<QString()>> map = {
        // clang-format off
        {"extension",        [this]() { return m_extension; }},
        {"imdbId",           [this]() { return m_movie.imdbId().toString(); }},
        {"tmdbId",           [this]() { return m_movie.tmdbId().toString(); }},
        {"partNo",           [this]() { return QString::number(m_partNo); }},
        {"title",            [this]() { return m_movie.name(); }},
        {"originalTitle",    [this]() { return m_movie.originalName().isEmpty() ? m_movie.name() : m_movie.originalName(); }},
        {"sortTitle",        [this]() { return m_movie.sortTitle(); }},
        {"director",         [this]() { return m_movie.director(); }},
        {"studio",           [this]() { return m_movie.studios().join(","); }},
        {"year",             [this]() { return m_movie.released().toString("yyyy"); }},
        {"videoCodec",       [this]() { return m_movie.streamDetails()->videoCodec(); }},
        {"audioLanguage",    [this]() { return m_movie.streamDetails()->allAudioLanguages().join("-"); }},
        {"subtitleLanguage", [this]() { return m_movie.streamDetails()->allSubtitleLanguages().join("-"); }},
        {"audioCodec",       [this]() { return m_movie.streamDetails()->audioCodec(); }},
        {"channels",         [this]() { return QString::number(m_movie.streamDetails()->audioChannels()); }},
        {"movieset",         [this]() { return m_movie.set().name; }},
        {"resolution",       [this]() {
            return helper::matchResolution(m_videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                m_videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                m_videoDetails.value(StreamDetails::VideoDetails::ScanType));
        }},
        // clang-format on
    };

    MediaElch_Ensure_Data_Matches_Placeholders(MovieRenamerPlaceholders, map);

    if (map.contains(name)) {
        return map[name]();
    }
    qCCritical(generic) << "MovieRenamerData::value: Unknown tag:" << name;
    MediaElch_Debug_Unreachable();
    return "";
}

ELCH_NODISCARD bool MovieRenamerData::passesCondition(const QString& name) const
{
    const QMap<QString, std::function<bool()>> map = {
        // clang-format off
        {"bluray", [this]() { return m_isBluRay; }},
        {"dvd",    [this]() { return m_isDvd; }},
        {"3D",     [this]() { return m_videoDetails.value(StreamDetails::VideoDetails::StereoMode) != ""; }},
        // clang-format on
    };

    MediaElch_Ensure_Condition_Matches_Placeholders(MovieRenamerPlaceholders, map);

    return map.contains(name) //
               ? map[name]()
               : !value(name).isEmpty();
};

} // namespace mediaelch

MovieRenamer::MovieRenamer(RenamerConfig renamerConfig, RenamerDialog* dialog) : Renamer(renamerConfig, dialog)
{
}

MovieRenamer::RenameError MovieRenamer::renameMovie(Movie& movie)
{
    mediaelch::MovieRenamerPlaceholders renamerPlaceholder;
    mediaelch::MovieRenamerData renamerData{movie};

    QFileInfo movieInfo(movie.files().first().toString());
    QString fiCanonicalPath = movieInfo.canonicalPath();
    QDir dir(movieInfo.canonicalPath());
    QString newFolderName = m_config.directoryPattern;

    bool replaceDelimiter = m_config.replaceDelimiter;
    QString oldDelimiter = " ";
    QString newDelimiter = (replaceDelimiter) ? m_config.delimiter : oldDelimiter;

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

    renamerData.setIsBluRay(isBluRay);
    renamerData.setIsDvd(isDvd);

    // BluRay and DVD folder content must not be renamed.
    if (isBluRay || isDvd) {
        parentDirName = dir.dirName();
        dir.cdUp();
    }

    if (!isBluRay && !isDvd && m_config.renameFiles) {
        newMovieFiles.clear();
        int partNo = 0;
        const auto videoDetails = movie.streamDetails()->videoDetails();
        renamerData.setVideoDetails(videoDetails);
        const auto& files = movie.files();
        for (const mediaelch::FilePath& file : files) {
            newFileName = (files.count() == 1) ? m_config.filePattern : m_config.filePatternMulti;
            QFileInfo fi(file.toString());
            QString baseName = fi.completeBaseName();
            QDir currentDir = fi.dir();

            renamerData.setPartNo(++partNo);
            renamerData.setExtension(fi.suffix());
            newFileName = renamerPlaceholder.replace(newFileName, renamerData);

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
        renamerData.setPartNo(0);
        renamerData.setExtension("");
        newFileName = renamerPlaceholder.replace(newFolderName, renamerData);

        // Sanitize + Replace Delimiter with the one chosen by the user
        helper::sanitizeFolderName(newFolderName, newDelimiter);

        if (dir.dirName() != newFolderName) {
            renameRow = m_dialog->addResultToTable(dir.dirName(), newFolderName, RenameOperation::Rename);
        }
    }
    // create dir for new dir structure
    else if (m_config.renameDirectories) {
        renamerData.setPartNo(0);
        renamerData.setExtension("");
        newFileName = renamerPlaceholder.replace(newFolderName, renamerData);

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
