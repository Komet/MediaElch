#ifndef RENAMER_H
#define RENAMER_H

#include <QDialog>
#include <QDir>
#include <QFile>
#include "data/Concert.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "movies/Movie.h"

namespace Ui {
class Renamer;
}

class Renamer : public QDialog
{
    Q_OBJECT

public:
    enum RenameType {
        TypeMovies, TypeTvShows, TypeConcerts, TypeAll
    };

    explicit Renamer(QWidget *parent = 0);
    ~Renamer();
    void setMovies(QList<Movie*> movies);
    void setConcerts(QList<Concert*> concerts);
    void setShows(QList<TvShow*> shows);
    void setEpisodes(QList<TvShowEpisode*> episodes);
    void setRenameType(RenameType type);

public slots:
    int exec();
    void reject();

signals:
    void sigFilesRenamed(Renamer::RenameType);

private slots:
    void onRename();
    void onDryRun();
    void onChkRenameDirectories();
    void onChkRenameFiles();
    void onChkUseSeasonDirectories();

private:
    Ui::Renamer *ui;

    QList<Movie*> m_movies;
    QList<Concert*> m_concerts;
    QList<TvShow*> m_shows;
    QList<TvShowEpisode*> m_episodes;
    RenameType m_renameType;
    bool m_filesRenamed;
    QStringList m_extraFiles;

    void renameMovies(QList<Movie*> movies, const QString &filePattern, const QString &filePatternMulti,
                      const QString &directoryPattern, const bool &renameFiles, const bool &renameDirectories, const bool &dryRun = false);
    void renameConcerts(QList<Concert*> concerts, const QString &filePattern, const QString &filePatternMulti,
                        const QString &directoryPattern, const bool &renameFiles, const bool &renameDirectories, const bool &dryRun = false);
    void renameEpisodes(QList<TvShowEpisode*> episodes, const QString &filePattern, const QString &filePatternMulti, const QString &seasonPattern, const bool &renameFiles,
                        const bool &useSeasonDirectories, const bool &dryRun = false);
    void renameShows(QList<TvShow*> shows, const QString &directoryPattern, const bool &renameDirectories, const bool &dryRun = false);

    bool rename(const QString &file, const QString &newName);
    bool rename(QDir &dir, QString newName);

};

#endif // RENAMER_H
