#ifndef SUPPORTDIALOG_H
#define SUPPORTDIALOG_H

#include <QDialog>

namespace Ui {
class SupportDialog;
}

class SupportDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SupportDialog(QWidget *parent = 0);
    ~SupportDialog();
    
private:
    Ui::SupportDialog *ui;
};

#endif // SUPPORTDIALOG_H
