#pragma once

#include "export/ExportTemplate.h"

#include <QDir>
#include <QObject>
#include <atomic>

class Concert;
class Movie;
class TvShow;
class TvShowEpisode;
class StreamDetails;

namespace mediaelch {

/// Default export engine for MediaElch. Simple find&replace semantics,
/// only basic functionality (e.g. condintional block)
class SimpleEngine : public QObject
{
    Q_OBJECT
public:
    explicit SimpleEngine(ExportTemplate& exportTemplate,
        QDir directory,
        std::atomic_bool& cancelFlag,
        QObject* parent = nullptr);

signals:
    /// Signal is emitted each time an item is exported (e.g. image, generated HTML, etc.)
    /// Useful for progress bars.
    void sigItemExported();

public:
    void exportMovies(QVector<Movie*> movies);
    void exportConcerts(QVector<Concert*> concerts);
    void exportTvShows(QVector<TvShow*> shows);

private:
    void saveImage(QSize size, QString imageFile, QString destinationFile, const char* format, int quality);
    // TODO: Cleanup
    void replaceImages(QString& m,
        const bool& subDir,
        const Movie* movie,
        const Concert* concert,
        const QString& concertId,
        const TvShow* tvShow,
        const TvShowEpisode* episode);

    void replaceImages(QString& m, const bool& subDir, const Movie* movie);
    void replaceImages(QString& m, const bool& subDir, const Concert* concert, const QString& concertId);
    void replaceImages(QString& m, const bool& subDir, const TvShow* tvShow);
    void replaceImages(QString& m, const bool& subDir, const TvShowEpisode* episode);

    bool saveImageForType(const QString& type,
        const QSize& size,
        QString& destFile,
        const Movie* movie,
        bool* isPlaceHolderUsed);
    bool saveImageForType(const QString& type,
        const QSize& size,
        QString& destFile,
        const Concert* concert,
        const QString& concertId,
        bool* isPlaceHolderUsed);
    bool saveImageForType(const QString& type,
        const QSize& size,
        QString& destFile,
        const TvShow* tvShow,
        bool* isPlaceHolderUsed);
    bool saveImageForType(const QString& type,
        const QSize& size,
        QString& destFile,
        const TvShowEpisode* episode,
        bool* isPlaceHolderUsed);
    void replaceVars(QString& m, Movie* movie, bool subDir = false);
    void replaceVars(QString& m, const Concert* concert, const QString& id, bool subDir = false);
    void replaceVars(QString& m, const TvShow* show, bool subDir = false);
    void replaceVars(QString& m, const TvShowEpisode* episode, bool subDir = false);
    void replaceSingleBlock(QString& m, QString blockName, QString itemName, QStringList replaces);
    void replaceMultiBlock(QString& m, QString blockName, QStringList itemNames, QVector<QStringList> replaces);
    void replaceStreamDetailsVars(QString& m, const StreamDetails* details);

private:
    std::atomic_bool& m_cancelFlag;
    ExportTemplate* m_template = nullptr;
    QDir m_dir;
};

} // namespace mediaelch
