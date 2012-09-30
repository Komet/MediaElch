#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include "AboutDialog.h"
#include "globals/Globals.h"
#include "data/MovieFileSearcher.h"
#include "smallWidgets/FilterWidget.h"
#include "settings/Settings.h"
#include "settings/SettingsWidget.h"

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
    void onMenu(MainWidgets widget);
    void onMenuMovies();
    void onMenuMovieSets();
    void onMenuTvShows();
    void onMenuConcerts();
    void onMenuGenres();
    void onMenuCertifications();
    void onActionSearch();
    void onActionSave();
    void onActionSaveAll();
    void onFilterChanged(QString text);
    void onSetSaveEnabled(bool enabled, MainWidgets widget);
    void onSetSearchEnabled(bool enabled, MainWidgets widget);
    void onMovieSplitterMoved();
    void onTvShowSplitterMoved();
    void onMovieSetsSplitterMoved();
    void onConcertSplitterMoved();

private:
    Ui::MainWindow *ui;
    Settings *m_settings;
    SettingsWidget *m_settingsWidget;
    AboutDialog *m_aboutDialog;
    QAction *m_actionSearch;
    QAction *m_actionSave;
    QAction *m_actionAbout;
    QAction *m_actionQuit;
    QAction *m_actionSaveAll;
    QAction *m_actionSettings;
    QMap<MainWidgets, QMap<MainActions, bool> > m_actions;
    FilterWidget *m_filterWidget;
    void setupToolbar();
};

#endif // MAINWINDOW_H
