#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include <QToolButton>

#include "data/MovieFileSearcher.h"
#include "export/ExportDialog.h"
#include "globals/Filter.h"
#include "globals/Globals.h"
#include "main/AboutDialog.h"
#include "main/FileScannerDialog.h"
#include "plugins/PluginInterface.h"
#include "renamer/Renamer.h"
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
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    static MainWindow *instance();

public slots:
    void setNewMarks();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void progressProgress(int current, int max, int id);
    void progressFinished(int id);
    void progressStarted(QString msg, int id);
    void onMenu(QToolButton *button = nullptr);
    void onActionSearch();
    void onActionSave();
    void onActionSaveAll();
    void onActionReload();
    void onActionXbmc();
    void onActionRename();
    void onFilterChanged(QList<Filter *> filters, QString text);
    void onSetSaveEnabled(bool enabled, MainWidgets widget);
    void onSetSearchEnabled(bool enabled, MainWidgets widget);
    void moveSplitter(int pos, int index);
    void onTriggerReloadAll();
    void onXbmcSyncFinished();
    void onFilesRenamed(Renamer::RenameType type = Renamer::TypeAll);
    void onRenewModels();
    void onJumpToMovie(Movie *movie);
    void updateTvShows();
    void onAddPlugin(PluginInterface *plugin);
    void onRemovePlugin(PluginInterface *plugin);

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
    QMap<MainWidgets, QMap<MainActions, bool>> m_actions;
    QMap<MainWidgets, QIcon> m_icons;
    static MainWindow *m_instance;
    QMap<int, PluginInterface *> m_plugins;
    QColor m_buttonColor;
    QColor m_buttonActiveColor;
    void setupToolbar();
    void setIcons(QToolButton *button);
};

#endif // MAINWINDOW_H
