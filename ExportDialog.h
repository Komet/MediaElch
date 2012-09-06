#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

namespace Ui {
class ExportDialog;
}

/**
 * @brief The ExportDialog class
 * This is an old class which is currently not used and will be removed later
 */
class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = 0);
    ~ExportDialog();
public slots:
    int exec();
private slots:
    void exportDatabase();
    void chooseExportDirectory();
    void onExportStarted();
    void onExportProgress(int current, int max);
    void onExportFinished();
    void onExportError(QString error);
private:
    Ui::ExportDialog *ui;
};

#endif // EXPORTDIALOG_H
