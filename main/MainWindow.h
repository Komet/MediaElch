#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include "globals/Filter.h"
#include "globals/Globals.h"
#include "data/MovieFileSearcher.h"
#include "export/ExportDialog.h"
#include "main/AboutDialog.h"
#include "main/FileScannerDialog.h"
#include "renamer/Renamer.h"
#include "smallWidgets/FilterWidget.h"
#include "settings/Settings.h"
#include "settings/SettingsWindow.h"
#include "support/SupportDialog.h"
#include "xbmc/XbmcSync.h"

namespace Ui {
class MainWindow;
}

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static MainWindow* instance();

public slots:
    void setNewMarks();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void progressProgress(int current, int max, int id);
    void progressFinished(int id);
    void progressStarted(QString msg, int id);
    void onMenu(MainWidgets widget);
    void onMenuMovies();
    void onMenuMovieSets();
    void onMenuTvShows();
    void onMenuConcerts();
    void onMenuGenres();
    void onMenuCertifications();
    void onMenuDownloads();
    void onActionSearch();
    void onActionSave();
    void onActionSaveAll();
    void onActionReload();
    void onActionXbmc();
    void onActionRename();
    void onFilterChanged(QList<Filter*> filters, QString text);
    void onSetSaveEnabled(bool enabled, MainWidgets widget);
    void onSetSearchEnabled(bool enabled, MainWidgets widget);
    void moveSplitter(int pos, int index);
    void onTriggerReloadAll();
    void onXbmcSyncFinished();
    void onFilesRenamed(Renamer::RenameType type = Renamer::TypeAll);
    void onRenewModels();
    void onJumpToMovie(Movie *movie);

private:
    Ui::MainWindow *ui;
    Settings *m_settings;
    SettingsWindow *m_settingsWindow;
    AboutDialog *m_aboutDialog;
    SupportDialog *m_supportDialog;
    FileScannerDialog *m_fileScannerDialog;
    ExportDialog *m_exportDialog;
    XbmcSync *m_xbmcSync;
    Renamer *m_renamer;
    QAction *m_actionSearch;
    QAction *m_actionSave;
    QAction *m_actionXbmc;
    QAction *m_actionAbout;
    QAction *m_actionQuit;
    QAction *m_actionSaveAll;
    QAction *m_actionSettings;
    QAction *m_actionLike;
    QAction *m_actionReload;
    QAction *m_actionRename;
    QAction *m_actionExport;
    QMap<MainWidgets, QMap<MainActions, bool> > m_actions;
    QMap<MainWidgets, QIcon> m_icons;
    FilterWidget *m_filterWidget;
    static MainWindow *m_instance;
    void setupToolbar();
};

#endif // MAINWINDOW_H
