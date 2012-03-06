#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include "AboutDialog.h"
#include "data/MovieFileSearcher.h"

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

private slots:
    void execSettingsDialog();
    void progressProgress(int current, int max);
    void progressFinished();
    void progressStarted();

private:
    Ui::MainWindow *ui;
    QProgressBar *m_progressBar;
    AboutDialog *m_aboutDialog;
    QAction *m_actionSearch;
    QAction *m_actionSave;
    QAction *m_actionSettings;
    QAction *m_actionAbout;
    QAction *m_actionQuit;
    void setupToolbar();
};

#endif // MAINWINDOW_H
