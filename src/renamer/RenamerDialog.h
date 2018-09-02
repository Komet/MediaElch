#ifndef RENAMER_DIALOG_H
#define RENAMER_DIALOG_H

#include <QDialog>
#include <QDir>
#include <QFile>

#include "data/Concert.h"
#include "data/Movie.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "renamer/Renamer.h"

namespace Ui {
class RenamerDialog;
}

class RenamerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenamerDialog(QWidget *parent = nullptr);
    ~RenamerDialog() override;
    void setMovies(QList<Movie *> movies);
    void setConcerts(QList<Concert *> concerts);
    void setShows(QList<TvShow *> shows);
    void setEpisodes(QList<TvShowEpisode *> episodes);
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

    QList<Movie *> m_movies;
    QList<Concert *> m_concerts;
    QList<TvShow *> m_shows;
    QList<TvShowEpisode *> m_episodes;
    Renamer::RenameType m_renameType;
    bool m_filesRenamed;
    QStringList m_extraFiles;
    bool m_renameErrorOccured;

    void renameMovies(QList<Movie *> movies, const RenamerConfig &config);
    void renameConcerts(QList<Concert *> concerts, const RenamerConfig &config);
    void renameEpisodes(QList<TvShowEpisode *> episodes, const RenamerConfig &config);
    void renameShows(QList<TvShow *> shows,
        const QString &directoryPattern,
        const bool &renameDirectories,
        const bool &dryRun = false);
};

#endif // RENAMER_DIALOG_H
