#ifndef FILESCANNERDIALOG_H
#define FILESCANNERDIALOG_H

#include <QDialog>

namespace Ui {
class FileScannerDialog;
}

/**
 * @brief The FileScannerDialog class
 */
class FileScannerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileScannerDialog(QWidget *parent = 0);
    ~FileScannerDialog();

public slots:
    void exec();
    void reject();

private slots:
    void onStartMovieScanner();
    void onStartTvShowScanner();
    void onStartConcertScanner();
    void onProgress(int current, int max);

private:
    Ui::FileScannerDialog *ui;
};

#endif // FILESCANNERDIALOG_H
