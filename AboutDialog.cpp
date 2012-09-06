#include "AboutDialog.h"
#include "ui_AboutDialog.h"

/**
 * @brief AboutDialog::AboutDialog
 * @param parent
 */
AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->labelXbmm->setText(tr("MediaElch %1").arg(QApplication::applicationVersion()));

#ifdef Q_WS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif
}

/**
 * @brief AboutDialog::~AboutDialog
 */
AboutDialog::~AboutDialog()
{
    delete ui;
}
