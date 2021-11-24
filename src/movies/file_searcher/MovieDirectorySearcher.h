#pragma once

#include "file/FileFilter.h"
#include "globals/Globals.h"

#include <QMutex>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <atomic>

class Movie;
class Database;

namespace mediaelch {

/// \brief   Thread safe store for movies.
/// \details An instance of this class must be provided when using any MovieLoader.
///          All MovieLoaders move their newly created movies into a store.
class MovieLoaderStore : public QObject
{
    Q_OBJECT
public:
    MovieLoaderStore(QObject* parent = nullptr) : QObject(parent) {}
    ~MovieLoaderStore() override = default;

    void addMovie(Movie* movie);
    void addMovies(const QVector<Movie*>& movies);

    QVector<Movie*> takeAll(QObject* parent);
    /// \brief Clear and delete all stored movies.
    void clear();

private:
    QVector<Movie*> m_movies;
    QMutex m_lock;
};

/// \brief Interface for loading movies.
class MovieLoader : public QObject
{
    Q_OBJECT
public:
    explicit MovieLoader(MovieLoaderStore* store, QObject* parent = nullptr) : QObject(parent), m_store{store} {}
    ~MovieLoader() override = default;

public:
    virtual void start() = 0;
    /// \brief   Thread-safe way to abort the MovieLoader.
    /// \details Implementations must ensure that this method can be called from
    ///          any thread, i.e. this function must be thread safe.
    ///          Furthermore, the finished() signal MUST still be emitted.
    virtual void abort() = 0;
    /// \brief Thread-safe way to check whether the MovieLoader was aborted.
    virtual bool isAborted() = 0;

signals:
    void progress(mediaelch::MovieLoader* job, int processed, int total);
    /// \brief   A translated string representing the current loading state.
    /// \details For example the currently scanned directory.
    void progressText(mediaelch::MovieLoader* job, QString text);
    void finished(mediaelch::MovieLoader* job);

protected:
    MovieLoaderStore* m_store = nullptr;
};


/// \brief Creates a thread and moves the worker to it. Auto deletes thread when worker is finished.
QThread* createAutoDeleteThreadWithMovieLoader(MovieLoader* worker, QObject* threadParent);

/// \brief Load movies from disk.
class MovieDiskLoader : public MovieLoader
{
    Q_OBJECT
public:
    MovieDiskLoader(SettingsDir dir, MovieLoaderStore& store, FileFilter filter, QObject* parent = nullptr);
    ~MovieDiskLoader() override;

public:
    void start() override;
    void abort() override;
    bool isAborted() override { return m_aborted.load(); }

private:
    void loadMovieContents();
    void createMovie(QStringList files);
    /// \brief Store all loaded movies into the MovieLoaderStore and database.
    void storeAndAddToDatabase();

private:
    SettingsDir m_dir;
    FileFilter m_filter;
    Database* m_db = nullptr;
    QMutex m_mutex;
    QVector<Movie*> m_movies;
    std::atomic_bool m_aborted{false};
    std::atomic_int m_processed{0};
    int m_approxTotal{0};

    // TODO: Streamline, e.g. use one vector of directories with DiscType tags
    QHash<QString, QDateTime> m_lastModifications;
    QStringList m_bluRayDirectories;
    QStringList m_dvdDirectories;
    QMap<QString, QStringList> m_contents;
};

/// \brief Load movies from database
class MovieDatabaseLoader : public MovieLoader
{
    Q_OBJECT
public:
    MovieDatabaseLoader(SettingsDir dir, MovieLoaderStore& store, QObject* parent = nullptr) :
        MovieLoader(&store, parent), m_dir{dir}
    {
    }
    ~MovieDatabaseLoader() override = default;

    void start() override;
    void abort() override;
    bool isAborted() override { return m_aborted.load(); }

private:
    SettingsDir m_dir;
    std::atomic_bool m_aborted{false};
};


} // namespace mediaelch
