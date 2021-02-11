#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include <QToolButton>

#include "globals/Filter.h"
#include "globals/Globals.h"
#include "renamer/RenamerDialog.h"
#include "settings/Settings.h"
#include "ui/export/ExportDialog.h"
#include "ui/main/AboutDialog.h"
#include "ui/main/FileScannerDialog.h"
#include "ui/media_centers/KodiSync.h"
#include "ui/settings/SettingsWindow.h"
#include "ui/support/SupportDialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    static MainWindow* instance();

public slots:
    /// \brief Sets or removes the new mark in the main menu on the left
    void setNewMarks();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent*) override;

private slots:
    void progressProgress(int current, int max, int id);
    void progressFinished(int id);
    void progressStarted(QString msg, int id);
    void onMenu(QToolButton* button = nullptr);
    void onActionSearch();
    void onActionSave();

    /// \brief Called when the action "Save all" was clicked
    /// Delegates the event down to the current subwidget
    void onActionSaveAll();
    /// \brief Executes the file scanner dialog
    void onActionReload();
    void onActionXbmc();
    void onActionRename();
    void onFilterChanged(QVector<Filter*> filters, QString text);
    void onSetSaveEnabled(bool enabled, MainWidgets widget);
    void onSetSearchEnabled(bool enabled, MainWidgets widget);
    /// \brief Moves all splitters
    /// \details Each widget has its own splitter. This slot resizes all of them.
    void moveSplitter(int pos, int index);
    void onTriggerReloadAll();
    void onKodiSyncFinished();
    void onFilesRenamed(Renamer::RenameType type = Renamer::RenameType::All);
    void onRenewModels();
    void onJumpToMovie(Movie* movie);
    void updateTvShows();
    void onCommandBarOpen();

private:
    MainWidgets currentTab() const;
    void setupToolbar();
    void setIcons(QToolButton* button);

private:
    Ui::MainWindow* ui = nullptr;
    Settings* m_settings = nullptr;
    SettingsWindow* m_settingsWindow = nullptr;
    AboutDialog* m_aboutDialog = nullptr;
    SupportDialog* m_supportDialog = nullptr;
    FileScannerDialog* m_fileScannerDialog = nullptr;
    KodiSync* m_xbmcSync = nullptr;
    RenamerDialog* m_renamer = nullptr;
    QAction* m_actionSearch = nullptr;
    QAction* m_actionSave = nullptr;
    QAction* m_actionXbmc = nullptr;
    QAction* m_actionAbout = nullptr;
    QAction* m_actionQuit = nullptr;
    QAction* m_actionSaveAll = nullptr;
    QAction* m_actionSettings = nullptr;
    QAction* m_actionLike = nullptr;
    QAction* m_actionReload = nullptr;
    QAction* m_actionRename = nullptr;
    QAction* m_actionExport = nullptr;
    QMap<MainWidgets, QMap<MainActions, bool>> m_actions;
    QMap<MainWidgets, QIcon> m_icons;
    static MainWindow* m_instance;
    QColor m_buttonColor;
    QColor m_buttonActiveColor;
};
