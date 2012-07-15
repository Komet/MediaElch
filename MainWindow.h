#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include "AboutDialog.h"
#include "ExportDialog.h"
#include "Globals.h"
#include "data/MovieFileSearcher.h"
#include "smallWidgets/FilterWidget.h"
#include "SettingsWidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void progressProgress(int current, int max, int id);
    void progressFinished(int id);
    void progressStarted(QString msg, int id);
    void onMenuMovies();
    void onMenuTvShows();
    void onMenuSettings();
    void onActionSearch();
    void onActionSave();
    void onActionSaveAll();
    void onActionRefresh();
    void onFilterChanged(QString text);
    void onSetRefreshEnabled(bool enabled, MainWidgets widget);
    void onSetExportEnabled(bool enabled, MainWidgets widget);
    void onSetSaveEnabled(bool enabled, MainWidgets widget);
    void onSetSearchEnabled(bool enabled, MainWidgets widget);
    void onMovieSplitterMoved();
    void onTvShowSplitterMoved();

private:
    Ui::MainWindow *ui;
    SettingsWidget *m_settingsWidget;
    AboutDialog *m_aboutDialog;
    ExportDialog *m_exportDialog;
    QAction *m_actionSearch;
    QAction *m_actionSave;
    QAction *m_actionExport;
    QAction *m_actionAbout;
    QAction *m_actionQuit;
    QAction *m_actionRefreshFiles;
    QAction *m_actionSaveAll;
    QMap<MainActions, bool> m_movieActions;
    QMap<MainActions, bool> m_tvShowActions;
    FilterWidget *m_filterWidget;
    void setupToolbar();
};

#endif // MAINWINDOW_H
