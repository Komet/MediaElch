#pragma once

#include <QDialog>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QVector>

#include "export/ExportTemplate.h"

namespace Ui {
class ExportDialog;
}

class Concert;
class Movie;
class TvShow;
class TvShowEpisode;
class StreamDetails;

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget* parent = nullptr);
    ~ExportDialog() override;

public slots:
    int exec() override;

private slots:
    void onBtnExport();
    void onThemeChanged();
    void onBtnClose();

private:
    Ui::ExportDialog* ui = nullptr;
    bool m_canceled = false;

    void parseAndSaveMovies(QDir dir, ExportTemplate* exportTemplate, QVector<Movie*> movies);
    void parseAndSaveConcerts(QDir dir, ExportTemplate* exportTemplate, QVector<Concert*> concerts);
    void parseAndSaveTvShows(QDir dir, ExportTemplate* exportTemplate, QVector<TvShow*> shows);
    void saveImage(QSize size, QString imageFile, QString destinationFile, const char* format, int quality);
    void replaceImages(QString& m,
        const QDir& dir,
        const bool& subDir,
        const Movie* movie = nullptr,
        const Concert* concert = nullptr,
        const TvShow* tvShow = nullptr,
        const TvShowEpisode* episode = nullptr);
    bool
    saveImageForType(const QString& type, const QSize& size, const QDir& dir, QString& destFile, const Movie* movie);
    bool saveImageForType(const QString& type,
        const QSize& size,
        const QDir& dir,
        QString& destFile,
        const Concert* concert);
    bool
    saveImageForType(const QString& type, const QSize& size, const QDir& dir, QString& destFile, const TvShow* tvShow);
    bool saveImageForType(const QString& type,
        const QSize& size,
        const QDir& dir,
        QString& destFile,
        const TvShowEpisode* episode);
    void replaceVars(QString& m, Movie* movie, QDir dir, bool subDir = false);
    void replaceVars(QString& m, const Concert* concert, QDir dir, bool subDir = false);
    void replaceVars(QString& m, const TvShow* show, QDir dir, bool subDir = false);
    void replaceVars(QString& m, TvShowEpisode* episode, QDir dir, bool subDir = false);
    void replaceSingleBlock(QString& m, QString blockName, QString itemName, QStringList replaces);
    void replaceMultiBlock(QString& m, QString blockName, QStringList itemNames, QVector<QStringList> replaces);
    void replaceStreamDetailsVars(QString& m, const StreamDetails* streamDetails);
};
