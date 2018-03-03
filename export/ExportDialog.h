#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QDir>

#include "data/Concert.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "export/ExportTemplate.h"
#include "movies/Movie.h"

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog() override;

public slots:
    int exec() override;

private slots:
    void onBtnExport();
    void onThemeChanged();
    void onBtnClose();

private:
    Ui::ExportDialog *ui;
    bool m_canceled;

    void parseAndSaveMovies(QDir dir, ExportTemplate *exportTemplate, QList<Movie *> movies);
    void parseAndSaveConcerts(QDir dir, ExportTemplate *exportTemplate, QList<Concert *> concerts);
    void parseAndSaveTvShows(QDir dir, ExportTemplate *exportTemplate, QList<TvShow *> shows);
    void saveImage(QSize size, QString imageFile, QString destinationFile, const char *format, int quality);
    void replaceImages(QString &m,
        const QDir &dir,
        const bool &subDir,
        Movie *movie = nullptr,
        Concert *concert = nullptr,
        TvShow *tvShow = nullptr,
        TvShowEpisode *episode = nullptr);
    bool saveImageForType(const QString &type, const QSize &size, const QDir &dir, QString &destFile, Movie *movie);
    bool saveImageForType(const QString &type, const QSize &size, const QDir &dir, QString &destFile, Concert *concert);
    bool saveImageForType(const QString &type, const QSize &size, const QDir &dir, QString &destFile, TvShow *tvShow);
    bool saveImageForType(const QString &type,
        const QSize &size,
        const QDir &dir,
        QString &destFile,
        TvShowEpisode *episode);
    void replaceVars(QString &m, Movie *movie, QDir dir, bool subDir = false);
    void replaceVars(QString &m, Concert *concert, QDir dir, bool subDir = false);
    void replaceVars(QString &m, TvShow *show, QDir dir, bool subDir = false);
    void replaceVars(QString &m, TvShowEpisode *episode, QDir dir, bool subDir = false);
    void replaceSingleBlock(QString &m, QString blockName, QString itemName, QStringList replaces);
    void replaceMultiBlock(QString &m, QString blockName, QStringList itemNames, QList<QStringList> replaces);
    void replaceStreamDetailsVars(QString &m, StreamDetails *streamDetails);
};

#endif // EXPORTDIALOG_H
