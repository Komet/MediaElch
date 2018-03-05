#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

/**
 * @brief The AboutDialog class
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog() override;

public slots:
    int exec() override;

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
