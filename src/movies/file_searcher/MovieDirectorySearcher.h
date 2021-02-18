#pragma once

#include "globals/Globals.h"

#include <QFutureWatcher>
#include <QString>
#include <QVector>
#include <atomic>

class Movie;

namespace mediaelch {

/// \brief   Searches for movies in a given directory.
/// \details Create an instance of this class and connect to \see movieProcessed(Movie*).
///          The caller must take ownership of the created movies.
class MovieDirectorySearcher : public QObject
{
    Q_OBJECT
public:
    MovieDirectorySearcher(const SettingsDir& dir, bool inSeparateFolders, QObject* parent = nullptr);
    ~MovieDirectorySearcher() override = default;

signals:
    void startLoading(int approximateMovieCount);
    void loaded(MovieDirectorySearcher* self);
    void movieProcessed(Movie* movie);

public:
    void load();
    void abort();

    const QVector<Movie*> movies() const { return m_movies; }
    SettingsDir directory() const { return m_dir; }
    int currentMovieCount() const { return m_movies.size(); }

private:
    void loadMovieContents();
    void createMovies();
    QVector<Movie*> createMovie(QStringList files);

    void postProcessMovie(Movie* movie);

    /// Get a list of files in a directory
    QStringList getFiles(QString path);

private:
    const SettingsDir& m_dir;
    QHash<QString, QDateTime> m_lastModifications;

    QFutureWatcher<QVector<Movie*>> m_watcher;

    QStringList m_bluRayDirectories;
    QStringList m_dvdDirectories;

    QMap<QString, QStringList> m_contents;
    QVector<Movie*> m_movies;

    bool m_inSeparateFolders{false};
    std::atomic_bool m_aborted{false};
};


} // namespace mediaelch
