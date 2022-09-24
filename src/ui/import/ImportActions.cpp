#include "ImportActions.h"
#include "ui_ImportActions.h"

#include "globals/Manager.h"
#include "ui/UiUtils.h"
#include "ui/import/ImportDialog.h"

#include <QMessageBox>

ImportActions::ImportActions(QWidget* parent) : QWidget(parent), ui(new Ui::ImportActions)
{
    ui->setupUi(this);
    mediaelch::ui::setButtonStyle(ui->btnImport, mediaelch::ui::ButtonSuccess);
    mediaelch::ui::setButtonStyle(ui->btnDelete, mediaelch::ui::ButtonDanger);
    connect(ui->btnImport, &QAbstractButton::clicked, this, &ImportActions::onImport);
    connect(ui->btnDelete, &QAbstractButton::clicked, this, &ImportActions::onDelete);
    m_tvShow = nullptr;
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
    // Initialize the import dialog only when we actually import something.
    // Don't initialize it in this class' constructor as it will take a lot of
    // time to initialize an import dialog for each import item.
    // Don't set the parent because it will only inherit its stylesheet...
    // We delete the dialog in this function so that is fine.
    auto* importDialog = new ImportDialog();
    importDialog->setFiles(files());
    importDialog->setExtraFiles(extraFiles());

    if (type() == "movie") {
        importDialog->setImportDir(importDir());
        if (importDialog->execMovie(baseName()) == QDialog::Accepted) {
            Manager::instance()->database()->addImport(baseName(), type(), mediaelch::DirectoryPath(importDir()));
        }
    } else if (type() == "tvshow") {
        if (importDialog->execTvShow(baseName(), tvShow()) == QDialog::Accepted) {
            Manager::instance()->database()->addImport(baseName(), type(), tvShow()->dir());
        }
    } else if (type() == "concert") {
        importDialog->setImportDir(importDir());
        if (importDialog->execConcert(baseName()) == QDialog::Accepted) {
            Manager::instance()->database()->addImport(baseName(), type(), mediaelch::DirectoryPath(importDir()));
        }
    }

    // The dialog was closed so we can safely delete it again.
    importDialog->deleteLater();

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
