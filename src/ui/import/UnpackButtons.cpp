#include "UnpackButtons.h"
#include "ui_UnpackButtons.h"

#include <QInputDialog>
#include <QMessageBox>

#include "globals/Helper.h"

UnpackButtons::UnpackButtons(QWidget* parent) : QWidget(parent), ui(new Ui::UnpackButtons)
{
    ui->setupUi(this);
    helper::setButtonStyle(ui->btnUnpack, helper::ButtonPrimary);
    helper::setButtonStyle(ui->btnUnpackWithPassword, helper::ButtonInfo);
    helper::setButtonStyle(ui->btnStop, helper::ButtonWarning);
    helper::setButtonStyle(ui->btnDelete, helper::ButtonDanger);
    ui->progressBar->setVisible(false);
    ui->btnStop->setVisible(false);
    connect(ui->btnUnpack, &QAbstractButton::clicked, this, &UnpackButtons::onUnpack);
    connect(ui->btnUnpackWithPassword, &QAbstractButton::clicked, this, &UnpackButtons::onUnpackWithPassword);
    connect(ui->btnStop, &QAbstractButton::clicked, this, &UnpackButtons::onStop);
    connect(ui->btnDelete, &QAbstractButton::clicked, this, &UnpackButtons::onDelete);

    ui->progressBar->setFormat("   " + ui->progressBar->format());
}

UnpackButtons::~UnpackButtons()
{
    delete ui;
}

void UnpackButtons::setBaseName(QString baseName)
{
    m_baseName = baseName;
}

QString UnpackButtons::baseName() const
{
    return m_baseName;
}

void UnpackButtons::onUnpack()
{
    emit sigUnpack(m_baseName, "");
}

void UnpackButtons::onUnpackWithPassword()
{
    bool ok = false;
    QString password =
        QInputDialog::getText(this, tr("Extraction password"), tr("Password"), QLineEdit::Password, "", &ok);
    if (!ok) {
        return;
    }
    emit sigUnpack(m_baseName, password);
}

void UnpackButtons::onStop()
{
    emit sigStop(m_baseName);
}

void UnpackButtons::onDelete()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Delete archive?"));
    msgBox.setInformativeText(tr("Do you really want to delete this archive?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setIcon(QMessageBox::Question);
    if (msgBox.exec() == QMessageBox::Yes) {
        emit sigDelete(m_baseName);
    }
}

void UnpackButtons::setShowProgress(bool showProgress)
{
    ui->progressBar->setVisible(showProgress);
    ui->btnStop->setVisible(showProgress);
    ui->btnUnpack->setVisible(!showProgress);
    ui->btnUnpackWithPassword->setVisible(!showProgress);
    ui->btnDelete->setVisible(!showProgress);
}

void UnpackButtons::setProgress(int progress)
{
    ui->progressBar->setValue(progress);
}
