#include "ImportActions.h"
#include "ui_ImportActions.h"

#include <QDebug>
#include <QMessageBox>

#include "globals/Helper.h"
#include "globals/Manager.h"

ImportActions::ImportActions(QWidget* parent) : QWidget(parent), ui(new Ui::ImportActions)
{
    ui->setupUi(this);
    helper::setButtonStyle(ui->btnImport, helper::ButtonSuccess);
    helper::setButtonStyle(ui->btnDelete, helper::ButtonDanger);
    connect(ui->btnImport, &QAbstractButton::clicked, this, &ImportActions::onImport);
    connect(ui->btnDelete, &QAbstractButton::clicked, this, &ImportActions::onDelete);
    m_tvShow = nullptr;
    m_importDialog = new ImportDialog(parent);
}

ImportActions::~ImportActions()
{
    delete ui;
}

void ImportActions::setButtonEnabled(bool enabled)
{
    ui->btnImport->setEnabled(enabled);
}

void ImportActions::setBaseName(QString baseName)
{
    m_baseName = baseName;
}

QString ImportActions::baseName()
{
    return m_baseName;
}

void ImportActions::setType(QString type)
{
    m_type = type;
}

QString ImportActions::type()
{
    return m_type;
}

void ImportActions::setTvShow(TvShow* show)
{
    m_importDir.clear();
    m_tvShow = show;
}

TvShow* ImportActions::tvShow()
{
    return m_tvShow;
}

void ImportActions::setImportDir(QString dir)
{
    m_importDir = dir;
}

QString ImportActions::importDir()
{
    m_tvShow = nullptr;
    return m_importDir;
}

void ImportActions::setFiles(QStringList files)
{
    m_files = files;
}

QStringList ImportActions::files()
{
    return m_files;
}

void ImportActions::setExtraFiles(QStringList extraFiles)
{
    m_extraFiles = extraFiles;
}

QStringList ImportActions::extraFiles()
{
    return m_extraFiles;
}

void ImportActions::onImport()
{
    m_importDialog->setFiles(files());
    m_importDialog->setExtraFiles(extraFiles());

    if (type() == "movie") {
        m_importDialog->setImportDir(importDir());
        if (m_importDialog->execMovie(baseName()) == QDialog::Accepted) {
            Manager::instance()->database()->addImport(baseName(), type(), importDir());
        }
    } else if (type() == "tvshow") {
        if (m_importDialog->execTvShow(baseName(), tvShow()) == QDialog::Accepted) {
            Manager::instance()->database()->addImport(baseName(), type(), tvShow()->dir());
        }
    } else if (type() == "concert") {
        m_importDialog->setImportDir(importDir());
        if (m_importDialog->execConcert(baseName()) == QDialog::Accepted) {
            Manager::instance()->database()->addImport(baseName(), type(), importDir());
        }
    }

    emit sigDialogClosed();
}

void ImportActions::onDelete()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Delete file?"));
    msgBox.setInformativeText(tr("Do you really want to delete this file?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setIcon(QMessageBox::Question);
    if (msgBox.exec() == QMessageBox::Yes) {
        emit sigDelete(m_baseName);
    }
}
