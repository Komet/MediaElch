#pragma once

#include "data/Filter.h"
#include "globals/Globals.h"
#include "renamer/Renamer.h"

#include <QLabel>
#include <QMainWindow>
#include <QToolButton>

namespace Ui {
class MainWindow;
}

class Movie;
class FileScannerDialog;
class KodiSync;
class ConcertRenamerDialog;
class MovieRenamerDialog;
class TvShowRenamerDialog;
class SupportDialog;
class Settings;
class SettingsWindow;

class MainWindowConfiguration
{
public:
    explicit MainWindowConfiguration(Settings& settings);

    void init();

    ELCH_NODISCARD QSize mainWindowSize();
    void setMainWindowSize(QSize mainWindowSize);

    ELCH_NODISCARD QPoint mainWindowPosition();
    void setMainWindowPosition(QPoint mainWindowPosition);

    ELCH_NODISCARD bool mainWindowMaximized() const;
    void setMainWindowMaximized(bool max);

    ELCH_NODISCARD QByteArray mainSplitterState();
    void setMainSplitterState(QByteArray state);

private:
    Settings& m_settings;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(MainWindowConfiguration& settings, QWidget* parent = nullptr);
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

    void showAboutDialog();

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
    void onFilesRenamed(RenameType type, bool hasError);
    void onRenewModels();
    void onJumpToMovie(Movie* movie);
    void updateTvShows();
    void onCommandBarOpen();

private:
    MainWidgets currentTab() const;
    void setupToolbar();

private:
    Ui::MainWindow* ui{nullptr};
    MainWindowConfiguration& m_settings;
    SettingsWindow* m_settingsWindow{nullptr};
    SupportDialog* m_supportDialog{nullptr};
    FileScannerDialog* m_fileScannerDialog{nullptr};
    KodiSync* m_xbmcSync = nullptr;
    MovieRenamerDialog* m_movieRenamer{nullptr};
    ConcertRenamerDialog* m_concertRenamer{nullptr};
    TvShowRenamerDialog* m_tvShowRenamer{nullptr};
    QMap<MainWidgets, QMap<MainActions, bool>> m_actions;
    QMap<MainWidgets, QIcon> m_icons;
    static MainWindow* m_instance;
    QColor m_buttonColor;
    QColor m_buttonActiveColor;
};
