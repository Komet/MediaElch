#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QFileDialog>
#include <QSettings>
#include <QTableWidgetItem>
#include <QWidget>

#include "globals/Globals.h"
#include "settings/DataFile.h"
#include "settings/Settings.h"

namespace Ui {
class SettingsWidget;
}

/**
 * @brief The Settings class stores all MediaElch settings and displays the settings widget.
 */
class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = 0);
    ~SettingsWidget();

    void loadSettings();


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
    Ui::SettingsWidget *ui;
    Settings *m_settings;

    QFileDialog *m_logFileDialog;
    QFileDialog *m_movieDirDialog;
    QFileDialog *m_tvShowDirDialog;
    QFileDialog *m_concertDirDialog;
    QFileDialog *m_xbmcThumbnailDirDialog;
    QFileDialog *m_xbmcSqliteDatabaseDialog;

    void setXbmcThumbnailPathEnabled(bool enabled);
};

#endif // SETTINGSWIDGET_H
