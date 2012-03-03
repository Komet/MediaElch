#include "AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->labelXbmm->setText(tr("XBMM %1").arg(QApplication::applicationVersion()));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
