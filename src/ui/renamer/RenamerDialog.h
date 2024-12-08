#pragma once

#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
#include "data/tv_show/TvShow.h"
#include "data/tv_show/TvShowEpisode.h"
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
    explicit RenamerDialog(QWidget* parent = nullptr);
    ~RenamerDialog() override;
    void setMovies(QVector<Movie*> movies);
    void setConcerts(QVector<Concert*> concerts);
    void setShows(QVector<TvShow*> shows);
    void setEpisodes(QVector<TvShowEpisode*> episodes);

    bool renameErrorOccurred() const;

    int addResultToTable(const QString& oldFileName, const QString& newFileName, Renamer::RenameOperation operation);
    void setResultStatus(int row, Renamer::RenameResult result);
    void appendResultText(QString str);

public slots:
    int exec() override;
    void reject() override;

signals:
    void sigFilesRenamed(RenameType type, bool hasErrors);

private slots:
    void onRename();
    void onDryRun();
    void onChkRenameDirectories();
    void onChkRenameFiles();
    void onChkUseSeasonDirectories();
    void onChkReplaceDelimiter();
    void onRenamed();

private:
    void renameType(const bool isDryRun);
    void renameMovies(QVector<Movie*> movies, const RenamerConfig& config);
    void renameConcerts(QVector<Concert*> concerts, const RenamerConfig& config);
    void renameEpisodes(QVector<TvShowEpisode*> episodes, const RenamerConfig& config);
    void renameTvShows(const QVector<TvShow*>& shows, const QString& directoryPattern, const bool& dryRun = false);

protected:
    Ui::RenamerDialog* ui = nullptr;

    QVector<Movie*> m_movies;
    QVector<Concert*> m_concerts;
    QVector<TvShow*> m_shows;
    QVector<TvShowEpisode*> m_episodes;
    RenameType m_renameType = RenameType::All;
    bool m_filesRenamed = 0;
    mediaelch::FileFilter m_extraFiles;
    bool m_renameErrorOccured = 0;
};
