#include "MovieFilesOrganizer.h"
#include "MovieFileSearcher.h"
#include "globals/NameFormatter.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QMessageBox>

MovieFilesOrganizer::MovieFilesOrganizer(QObject* parent) : QThread(parent)
{
}

/**
 * @brief moves all movies in given path to seperate directories
 * @param path place to organize
 */
void MovieFilesOrganizer::moveToDirs(QString path)
{
    path = QDir::toNativeSeparators(path);
    QFileInfo fi(path);
    if (!fi.isDir()) {
        canceled(tr("Source %1 is no directory").arg(path));
        return;
    }

    QVector<QStringList> contents;
    auto fileSearcher = new MovieFileSearcher(this);
    fileSearcher->scanDir(path, path, contents, false, true);
    fileSearcher->deleteLater();

    const int pos = path.lastIndexOf(QDir::separator());
    QString dirName = path.right(path.length() - pos - 1);
    QString fileName;
    NameFormatter* nameFormat = NameFormatter::instance(this);


    for (const QStringList& movie : contents) {
        const int movieIndex = movie.at(0).lastIndexOf(QDir::separator());
        if (!(movie.at(0).left(movieIndex).endsWith(dirName))) {
            qDebug() << "skipping " << movie.at(0);
            continue;
        }

        fi.setFile(movie.at(0));
        fileName = fi.completeBaseName();
        QDir dir;

        QString newFolder;
        if (movie.length() == 1) {
            newFolder = path + QDir::separator() + nameFormat->formatName(fileName);
        } else if (movie.length() > 1) {
            newFolder = path + QDir::separator() + nameFormat->formatName(nameFormat->formatParts(fileName));
        } else {
            continue;
        }

        if (!(dir.mkdir(newFolder))) {
            continue;
        }

        for (const QString& file : movie) {
            if (!dir.rename(file,
                    newFolder + QDir::separator()
                        + file.right(file.length() - file.lastIndexOf(QDir::separator()) - 1))) {
                qDebug() << "Moving " << file << "to " << newFolder << " failed.";
            }
        }
    }
}

/**
 * @brief Prints an error message, that tells why the
 * foldering process has been canceled
 * @param msg message with reason
 */
void MovieFilesOrganizer::canceled(QString msg)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(tr("Operation not possible."));
    msgBox.setInformativeText(tr("%1\n Operation Canceled.").arg(msg));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setButtonText(1, "Ok");
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}
