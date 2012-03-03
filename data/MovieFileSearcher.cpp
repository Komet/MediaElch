#include "MovieFileSearcher.h"

#include <QApplication>
#include <QDebug>
#include "Manager.h"

MovieFileSearcher::MovieFileSearcher(QObject *parent) :
    QThread(parent)
{
}

MovieFileSearcher::~MovieFileSearcher()
{
}

void MovieFileSearcher::run()
{
    emit searchStarted();

    Manager::instance()->movieModel()->clear();
    QList<QStringList> contents;
    foreach (const QString &path, m_directories) {
        this->getDirContents(path, contents);
    }

    int i=0;
    int n=contents.size();
    foreach (const QStringList &files, contents) {
        Movie *movie = new Movie(files);
        movie->loadData(Manager::instance()->mediaCenterInterface());
        Manager::instance()->movieModel()->addMovie(movie);
        emit progress(++i, n);
    }

    emit moviesLoaded();
}

void MovieFileSearcher::setMovieDirectories(QStringList directories)
{
    m_directories.clear();
    foreach (const QString &path, directories) {
        QFileInfo fi(path);
        if (fi.isDir()) {
            m_directories.append(path);
        }
    }
}

void MovieFileSearcher::getDirContents(QString path, QList<QStringList> &contents)
{
    QDir dir(path);
    foreach (const QString &cDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        this->getDirContents(path + QDir::separator() + cDir, contents);
    }

    QStringList filters;
    QStringList dvdFilters;
    QStringList files;
    filters << "*.mkv" << "*.avi" << "*.mpg" << "*.mpeg" << "VIDEO_TS.IFO";
    dvdFilters << "*.VOB" << "*.IFO" << "*.BUP";
    foreach (const QString &file, dir.entryList(filters, QDir::Files | QDir::System)) {
        files.append(file);
    }
    files.sort();

    QRegExp rx("((part|cd)[\\s_]*)(\\d+)", Qt::CaseInsensitive);
    for (int i=0, n=files.size() ; i<n ; i++) {
        QStringList movieFiles;
        QString file = files.at(i);
        if (file.isEmpty())
            continue;

        movieFiles << path + QDir::separator() + file;

        if (file == "VIDEO_TS.IFO") {
            foreach (const QString &dvdFile, dir.entryList(dvdFilters, QDir::Files | QDir::System)) {
                if (dvdFile != "VIDEO_TS.IFO")
                    movieFiles.append(path + QDir::separator() + dvdFile);
            }
        } else {
            int pos = rx.indexIn(file);
            if (pos != -1) {
                QString left = file.left(pos) + rx.cap(1);
                QString right = file.mid(pos+rx.cap(1).size()+rx.cap(2).size());
                for (int x=0 ; x<n ; x++) {
                    QString subFile = files.at(x);
                    if (subFile != file) {
                        if (subFile.startsWith(left) && subFile.endsWith(right)) {
                            movieFiles << path + QDir::separator() + subFile;
                            files[x] = ""; // set an empty file name, this way we can skip this file in the main loop
                        }
                    }
                }
            }
        }

        contents.append(movieFiles);
    }
}
