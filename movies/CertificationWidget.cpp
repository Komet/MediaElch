#include "CertificationWidget.h"
#include "ui_CertificationWidget.h"

CertificationWidget::CertificationWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CertificationWidget)
{
    ui->setupUi(this);
}

CertificationWidget::~CertificationWidget()
{
    delete ui;
}

void CertificationWidget::clear()
{

}

void CertificationWidget::loadCertifications()
{
    qDebug() << "Entered";

}

void CertificationWidget::onSaveInformation()
{
    qDebug() << "Entered";

}
