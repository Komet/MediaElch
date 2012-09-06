#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include "AboutDialog.h"
#include "Globals.h"
#include "data/MovieFileSearcher.h"
#include "smallWidgets/FilterWidget.h"
#include "settings/Settings.h"

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

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void progressProgress(int current, int max, int id);
    void progressFinished(int id);
    void progressStarted(QString msg, int id);
    void onMenuMovies();
    void onMenuMovieSets();
    void onMenuTvShows();
    void onMenuSettings();
    void onActionSearch();
    void onActionSave();
    void onActionSaveAll();
    void onFilterChanged(QString text);
    void onSetSaveEnabled(bool enabled, MainWidgets widget);
    void onSetSearchEnabled(bool enabled, MainWidgets widget);
    void onMovieSplitterMoved();
    void onTvShowSplitterMoved();
    void onMovieSetsSplitterMoved();

private:
    Ui::MainWindow *ui;
    Settings *m_settings;
    AboutDialog *m_aboutDialog;
    QAction *m_actionSearch;
    QAction *m_actionSave;
    QAction *m_actionAbout;
    QAction *m_actionQuit;
    QAction *m_actionSaveAll;
    QMap<MainActions, bool> m_movieActions;
    QMap<MainActions, bool> m_movieSetActions;
    QMap<MainActions, bool> m_tvShowActions;
    FilterWidget *m_filterWidget;
    void setupToolbar();
};

#endif // MAINWINDOW_H
