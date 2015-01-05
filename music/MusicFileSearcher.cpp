#include "MusicFileSearcher.h"

#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include "../globals/Manager.h"

MusicFileSearcher::MusicFileSearcher(QObject *parent) : QObject(parent)
{
    m_progressMessageId = Constants::MusicFileSearcherProgressMessageId;
}

MusicFileSearcher::~MusicFileSearcher()
{
}

void MusicFileSearcher::setMusicDirectories(QList<SettingsDir> directories)
{
    m_directories.clear();
    foreach (SettingsDir dir, directories) {
        QFileInfo fi(dir.path);
        if (fi.isDir())
            m_directories.append(dir);
    }
}

void MusicFileSearcher::reload(bool force)
{
    // @todo: load from database
    m_aborted = false;

    emit searchStarted(tr("Searching for Music..."), m_progressMessageId);
    Manager::instance()->musicModel()->clear();
    Manager::instance()->musicFilesWidget()->renewModel();

    foreach (SettingsDir dir, m_directories) {
        if (m_aborted)
            break;

        qDebug() << "Loading music from dir" << dir.path;
        QDirIterator it(dir.path, QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::FollowSymlinks);
        while (it.hasNext()) {
            if (m_aborted)
                break;

            it.next();
            emit currentDir(it.fileInfo().baseName());
            Artist *artist = new Artist(it.filePath(), this);
            artist->setName(it.fileInfo().baseName());

            MusicModelItem *artistItem = Manager::instance()->musicModel()->appendChild(artist);

            QDirIterator itAlbums(it.filePath(), QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::FollowSymlinks);
            while (itAlbums.hasNext()) {
                itAlbums.next();
                Album *album = new Album(itAlbums.filePath(), this);
                album->setTitle(itAlbums.fileInfo().baseName());
                artistItem->appendChild(album);
            }
        }
    }

    emit currentDir("");
    emit searchStarted(tr("Loading Music..."), m_progressMessageId);

    if (!m_aborted)
        emit musicLoaded(m_progressMessageId);
}

void MusicFileSearcher::abort()
{
    m_aborted = true;
}
