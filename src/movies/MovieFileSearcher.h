#pragma once

#include "movies/Movie.h"

#include <QDir>
#include <QHash>
#include <QObject>
#include <QTime>

/**
 * @brief The MovieFileSearcher class
 */
class MovieFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit MovieFileSearcher(QObject* parent = nullptr);
    ~MovieFileSearcher() override = default;

    void setMovieDirectories(QVector<SettingsDir> directories);
    void scanDir(QString startPath,
        QString path,
        QVector<QStringList>& contents,
        bool separateFolders = false,
        bool firstScan = false);
    static Movie* loadMovieData(Movie* movie);

public slots:
    void reload(bool force);
    void abort();

signals:
    void searchStarted(QString, int);
    void progress(int, int, int);
    void moviesLoaded(int);
    void currentDir(QString);

private:
    QStringList getFiles(QString path);

    QVector<SettingsDir> m_directories;
    int m_progressMessageId;
    QHash<QString, QDateTime> m_lastModifications;
    bool m_aborted;

    struct MovieContents
    {
        QString path;
        bool inSeparateFolder;
        QMap<QString, QStringList> contents;
    };
};
