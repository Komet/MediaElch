#pragma once

#include "data/Concert.h"
#include "data/Movie.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "renamer/Renamer.h"

#include <QDialog>
#include <QDir>
#include <QFile>

namespace Ui {
class RenamerDialog;
}

class RenamerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenamerDialog(QWidget *parent = nullptr);
    ~RenamerDialog() override;
    void setMovies(QVector<Movie *> movies);
    void setConcerts(QVector<Concert *> concerts);
    void setShows(QVector<TvShow *> shows);
    void setEpisodes(QVector<TvShowEpisode *> episodes);
    void setRenameType(Renamer::RenameType type);

    bool renameErrorOccured() const;

    int addResultToTable(const QString &oldFileName, const QString &newFileName, Renamer::RenameOperation operation);
    void setResultStatus(int row, Renamer::RenameResult result);
    void appendResultText(QString str);

public slots:
    int exec() override;
    void reject() override;

signals:
    void sigFilesRenamed(Renamer::RenameType);

private slots:
    void onRename();
    void onDryRun();
    void onChkRenameDirectories();
    void onChkRenameFiles();
    void onChkUseSeasonDirectories();
    void onRenamed();

private:
    Ui::RenamerDialog *ui;

    QVector<Movie *> m_movies;
    QVector<Concert *> m_concerts;
    QVector<TvShow *> m_shows;
    QVector<TvShowEpisode *> m_episodes;
    Renamer::RenameType m_renameType;
    bool m_filesRenamed;
    QStringList m_extraFiles;
    bool m_renameErrorOccured;

    void renameType(const bool isDryRun);
    void renameMovies(QVector<Movie *> movies, const RenamerConfig &config);
    void renameConcerts(QVector<Concert *> concerts, const RenamerConfig &config);
    void renameEpisodes(QVector<TvShowEpisode *> episodes, const RenamerConfig &config);
    void renameShows(QVector<TvShow *> shows,
        const QString &directoryPattern,
        const bool &renameDirectories,
        const bool &dryRun = false);
};
