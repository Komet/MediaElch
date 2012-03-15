#include "FilterWidget.h"
#include "ui_FilterWidget.h"

FilterWidget::FilterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterWidget)
{
    ui->setupUi(this);
    ui->lineEdit->setShowMagnifier(true);
    ui->lineEdit->setAdditionalStyleSheet("QLineEdit { border: 1px solid rgba(0, 0, 0, 100); border-radius: 10px; }");
    ui->lineEdit->setType(MyLineEdit::TypeClear);
    ui->lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), this, SIGNAL(sigFilterTextChanged(QString)));
}

FilterWidget::~FilterWidget()
{
    delete ui;
}
