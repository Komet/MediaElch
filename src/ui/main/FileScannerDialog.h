#pragma once

#include "file/Path.h"

#include <QDialog>

namespace Ui {
class FileScannerDialog;
}

class FileScannerDialog : public QDialog
{
    Q_OBJECT

public:
    enum class ReloadType
    {
        All,
        Movies,
        TvShows,
        Concerts,
        Episodes,
        Music
    };

    explicit FileScannerDialog(QWidget* parent = nullptr);
    ~FileScannerDialog() override;
    void setForceReload(bool force);
    void setReloadType(ReloadType type);
    void setScanDir(const mediaelch::DirectoryPath& dir);

public slots:
    int exec() override;
    void reject() override;

private slots:
    void onProgress(int current, int max);
    /// Alternative to onProgress which takes a percent value.
    void onProgressPercent(float percent);
    void onCurrentDir(QString dir);
    void onStartMovieScanner();
    void onStartMovieScannerForce();
    void onStartMovieScannerCache();
    void onStartTvShowScanner();
    void onStartTvShowScannerForce();
    void onStartTvShowScannerCache();
    void onStartConcertScanner();
    void onStartConcertScannerForce();
    void onStartConcertScannerCache();
    void onStartEpisodeScanner();
    void onStartMusicScanner();
    void onStartMusicScannerForce();
    void onStartMusicScannerCache();

private:
    Ui::FileScannerDialog* ui;

    bool m_forceReload = false;
    ReloadType m_reloadType = ReloadType::All;
    mediaelch::DirectoryPath m_scanDir;
};
