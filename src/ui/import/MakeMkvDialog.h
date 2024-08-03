#pragma once

#include "data/movie/Movie.h"
#include "import/MakeMkvCon.h"
#include "settings/ImportSettings.h"

#include <QDialog>
#include <QMap>
#include <QPointer>
#include <QString>

namespace Ui {
class MakeMkvDialog;
}

class MakeMkvDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MakeMkvDialog(ImportSettings& settings, QWidget* parent = nullptr);
    ~MakeMkvDialog() override;

public slots:
    int exec() override;
    void reject() override;
    void accept() override;

private slots:
    void onGotDrives(QMap<int, QString> drives);
    void onScanDrive();
    void onScanFinished(QString title, QMap<int, MakeMkvCon::Track> tracks);
    void onImportTracks();
    void onImportComplete();
    void onMovieChosen();
    void onLoadDone(Movie* movie);
    void onImport();
    void onDiscBackedUp();
    void onTrackImported(int trackId);
    void onImportProgress(int value, int max);

private:
    Ui::MakeMkvDialog* ui = nullptr;
    ImportSettings& m_settings;
    MakeMkvCon* m_makeMkvCon = nullptr;
    QPointer<Movie> m_movie;
    QString m_title;
    QMap<int, QString> m_tracks;
    bool m_importComplete = false;
    QString m_importDir;

    void setDefaults();
    void storeDefaults();
    void importFinished();
};
