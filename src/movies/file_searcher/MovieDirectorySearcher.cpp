#include "MovieDirectorySearcher.h"

#include "file/FilenameUtils.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"

#include "file/FilenameUtils.h"

#include <QFutureWatcher>
#include <QtConcurrent>

namespace mediaelch {

MovieDirectorySearcher::MovieDirectorySearcher(const SettingsDir& dir, bool inSeparateFolders, QObject* parent) :
    QObject(parent), m_dir{dir}, m_inSeparateFolders{inSeparateFolders}
{
}

void MovieDirectorySearcher::load()
{
    if (m_aborted.load()) {
        return;
    }

    Manager::instance()->database()->clearMoviesInDirectory(m_dir.path);

    // No filter, no media files...
    if (!Settings::instance()->advanced()->movieFilters().hasFilter()) {
        emit loaded(this);
        return;
    }

    qDebug() << "[MovieDirectorySearcher] Scanning directory:" << QDir::toNativeSeparators(m_dir.path.path());
    loadMovieContents();

    const int approximateMovieCount = m_inSeparateFolders ? m_contents.size() : 0;
    emit startLoading(approximateMovieCount);

    qDebug() << "[MovieDirectorySearcher] Creating movies for" << QDir::toNativeSeparators(m_dir.path.path());
    createMovies();
}

void MovieDirectorySearcher::abort()
{
    m_aborted.store(true);
    m_watcher.cancel();
    m_watcher.waitForFinished();
}

void MovieDirectorySearcher::loadMovieContents()
{
    QDirIterator it(m_dir.path.path(),
        Settings::instance()->advanced()->movieFilters().filters(),
        QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files,
        QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    QString lastDir;

    while (it.hasNext()) {
        if (m_aborted.load()) {
            // 0, because "contents" isn't stored, yet
            emit loaded(this);
            return;
        }
        it.next();

        QString dirName = it.fileInfo().dir().dirName();
        QString fileName = it.fileName(); // may actually be a directory name

        const bool isFile = it.fileInfo().isFile();
        const bool isDir = it.fileInfo().isDir();
        bool isSpecialDir = false; // set to true for DVD or BluRay Structure

        if (isFile && Settings::instance()->advanced()->isFileExcluded(fileName)) {
            continue;
        }

        // TODO: If there is a BluRay structure then the directory filter may not work
        // because BDMV's parent directory is not listed.
        if ((isDir && Settings::instance()->advanced()->isFolderExcluded(fileName))
            || Settings::instance()->advanced()->isFolderExcluded(dirName)) {
            continue;
        }

        // Skips Extras files
        if (isFile
            && (fileName.contains("-trailer", Qt::CaseInsensitive)            //
                || fileName.contains("-sample", Qt::CaseInsensitive)          //
                || fileName.contains("-behindthescenes", Qt::CaseInsensitive) //
                || fileName.contains("-deleted", Qt::CaseInsensitive)         //
                || fileName.contains("-featurette", Qt::CaseInsensitive)      //
                || fileName.contains("-interview", Qt::CaseInsensitive)       //
                || fileName.contains("-scene", Qt::CaseInsensitive)           //
                || fileName.contains("-short", Qt::CaseInsensitive))) {
            continue;
        }

        // Skip actors folder and all files inside it
        if (QString::compare(".actors", dirName, Qt::CaseInsensitive) == 0) {
            continue;
        }

        // Skip extras folder and all files inside it
        if (QString::compare("extras", dirName, Qt::CaseInsensitive) == 0) {
            continue;
        }

        // Skip extra fanarts folder and all files inside it
        if (QString::compare("extrafanart", dirName, Qt::CaseInsensitive) == 0) {
            continue;
        }

        // Skip extra thumbs folder and all files inside it
        if (QString::compare("extrathumbs", dirName, Qt::CaseInsensitive) == 0) {
            continue;
        }

        // Skip BluRay backup folder
        if (QString::compare("backup", dirName, Qt::CaseInsensitive) == 0
            && QString::compare("index.bdmv", fileName, Qt::CaseInsensitive) == 0) {
            continue;
        }

        if (dirName != lastDir) {
            lastDir = dirName;
            if (m_contents.count() % 20 == 0) {
                // TODO  emit currentDir(dirName);
            }
        }

        if (isFile && QString::compare("index.bdmv", fileName, Qt::CaseInsensitive) == 0) {
            qDebug() << "[MovieDirectorySearcher] Found BluRay structure";
            QDir bluRayDir(it.fileInfo().dir());
            if (QString::compare(bluRayDir.dirName(), "BDMV", Qt::CaseInsensitive) == 0) {
                bluRayDir.cdUp();
            }
            m_bluRayDirectories << bluRayDir.path();
            isSpecialDir = true;
        }
        if (QString::compare("VIDEO_TS.IFO", fileName, Qt::CaseInsensitive) == 0) {
            qDebug() << "[MovieDirectorySearcher] Found DVD structure";
            QDir videoDir(it.fileInfo().dir());
            if (QString::compare(videoDir.dirName(), "VIDEO_TS", Qt::CaseInsensitive) == 0) {
                videoDir.cdUp();
            }
            m_dvdDirectories << videoDir.path();
            isSpecialDir = true;
        }

        const QString dirPath = it.fileInfo().path();
        if (!m_contents.contains(dirPath)) {
            m_contents.insert(dirPath, {});
        }
        if (isFile || isSpecialDir) {
            m_contents[dirPath].append(it.filePath());
            m_lastModifications.insert(it.filePath(), it.fileInfo().lastModified());
        }
    }
}

void MovieDirectorySearcher::createMovies()
{
    std::function<QVector<Movie*>(QStringList files)> fct = [this](QStringList files) -> QVector<Movie*> {
        return createMovie(std::move(files));
    };

    connect(&m_watcher, &QFutureWatcher<QVector<Movie*>>::finished, this, [this]() { emit loaded(this); });
    connect(&m_watcher, &QFutureWatcher<QVector<Movie*>>::resultReadyAt, this, [this](int index) {
        // Called in the main thread, so no need for a mutex.
        const QVector<Movie*> movies = m_watcher.resultAt(index);
        for (Movie* movie : movies) {
            postProcessMovie(movie);
        }
    });
    QFuture<QVector<Movie*>> future = QtConcurrent::mapped(m_contents, fct);
    m_watcher.setFuture(future);
}

QVector<Movie*> MovieDirectorySearcher::createMovie(QStringList files)
{
    if (m_aborted.load()) {
        return {};
    }

    QVector<Movie*> movies;

    DiscType discType = DiscType::Single;

    // BluRay handling
    for (const QString& path : asConst(m_bluRayDirectories)) {
        if (!files.isEmpty() && (files.first().startsWith(path + "/") || files.first().startsWith(path + "\\"))) {
            QStringList f;
            for (const QString& file : files) {
                if (file.endsWith("index.bdmv", Qt::CaseInsensitive)) {
                    f.append(file);
                }
            }
            files = f;
            discType = DiscType::BluRay;
            qDebug() << "It's a BluRay structure";
        }
    }

    // DVD handling
    for (const QString& path : asConst(m_dvdDirectories)) {
        if (!files.isEmpty() && (files.first().startsWith(path + "/") || files.first().startsWith(path + "\\"))) {
            QStringList f;
            for (const QString& file : asConst(files)) {
                if (file.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
                    f.append(file);
                }
            }
            files = f;
            discType = DiscType::Dvd;
            qDebug() << "It's a DVD structure";
        }
    }

    if (files.isEmpty()) {
        return {};
    }

    if (files.count() == 1 || m_inSeparateFolders) {
        // single file or in separate folder
        mediaelch::file::sortFilenameList(files);
        auto* movie = new Movie(files);
        movie->setInSeparateFolder(m_inSeparateFolders);
        movie->setFileLastModified(m_lastModifications.value(files.at(0)));
        movie->setDiscType(discType);
        movie->setLabel(Manager::instance()->database()->getLabel(movie->files()));
        movie->setChanged(false);
        movie->controller()->loadData(Manager::instance()->mediaCenterInterface());
        if (discType == DiscType::Single) {
            QFileInfo mFi(files.first());
            const QList<QFileInfo> subFiles = mFi.dir().entryInfoList(
                QStringList{"*.sub", "*.srt", "*.smi", "*.ssa"}, QDir::Files | QDir::NoDotAndDotDot);
            for (const QFileInfo& subFi : subFiles) {
                QString subFileName = subFi.fileName().mid(mFi.completeBaseName().length() + 1);
                QStringList parts = subFileName.split(QRegExp(R"(\s+|\-+|\.+)"));
                if (parts.isEmpty()) {
                    continue;
                }
                parts.takeLast();

                QStringList subFiles = QStringList() << subFi.fileName();
                if (QString::compare(subFi.suffix(), "sub", Qt::CaseInsensitive) == 0) {
                    QFileInfo subIdxFi(subFi.absolutePath() + "/" + subFi.completeBaseName() + ".idx");
                    if (subIdxFi.exists()) {
                        subFiles << subIdxFi.fileName();
                    }
                }
                auto* subtitle = new Subtitle(movie);
                subtitle->setFiles(subFiles);
                if (parts.contains("forced", Qt::CaseInsensitive)) {
                    subtitle->setForced(true);
                    parts.removeAll("forced");
                }
                if (!parts.isEmpty()) {
                    subtitle->setLanguage(parts.first());
                }
                subtitle->setChanged(false);
                movie->addSubtitle(subtitle, true);
            }
        }

        // This method is called in parallel. Move it to the main object's thread.
        movie->moveToThread(thread());
        movies << movie;

    } else {
        QMap<QString, QStringList> stacked;
        while (!files.isEmpty()) {
            QString file = files.takeLast();

            QString stackedBase = mediaelch::file::stackedBaseName(file);
            stacked.insert(stackedBase, {file});

            for (int fileIndex = 0; fileIndex < files.count();) {
                const QString& f = files[fileIndex];

                if (mediaelch::file::stackedBaseName(f) == stackedBase) {
                    stacked[stackedBase].append(f);
                    files.removeAt(fileIndex);
                } else {
                    fileIndex++;
                }
            }
        }
        QMapIterator<QString, QStringList> it(stacked);
        while (it.hasNext()) {
            it.next();
            if (it.value().isEmpty()) {
                continue;
            }
            QStringList stackedFiles = it.value();
            stackedFiles.sort();
            auto* movie = new Movie(stackedFiles);
            movie->setInSeparateFolder(m_inSeparateFolders);
            movie->setFileLastModified(m_lastModifications.value(it.value().at(0)));
            movie->controller()->loadData(Manager::instance()->mediaCenterInterface());
            movie->setLabel(Manager::instance()->database()->getLabel(movie->files()));
            // This method is called in parallel. Move it to the main object's thread.
            movie->moveToThread(thread());
            movies << movie;
        }
    }
    return movies;
}

void MovieDirectorySearcher::postProcessMovie(Movie* movie)
{
    m_movies.push_back(movie);
    emit movieProcessed(movie);
}

QStringList MovieDirectorySearcher::getFiles(QString path)
{
    const auto& filters = Settings::instance()->advanced()->movieFilters();
    QStringList files;

    const QStringList filteredFiles = filters.files(QDir(path));
    for (const QString& file : filteredFiles) {
        m_lastModifications.insert(
            QDir::toNativeSeparators(path + "/" + file), QFileInfo(path + QDir::separator() + file).lastModified());
        files.append(file);
    }
    return files;
}

} // namespace mediaelch
