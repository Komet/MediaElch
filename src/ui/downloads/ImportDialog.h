#pragma once

#include "concerts/Concert.h"
#include "downloads/FileWorker.h"
#include "globals/DownloadManager.h"
#include "globals/DownloadManagerElement.h"
#include "movies/Movie.h"
#include "renamer/RenamerDialog.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

#include <QCloseEvent>
#include <QDialog>
#include <QPointer>
#include <QThread>
#include <QTimer>

namespace Ui {
class ImportDialog;
}

class ImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportDialog(QWidget* parent = nullptr);
    ~ImportDialog() override;
    void setFiles(QStringList files);
    QStringList files();
    void setExtraFiles(QStringList extraFiles);
    QStringList extraFiles();
    void setImportDir(QString dir);
    QString importDir();

public slots:
    int exec() override;
    int execMovie(QString searchString);
    int execTvShow(QString searchString, TvShow* tvShow);
    int execConcert(QString searchString);
    void reject() override;
    void accept() override;

private slots:
    void onMovieChosen();
    void onLoadDone(Movie* movie);
    void onConcertChosen();
    void onLoadDone(Concert* concert);
    void onTvShowChosen();
    void onEpisodeLoadDone();
    void onImport();
    void onFileWatcherTimeout();
    void onMovingFilesFinished();
    void onEpisodeDownloadFinished(DownloadManagerElement elem);

private:
    Ui::ImportDialog* ui = nullptr;
    QString m_type;
    QPointer<Movie> m_movie;
    QPointer<Concert> m_concert;
    QPointer<TvShow> m_show;
    QPointer<TvShowEpisode> m_episode;
    QStringList m_files;
    QStringList m_extraFiles;
    QString m_importDir;
    bool m_separateFolders = false;
    QTimer m_timer;
    QMap<QString, QString> m_filesToMove;
    QPointer<QThread> m_workerThread;
    QPointer<FileWorker> m_worker;
    QStringList m_newFiles;
    DownloadManager* m_posterDownloadManager = nullptr;

    void setDefaults(Renamer::RenameType renameType);
    void storeDefaults();
};
