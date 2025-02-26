#include "src/file_search/MovieFilesOrganizer.h"

#include "file_search/movie/MovieDirScan.h"
#include "log/Log.h"
#include "media/NameFormatter.h"
#include "settings/Settings.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>

MovieFilesOrganizer::MovieFilesOrganizer(QObject* parent) : QThread(parent)
{
}

/**
 * \brief moves all movies in given path to separate directories
 * \param dir place to organize
 */
void MovieFilesOrganizer::moveToDirs(mediaelch::DirectoryPath dir)
{
    QString path = dir.toNativePathString();
    QFileInfo fi(path);
    if (!fi.isDir()) {
        canceled(tr("Source %1 is no directory").arg(path));
        return;
    }

    QVector<QStringList> contents;
    auto* fileSearcher = new mediaelch::MovieDirScan(this);
    fileSearcher->scanDir(path, path, contents, false, true);
    fileSearcher->deleteLater();

    const auto pos = path.lastIndexOf(QDir::separator());
    QString dirName = path.right(path.length() - pos - 1);
    QString fileName;

    NameFormatter::setExcludeWords(Settings::instance()->excludeWords());

    for (const QStringList& movie : asConst(contents)) {
        const auto movieIndex = movie.at(0).lastIndexOf(QDir::separator());
        if (!(movie.at(0).left(movieIndex).endsWith(dirName))) {
            qCDebug(generic) << "[MovieFilesOrganizer] skipping " << movie.at(0);
            continue;
        }

        fi.setFile(movie.at(0));
        fileName = fi.completeBaseName();
        QDir dir2;

        QString newFolder;
        if (movie.length() == 1) {
            newFolder = path + QDir::separator() + NameFormatter::formatName(fileName);
        } else if (movie.length() > 1) {
            newFolder = path + QDir::separator() + NameFormatter::formatName(NameFormatter::removeParts(fileName));
        } else {
            continue;
        }

        if (!(dir2.mkdir(newFolder))) {
            continue;
        }

        for (const QString& file : movie) {
            if (!dir2.rename(file,
                    newFolder + QDir::separator()
                        + file.right(file.length() - file.lastIndexOf(QDir::separator()) - 1))) {
                qCWarning(generic) << "Moving " << file << "to " << newFolder << " failed.";
            }
        }
    }
}

/**
 * \brief Prints an error message, that tells why the
 * foldering process has been canceled
 * \param msg message with reason
 */
void MovieFilesOrganizer::canceled(QString msg)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(tr("Operation not possible."));
    msgBox.setInformativeText(tr("%1\n Operation Canceled.").arg(msg));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}
