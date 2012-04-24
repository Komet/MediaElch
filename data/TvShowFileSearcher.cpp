#include "TvShowFileSearcher.h"

#include <QFileInfo>
#include "Manager.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "data/TvShowModelItem.h"

TvShowFileSearcher::TvShowFileSearcher(QObject *parent) :
    QThread(parent)
{
    m_progressMessageId = Constants::TvShowSearcherProgressMessageId;
}

void TvShowFileSearcher::setMovieDirectories(QStringList directories)
{
    m_directories.clear();
    foreach (const QString &path, directories) {
        QFileInfo fi(path);
        if (fi.isDir()) {
            m_directories.append(path);
        }
    }
}

void TvShowFileSearcher::run()
{
    emit searchStarted(tr("Searching for TV Shows..."), m_progressMessageId);

    Manager::instance()->tvShowModel()->clear();
    QMap<QString, QList<QStringList> > contents;
    foreach (const QString &path, m_directories) {
        this->getDirContents(path, contents);
    }

    int i=0;
    int n=0;
    QMapIterator<QString, QList<QStringList> > it(contents);
    while (it.hasNext()) {
        it.next();
        n += it.value().size();
    }
    it.toFront();
    while (it.hasNext()) {
        it.next();
        TvShow *show = new TvShow(it.key());
        show->loadData(Manager::instance()->mediaCenterInterface());
        TvShowModelItem *item = Manager::instance()->tvShowModel()->appendChild(show);
        foreach (const QStringList &files, it.value()) {
            TvShowEpisode *episode = new TvShowEpisode(files, show);
            episode->loadData(Manager::instance()->mediaCenterInterface());
            show->addEpisode(episode);
            item->appendChild(episode);
            emit progress(++i, n, m_progressMessageId);
        }
        show->moveToMainThread();
    }
    emit tvShowsLoaded(m_progressMessageId);
}

void TvShowFileSearcher::getDirContents(QString path, QMap<QString, QList<QStringList> > &contents)
{
    QDir dir(path);
    foreach (const QString &cDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDir subDir(path + QDir::separator() + cDir);
        QStringList filters;
        QStringList files;
        filters << "*.mkv" << "*.avi" << "*.mpg" << "*.mpeg";
        foreach (const QString &file, subDir.entryList(filters, QDir::Files | QDir::System)) {
            files.append(file);
        }
        files.sort();

        QRegExp rx("((part|cd)[\\s_]*)(\\d+)", Qt::CaseInsensitive);
        for (int i=0, n=files.size() ; i<n ; ++i) {
            QStringList tvShowFiles;
            QString file = files.at(i);
            if (file.isEmpty())
                continue;
            tvShowFiles << subDir.path() + QDir::separator() + file;
            int pos = rx.indexIn(file);
            if (pos != -1) {
                QString left = file.left(pos) + rx.cap(1);
                QString right = file.mid(pos+rx.cap(1).size()+rx.cap(2).size());
                for (int x=0 ; x<n ; x++) {
                    QString subFile = files.at(x);
                    if (subFile != file) {
                        if (subFile.startsWith(left) && subFile.endsWith(right)) {
                            tvShowFiles << path + QDir::separator() + subFile;
                            files[x] = ""; // set an empty file name, this way we can skip this file in the main loop
                        }
                    }
                }
            }
            if (contents.contains(subDir.path())) {
                contents[subDir.path()].append(tvShowFiles);
            } else {
                QList<QStringList> l;
                l << tvShowFiles;
                contents.insert(subDir.path(), l);
            }
        }
    }
}
