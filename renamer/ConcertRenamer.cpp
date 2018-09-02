#include "ConcertRenamer.h"

#include "data/Concert.h"
#include "data/MediaCenterInterface.h"
#include "globals/Helper.h"
#include "globals/Manager.h"

#include <QDir>
#include <QFileInfo>

ConcertRenamer::ConcertRenamer(RenamerConfig renamerConfig, RenamerDialog *dialog) : Renamer(renamerConfig, dialog)
{
}

ConcertRenamer::RenameError ConcertRenamer::renameConcert(Concert &concert)
{
    QFileInfo fi(concert.files().first());
    QString fiCanonicalPath = fi.canonicalPath();
    QDir dir(fi.canonicalPath());
    QString newFolderName = m_config.directoryPattern;
    QString newFileName;
    QStringList newConcertFiles;
    QString parentDirName;

    bool errorOccured = false;

    for (const QString &file : concert.files()) {
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

    if (!isBluRay && !isDvd && m_config.renameFiles) {
        newConcertFiles.clear();
        int partNo = 0;
        const auto videoDetails = concert.streamDetails()->videoDetails();
        for (const QString &file : concert.files()) {
            newFileName = (concert.files().count() == 1) ? m_config.filePattern : m_config.filePatternMulti;
            QFileInfo fi(file);
            QString baseName = fi.completeBaseName();
            QDir currentDir = fi.dir();

            Renamer::replace(newFileName, "title", concert.name());
            Renamer::replace(newFileName, "artist", concert.artist());
            Renamer::replace(newFileName, "album", concert.album());
            Renamer::replace(newFileName, "year", concert.released().toString("yyyy"));
            Renamer::replace(newFileName, "extension", fi.suffix());
            Renamer::replace(newFileName, "partNo", QString::number(++partNo));
            Renamer::replace(newFileName, "videoCodec", concert.streamDetails()->videoCodec());
            Renamer::replace(newFileName, "audioCodec", concert.streamDetails()->audioCodec());
            Renamer::replace(newFileName, "channels", QString::number(concert.streamDetails()->audioChannels()));
            Renamer::replace(newFileName,
                "resolution",
                Helper::instance()->matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                    videoDetails.value(StreamDetails::VideoDetails::ScanType)));
            Renamer::replaceCondition(
                newFileName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
            Helper::instance()->sanitizeFileName(newFileName);
            if (fi.fileName() != newFileName) {
                int row = m_dialog->addResultToTable(fi.fileName(), newFileName, Renamer::RenameOperation::Rename);
                if (!m_config.dryRun) {
                    if (!Renamer::rename(file, fi.canonicalPath() + "/" + newFileName)) {
                        m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
                        errorOccured = true;
                        continue;
                    } else {
                        newConcertFiles.append(newFileName);
                    }
                }

                QStringList filters;
                for (const QString &extra : m_extraFiles) {
                    filters << baseName + extra;
                }
                for (const QString &subFileName : currentDir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot)) {
                    QString subSuffix = subFileName.mid(baseName.length());
                    QString newBaseName = newFileName.left(newFileName.lastIndexOf("."));
                    QString newSubName = newBaseName + subSuffix;
                    int row = m_dialog->addResultToTable(subFileName, newSubName, Renamer::RenameOperation::Rename);
                    if (!m_config.dryRun) {
                        if (!Renamer::rename(currentDir.canonicalPath() + "/" + subFileName,
                                currentDir.canonicalPath() + "/" + newSubName)) {
                            m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
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
            QString newDataFileName = files.first().saveFileName(newFileName, -1, concert.files().count() > 1);
            Helper::instance()->sanitizeFileName(newDataFileName);
            if (newDataFileName == fileName) {
                // File already has correct name
                return;
            }

            int row = m_dialog->addResultToTable(fileName, newDataFileName, Renamer::RenameOperation::Rename);
            if (m_config.dryRun) {
                return;
            }

            if (!Renamer::rename(filePath, fiCanonicalPath + "/" + newDataFileName)) {
                m_dialog->setResultStatus(row, Renamer::RenameResult::Failed);
            }
        };

        MediaCenterInterface *mediaCenter = Manager::instance()->mediaCenterInterface();
        const auto renameImageType = [&](ImageType imageType) {
            DataFileType fileType = DataFile::dataFileTypeForImageType(imageType);
            renameFileType(mediaCenter->imageFileName(&concert, imageType), fileType);
        };

        renameFileType(mediaCenter->nfoFilePath(&concert), DataFileType::ConcertNfo);

        renameImageType(ImageType::ConcertPoster);
        renameImageType(ImageType::ConcertBackdrop);
    }

    int renameRow = -1;
    if (m_config.renameDirectories && concert.inSeparateFolder()) {
        const auto videoDetails = concert.streamDetails()->videoDetails();
        Renamer::replace(newFolderName, "title", concert.name());
        Renamer::replace(newFolderName, "artist", concert.artist());
        Renamer::replace(newFolderName, "album", concert.album());
        Renamer::replace(newFolderName, "year", concert.released().toString("yyyy"));
        Renamer::replaceCondition(newFolderName, "bluray", isBluRay);
        Renamer::replaceCondition(newFolderName, "dvd", isDvd);
        Renamer::replaceCondition(
            newFolderName, "3D", videoDetails.value(StreamDetails::VideoDetails::StereoMode) != "");
        Renamer::replace(newFolderName, "videoCodec", concert.streamDetails()->videoCodec());
        Renamer::replace(newFolderName, "audioCodec", concert.streamDetails()->audioCodec());
        Renamer::replace(newFolderName, "channels", QString::number(concert.streamDetails()->audioChannels()));
        Renamer::replace(newFolderName,
            "resolution",
            Helper::instance()->matchResolution(videoDetails.value(StreamDetails::VideoDetails::Width).toInt(),
                videoDetails.value(StreamDetails::VideoDetails::Height).toInt(),
                videoDetails.value(StreamDetails::VideoDetails::ScanType)));
        Helper::instance()->sanitizeFileName(newFolderName);
        if (dir.dirName() != newFolderName) {
            renameRow = m_dialog->addResultToTable(dir.dirName(), newFolderName, Renamer::RenameOperation::Rename);
        }
    }

    QString newConcertFolder = dir.path();
    if (!m_config.dryRun && dir.dirName() != newFolderName && m_config.renameDirectories
        && concert.inSeparateFolder()) {
        QDir parentDir(dir.path());
        parentDir.cdUp();
        if (!Renamer::rename(dir, parentDir.path() + "/" + newFolderName)) {
            m_dialog->setResultStatus(renameRow, Renamer::RenameResult::Failed);
            errorOccured = true;
        } else {
            newConcertFolder = parentDir.path() + "/" + newFolderName;
        }
    }

    if (!errorOccured && !m_config.dryRun) {
        QStringList files;
        for (const QString &file : newConcertFiles) {
            QString f = newConcertFolder;
            if (isBluRay || isDvd) {
                f += "/" + parentDirName;
            }
            f += "/" + file;
            files << f;
        }
        concert.setFiles(files);
        Manager::instance()->database()->update(&concert);
    }


    if (errorOccured) {
        return RenameError::Error;
    }
    return RenameError::None;
}
