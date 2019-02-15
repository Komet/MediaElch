#include "SupportDialog.h"
#include "ui_SupportDialog.h"

#include "settings/Settings.h"

SupportDialog::SupportDialog(QWidget* parent) : QDialog(parent), ui(new Ui::SupportDialog)
{
    ui->setupUi(this);
#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    connect(ui->btnAlreadyDonated, &QAbstractButton::clicked, this, &SupportDialog::onAlreadyDonated);
}

SupportDialog::~SupportDialog()
{
    delete ui;
}

void SupportDialog::onAlreadyDonated()
{
    Settings::instance()->setDonated(true);
    accept();
}
