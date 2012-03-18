#include "ExportDialog.h"
#include "ui_ExportDialog.h"

#include <QtConcurrentRun>
#include <QFileDialog>
#include "Manager.h"

ExportDialog::ExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);

#ifdef Q_WS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #ExportDialog { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    connect(ui->buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->buttonExport, SIGNAL(clicked()), this, SLOT(exportDatabase()));
    connect(ui->buttonChooseDir, SIGNAL(clicked()), this, SLOT(chooseExportDirectory()));
    connect(Manager::instance()->mediaCenterInterface(), SIGNAL(sigExportStarted()), this, SLOT(onExportStarted()));
    connect(Manager::instance()->mediaCenterInterface(), SIGNAL(sigExportProgress(int,int)), this, SLOT(onExportProgress(int,int)));
    connect(Manager::instance()->mediaCenterInterface(), SIGNAL(sigExportDone()), this, SLOT(onExportFinished()));
    connect(Manager::instance()->mediaCenterInterface(), SIGNAL(sigExportRaiseError(QString)), this, SLOT(onExportError(QString)));
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

int ExportDialog::exec()
{
    ui->labelError->hide();
    ui->exportDirectory->setText("");
    ui->search->setText("");
    ui->search->setReadOnly(false);
    ui->replace->setText("");
    ui->replace->setReadOnly(false);
    ui->progressBar->hide();
    ui->buttonCancel->show();
    ui->buttonExport->show();
    ui->buttonExport->setEnabled(false);
    ui->buttonChooseDir->setEnabled(true);
    ui->buttonCancel->setText(tr("Cancel"));
    return QDialog::exec();
}

void ExportDialog::exportDatabase()
{
    ui->buttonCancel->hide();
    ui->buttonExport->hide();
    ui->search->setReadOnly(true);
    ui->replace->setReadOnly(true);
    ui->buttonChooseDir->setEnabled(false);
    QtConcurrent::run(Manager::instance()->mediaCenterInterface(), &MediaCenterInterface::exportDatabase,
                      Manager::instance()->movieModel()->movies(), ui->exportDirectory->text(), ui->search->text(), ui->replace->text());
}

void ExportDialog::onExportStarted()
{
    ui->progressBar->show();
    ui->progressBar->setValue(0);
}

void ExportDialog::onExportProgress(int current, int max)
{
    ui->progressBar->setRange(0, max);
    ui->progressBar->setValue(current);
}

void ExportDialog::onExportFinished()
{
    ui->progressBar->hide();
    ui->buttonCancel->setText(tr("Close"));
    ui->buttonCancel->show();
}

void ExportDialog::onExportError(QString error)
{
    ui->labelError->setText(error);
    ui->labelError->show();
    ui->progressBar->hide();
    ui->buttonCancel->setText(tr("Close"));
    ui->buttonCancel->show();
}

void ExportDialog::chooseExportDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Export Directory"), QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty()) {
        ui->exportDirectory->setText(dir);
        ui->buttonExport->setEnabled(true);
    }
}
