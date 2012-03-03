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
    
private slots:
    void execAboutDialog();
    void execSettingsDialog();
    void progressProgress(int current, int max);
    void progressFinished();
    void progressStarted();

private:
    Ui::MainWindow *ui;
    QProgressBar *m_progressBar;
    AboutDialog *m_aboutDialog;
};

#endif // MAINWINDOW_H
