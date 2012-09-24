#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFileDialog>
#include <QSettings>
#include <QTableWidgetItem>
#include <QWidget>

#include "globals/Globals.h"
#include "settings/DataFile.h"

namespace Ui {
class Settings;
}

/**
 * @brief The Settings class stores all MediaElch settings and displays the settings widget.
 */
class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();

    static Settings *instance();
    void loadSettings();
    QSize mainWindowSize();
    QPoint mainWindowPosition();
    QByteArray movieSplitterState();
    QByteArray tvShowSplitterState();
    QByteArray movieSetsSplitterState();
    QByteArray concertSplitterState();
    QList<SettingsDir> movieDirectories();
    QList<SettingsDir> tvShowDirectories();
    QList<SettingsDir> concertDirectories();
    int mediaCenterInterface();
    QString xbmcMysqlHost();
    QString xbmcMysqlDatabase();
    QString xbmcMysqlUser();
    QString xbmcMysqlPassword();
    QString xbmcSqliteDatabase();
    QString xbmcThumbnailPath();
    bool debugModeActivated();
    QString debugLogPath();
    bool useYoutubePluginUrls();
    QList<DataFile*> enabledMovieNfoFiles();
    QList<DataFile*> enabledMoviePosterFiles();
    QList<DataFile*> enabledMovieFanartFiles();
    QList<DataFile*> enabledTvShowPosterFiles();
    QList<DataFile*> enabledTvShowBannerFiles();
    QList<DataFile*> enabledConcertNfoFiles();
    QList<DataFile*> enabledConcertPosterFiles();
    QList<DataFile*> enabledConcertFanartFiles();

    void setMainWindowSize(QSize mainWindowSize);
    void setMainWindowPosition(QPoint mainWindowPosition);
    void setMovieSplitterState(QByteArray state);
    void setTvShowSplitterState(QByteArray state);
    void setMovieSetsSplitterState(QByteArray state);
    void setConcertSplitterState(QByteArray state);

public slots:
    void saveSettings();

private slots:
    void addMovieDir(QString dir);
    void removeMovieDir();
    void movieListRowChanged(int currentRow);
    void addTvShowDir(QString dir);
    void removeTvShowDir();
    void tvShowListRowChanged(int currentRow);
    void addConcertDir(QString dir);
    void removeConcertDir();
    void concertListRowChanged(int currentRow);
    void movieMediaCenterPathChanged(QTableWidgetItem *item);
    void tvShowMediaCenterPathChanged(QTableWidgetItem *item);
    void concertMediaCenterPathChanged(QTableWidgetItem *item);
    void onMediaCenterXbmcXmlSelected();
    void onMediaCenterXbmcMysqlSelected();
    void onMediaCenterXbmcSqliteSelected();
    void onChooseMediaCenterXbmcSqliteDatabase(QString file);
    void onChooseXbmcThumbnailPath(QString dir);
    void onDebugLogPathChosen(QString file);
    void onActivateDebugMode();
    void onSetDebugLogPath(QString path);

private:
    Ui::Settings *ui;
    QSettings m_settings;
    QList<SettingsDir> m_movieDirectories;
    QList<SettingsDir> m_tvShowDirectories;
    QList<SettingsDir> m_concertDirectories;
    QSize m_mainWindowSize;
    QPoint m_mainWindowPosition;
    QByteArray m_movieSplitterState;
    QByteArray m_tvShowSplitterState;
    QByteArray m_movieSetsSplitterState;
    QByteArray m_concertSplitterState;
    int m_mediaCenterInterface;
    QString m_xbmcMysqlHost;
    QString m_xbmcMysqlDatabase;
    QString m_xbmcMysqlUser;
    QString m_xbmcMysqlPassword;
    QString m_xbmcSqliteDatabase;
    QString m_xbmcThumbnailPath;
    bool m_debugModeActivated;
    QString m_debugLogPath;
    static Settings *m_instance;
    QFileDialog *m_logFileDialog;
    QFileDialog *m_movieDirDialog;
    QFileDialog *m_tvShowDirDialog;
    QFileDialog *m_concertDirDialog;
    QFileDialog *m_xbmcThumbnailDirDialog;
    QFileDialog *m_xbmcSqliteDatabaseDialog;
    bool m_youtubePluginUrls;
    QList<DataFile*> m_movieNfoFiles;
    QList<DataFile*> m_moviePosterFiles;
    QList<DataFile*> m_movieFanartFiles;
    QList<DataFile*> m_concertNfoFiles;
    QList<DataFile*> m_concertPosterFiles;
    QList<DataFile*> m_concertFanartFiles;
    QList<DataFile*> m_tvShowPosterFiles;
    QList<DataFile*> m_tvShowBannerFiles;

    void setXbmcThumbnailPathEnabled(bool enabled);
};

#endif // SETTINGS_H
