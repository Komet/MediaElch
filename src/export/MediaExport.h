#pragma once

#include "export/ExportTemplate.h"

#include <QDir>
#include <QObject>

class Concert;
class Movie;
class TvShow;
class TvShowEpisode;
class StreamDetails;

namespace mediaelch {

/// Default export class for MediaElch.
class MediaExport : public QObject
{
    Q_OBJECT
public:
    explicit MediaExport(QObject* parent = nullptr);

signals:
    /// Signal is emitted each time an item is exported (e.g. image, generated HTML, etc.)
    /// Useful for progress bars.
    void sigItemExported(int itemsExported);

public:
    /// Export the user's media library to the given directory. Only export the given sections.
    void
    doExport(ExportTemplate& exportTemplate, QDir directory, const QVector<ExportTemplate::ExportSection>& sections);
    /// Cancels the export of the user's library. Sets an internal flag which is checked
    /// by all long running export functions. No cleanup is done.
    void cancel();
    /// Resets internal state (e.g. cancel flags)
    void reset();

private:
    void parseAndSaveMovies(QVector<Movie*> movies);
    void parseAndSaveConcerts(QVector<Concert*> concerts);
    void parseAndSaveTvShows(QVector<TvShow*> shows);
    void saveImage(QSize size, QString imageFile, QString destinationFile, const char* format, int quality);
    void replaceImages(QString& m,
        const bool& subDir,
        const Movie* movie = nullptr,
        const Concert* concert = nullptr,
        const TvShow* tvShow = nullptr,
        const TvShowEpisode* episode = nullptr);
    bool saveImageForType(const QString& type, const QSize& size, QString& destFile, const Movie* movie);
    bool saveImageForType(const QString& type, const QSize& size, QString& destFile, const Concert* concert);
    bool saveImageForType(const QString& type, const QSize& size, QString& destFile, const TvShow* tvShow);
    bool saveImageForType(const QString& type, const QSize& size, QString& destFile, const TvShowEpisode* episode);
    void replaceVars(QString& m, Movie* movie, bool subDir = false);
    void replaceVars(QString& m, const Concert* concert, bool subDir = false);
    void replaceVars(QString& m, const TvShow* show, bool subDir = false);
    void replaceVars(QString& m, TvShowEpisode* episode, bool subDir = false);
    void replaceSingleBlock(QString& m, QString blockName, QString itemName, QStringList replaces);
    void replaceMultiBlock(QString& m, QString blockName, QStringList itemNames, QVector<QStringList> replaces);
    void replaceStreamDetailsVars(QString& m, const StreamDetails* streamDetails);

private:
    volatile bool m_canceled = false;
    int m_itemsExported = 0;
    ExportTemplate* m_template = nullptr;
    QDir m_dir;
};

} // namespace mediaelch
