#ifndef QUESTIONDIALOG_H
#define QUESTIONDIALOG_H

#include <QDialog>

namespace Ui {
class QuestionDialog;
}

class QuestionDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit QuestionDialog(QWidget *parent = 0);
    ~QuestionDialog();
    static QuestionDialog *instance(QWidget *parent = 0);
public slots:
    int exec();
private:
    Ui::QuestionDialog *ui;
};

#endif // QUESTIONDIALOG_H
