#ifndef FILESCANNERDIALOG_H
#define FILESCANNERDIALOG_H

#include <QDialog>

namespace Ui {
class FileScannerDialog;
}

/**
 * @brief The FileScannerDialog class
 */
class FileScannerDialog : public QDialog
{
    Q_OBJECT

public:
    enum ReloadType {
        TypeAll, TypeMovies, TypeTvShows, TypeConcerts, TypeEpisodes, TypeMusic
    };

    explicit FileScannerDialog(QWidget *parent = 0);
    ~FileScannerDialog();
    void setForceReload(bool force);
    void setReloadType(ReloadType type);
    void setScanDir(QString dir);

public slots:
    int exec();
    void reject();

private slots:
    void onProgress(int current, int max);
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
    void onLoadDone(int msgId);

private:
    Ui::FileScannerDialog *ui;

    bool m_forceReload;
    ReloadType m_reloadType;
    QString m_scanDir;
};

#endif // FILESCANNERDIALOG_H
