#include "Navbar.h"
#include "ui_Navbar.h"

Navbar::Navbar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Navbar)
{
    ui->setupUi(this);

    ui->btnSearch->setShortcut(QKeySequence::Find);
    ui->btnSearch->setToolTip(tr("Search (%1)").arg(QKeySequence(QKeySequence::Find).toString(QKeySequence::NativeText)));

    ui->btnSave->setShortcut(QKeySequence::Save);
    ui->btnSave->setToolTip(tr("Save (%1)").arg(QKeySequence(QKeySequence::Save).toString(QKeySequence::NativeText)));

    QKeySequence seqSaveAll(Qt::CTRL+Qt::ShiftModifier+Qt::Key_S);
    ui->btnSaveAll->setShortcut(seqSaveAll);
    ui->btnSaveAll->setToolTip(tr("Save All (%1)").arg(seqSaveAll.toString(QKeySequence::NativeText)));

    ui->btnReload->setShortcut(QKeySequence::Refresh);
    ui->btnReload->setToolTip(tr("Reload all files (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));

    connect(ui->btnSearch, SIGNAL(clicked()), this, SIGNAL(sigSearch()));
    connect(ui->btnSave, SIGNAL(clicked()), this, SIGNAL(sigSave()));
    connect(ui->btnSaveAll, SIGNAL(clicked()), this, SIGNAL(sigSaveAll()));
    connect(ui->btnReload, SIGNAL(clicked()), this, SIGNAL(sigReload()));
    connect(ui->btnRename, SIGNAL(clicked()), this, SIGNAL(sigRename()));
    connect(ui->btnSettings, SIGNAL(clicked()), this, SIGNAL(sigSettings()));
    connect(ui->btnSync, SIGNAL(clicked()), this, SIGNAL(sigSync()));
    connect(ui->btnExport, SIGNAL(clicked()), this, SIGNAL(sigExport()));
    connect(ui->btnAbout, SIGNAL(clicked()), this, SIGNAL(sigAbout()));
    connect(ui->btnDonate, SIGNAL(clicked()), this, SIGNAL(sigLike()));
}

Navbar::~Navbar()
{
    delete ui;
}

void Navbar::setActionSearchEnabled(bool enabled)
{
    ui->btnSearch->setEnabled(enabled);
}

void Navbar::setActionSaveEnabled(bool enabled)
{
    ui->btnSave->setEnabled(enabled);
}

void Navbar::setActionSaveAllEnabled(bool enabled)
{
    ui->btnSaveAll->setEnabled(enabled);
}

void Navbar::setActionReloadEnabled(bool enabled)
{
    ui->btnReload->setEnabled(enabled);
}

void Navbar::setActionRenameEnabled(bool enabled)
{
    ui->btnRename->setEnabled(enabled);
}

void Navbar::setReloadToolTip(QString toolTip)
{
    ui->btnReload->setToolTip(toolTip);
}
