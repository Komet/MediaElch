#ifndef RENAMER_H
#define RENAMER_H

#include <QDialog>
#include <QDir>
#include <QFile>

#include "data/Concert.h"
#include "data/Movie.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"

namespace Ui {
class Renamer;
}

struct RenamerConfig
{
    QString filePattern;
    QString filePatternMulti;
    QString directoryPattern;
    bool renameFiles = false;
    bool renameDirectories = false;
    bool dryRun = false;
};

class Renamer : public QDialog
{
    Q_OBJECT

public:
    enum class RenameType
    {
        Movies,
        TvShows,
        Concerts,
        All
    };
    enum class RenameResult
    {
        Failed,
        Success
    };
    enum class RenameOperation
    {
        CreateDir,
        Move,
        Rename
    };

    explicit Renamer(QWidget *parent = nullptr);
    ~Renamer() override;
    void setMovies(QList<Movie *> movies);
    void setConcerts(QList<Concert *> concerts);
    void setShows(QList<TvShow *> shows);
    void setEpisodes(QList<TvShowEpisode *> episodes);
    void setRenameType(RenameType type);

    static QString replace(QString &text, const QString &search, const QString &replace);
    static QString replaceCondition(QString &text, const QString &condition, const QString &replace);
    static QString replaceCondition(QString &text, const QString &condition, bool hasCondition);

    static QString typeToString(RenameType type);

    bool renameErrorOccured() const;

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
    Ui::Renamer *ui;

    QList<Movie *> m_movies;
    QList<Concert *> m_concerts;
    QList<TvShow *> m_shows;
    QList<TvShowEpisode *> m_episodes;
    RenameType m_renameType;
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

    bool rename(const QString &file, const QString &newName);
    bool rename(QDir &dir, QString newName);
    int addResult(const QString &oldFileName, const QString &newFileName, RenameOperation operation);
    void setResultStatus(int row, RenameResult result);
};

#endif // RENAMER_H
