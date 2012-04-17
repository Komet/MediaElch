#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include "AboutDialog.h"
#include "ExportDialog.h"
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

public slots:
    void setActionSaveEnabled(bool enabled);
    void setActionSearchEnabled(bool enabled);
    void setActionExportEnabled(bool enabled = true);
    void setActionExportDisabled(bool disabled = true);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void progressProgress(int current, int max, int id);
    void progressFinished(int id);
    void progressStarted(QString msg, int id);
    void onMenuMovies();
    void onMenuTvShows();
    void onMenuSettings();
    void onActionSave();
    void onFilterChanged(QString text);

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
    FilterWidget *m_filterWidget;
    void setupToolbar();
};

#endif // MAINWINDOW_H
